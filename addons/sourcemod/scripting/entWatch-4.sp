//====================================================================================================
//
// Name: [entWatch] Core
// Author: zaCade, Prometheum, koen, tilgep, .Rushaway
// Description: Handle the core functions of [entWatch]
//
//====================================================================================================
// Requires Sourcemod Version: 1.10.0.6531 or above
//====================================================================================================
#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <sdkhooks>
#include <sdktools_entoutput>
#include <sdktools_functions>
#include <multicolors>
#include <entWatch_core>

// ─────────────────────────────────────────────
// Feature flags — comment/uncomment to toggle modules
// ─────────────────────────────────────────────
#define EW4_BEACONS
#define EW4_DEBUG
#define EW4_FORCEDROP
#define EW4_INTERFACE
#define EW4_OVERRIDE_ITEM
#define EW4_RESTRICTIONS
#define EW4_SERVERCOMMAND
#define EW4_SPAWN_ITEMS
#define EW4_TRANSFER
#define EW4_USE_PRIORITY

/* BOOLS */
bool g_bLate;
bool g_bIntermission;

/* INTEGERS */
int g_iPlayerFormat = 3;
int g_iAuthIDType = 1;
int g_iMessageMode = 1;

/* FLOATS */
float g_flGameFrameTime;

/* CONVARS */
ConVar g_hCVar_PlayerFormat;
ConVar g_hCVar_MsgsAuthID;
ConVar g_hCVar_ColorConfig;
ConVar g_hCVar_MessageMode;

/* ARRAYS */
ArrayList g_hArray_Items;
ArrayList g_hArray_Configs;

/* HANDLES */
Handle SDKCall_GetSlot;
Handle SDKCall_OnPickedUp;
Handle SDKCall_BumpWeapon;

/* FORWARDS */
GlobalForward g_hFwd_OnClientItemWeaponInteract;
GlobalForward g_hFwd_OnClientItemButtonInteract;
GlobalForward g_hFwd_OnClientItemTriggerInteract;

GlobalForward g_hFwd_OnClientItemWeaponCanInteract;
GlobalForward g_hFwd_OnClientItemButtonCanInteract;
GlobalForward g_hFwd_OnClientItemTriggerCanInteract;

/* STRUCTS */
enum struct ColorStruct
{
	char sTag[8];        // String: Hex color of entwatch tag
	char sName[8];       // String: Hex color of player name
	char sAuthID[8];     // String: Hex color of player steam ID
	char sActivate[8];   // String: Hex color of item use message
	char sPickup[8];     // String: Hex color of item pickup message
	char sDrop[8];       // String: Hex color of item drop message
	char sDeath[8];      // String: Hex color of player death message
	char sDisconnect[8]; // String: Hex color of player disconnect message
	char sWarning[8];    // String: Hex color of warning message

	void Reset()
	{
		this.sTag        = "E11E64";
		this.sName       = "F0F0F0";
		this.sAuthID     = "B4B4B4";
		this.sActivate   = "64AFE1";
		this.sPickup     = "AFE164";
		this.sDrop       = "E164AF";
		this.sDeath      = "E1AF64";
		this.sDisconnect = "E1AF64";
		this.sWarning    = "E1AF64";
	}
}

ColorStruct g_clr;

// ─────────────────────────────────────────────
// Module includes — do not edit below this line
// ─────────────────────────────────────────────
#if defined EW4_BEACONS
  #include "ew4/beacons.inc"
#endif
#if defined EW4_DEBUG
  #include "ew4/debug.inc"
#endif
#if defined EW4_FORCEDROP
  #include "ew4/forcedrop.inc"
#endif
#if defined EW4_INTERFACE
  #include "ew4/interface.inc"
#endif
#if defined EW4_OVERRIDE_ITEM
  #include "ew4/override-item.inc"
#endif
#if defined EW4_RESTRICTIONS
  #include "ew4/restrictions.inc"
#endif
#if defined EW4_SERVERCOMMAND
  #include "ew4/servercommand.inc"
#endif
#if defined EW4_SPAWN_ITEMS
  #include "ew4/spawn-items.inc"
#endif
#if defined EW4_TRANSFER
  #include "ew4/transfer.inc"
#endif
#if defined EW4_USE_PRIORITY
  #include "ew4/use-priority.inc"
#endif

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public Plugin myinfo =
{
	name         = "[entWatch] Core",
	author       = "zaCade, Prometheum, koen, tilgep, .Rushaway",
	description  = "Handle the core functions of [entWatch]",
	version      = EW_VERSION
};

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public APLRes AskPluginLoad2(Handle hMyself, bool bLate, char[] sError, int iErrorSize)
{
	g_bLate = bLate;

	CreateNative("EW_LoadConfig",      Native_LoadConfig);
	CreateNative("EW_GetItemsArray",   Native_GetItemsArray);
	CreateNative("EW_GetConfigsArray", Native_GetConfigsArray);
	CreateNative("EW_IsEntityItem",    Native_IsEntityItem);
	CreateNative("EW_ClientHasItem",   Native_ClientHasItem);

	RegPluginLibrary("entWatch-core");

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_AskPluginLoad2();
	#endif

	#if defined EW4_TRANSFER
	Ew4_Transfer_AskPluginLoad2();
	#endif

	#if defined EW4_SPAWN_ITEMS
	Ew4_Spawn_AskPluginLoad2();
	#endif

	return APLRes_Success;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	LoadTranslations("entWatch.phrases");

	g_hFwd_OnClientItemWeaponInteract  = new GlobalForward("EW_OnClientItemWeaponInteract", ET_Ignore, Param_Cell, Param_Cell, Param_Cell);
	g_hFwd_OnClientItemButtonInteract  = new GlobalForward("EW_OnClientItemButtonInteract", ET_Ignore, Param_Cell, Param_Cell);
	g_hFwd_OnClientItemTriggerInteract = new GlobalForward("EW_OnClientItemTriggerInteract", ET_Ignore, Param_Cell, Param_Cell);

	g_hFwd_OnClientItemWeaponCanInteract  = new GlobalForward("EW_OnClientItemWeaponCanInteract",  ET_Hook, Param_Cell, Param_Cell);
	g_hFwd_OnClientItemButtonCanInteract  = new GlobalForward("EW_OnClientItemButtonCanInteract",  ET_Hook, Param_Cell, Param_Cell);
	g_hFwd_OnClientItemTriggerCanInteract = new GlobalForward("EW_OnClientItemTriggerCanInteract", ET_Hook, Param_Cell, Param_Cell);

	g_hArray_Items   = new ArrayList();
	g_hArray_Configs = new ArrayList();

	g_hCVar_PlayerFormat = CreateConVar("sm_eplayer_format", "3", "Player info display format in chat messages (0 = Name only, 1 = Name + UserID, 2 = Name + SteamID, 3 = Name + UserID + SteamID)", FCVAR_NONE, true, 0.0, true, 3.0);
	g_hCVar_MsgsAuthID   = CreateConVar("sm_emessages_authid", "1", "AuthID type used in messages [0 = Engine, 1 = Steam2, 2 = Steam3, 3 = Steam64]", FCVAR_NONE, true, 0.0, true, 3.0);
	g_hCVar_MessageMode  = CreateConVar("sm_emessages_mode", "1", "Entwatch message recipient mode (1 = All, 2 = Team Only + Admin, 3 = Team Only)", FCVAR_NONE, true, 1.0, true, 3.0);
	g_hCVar_ColorConfig  = CreateConVar("sm_emessages_config", "classic", "Name of entWatch-message color config file");

	g_hCVar_PlayerFormat.AddChangeHook(OnConVarChange);
	g_hCVar_MsgsAuthID.AddChangeHook(OnConVarChange);
	g_hCVar_ColorConfig.AddChangeHook(OnConVarChange);
	g_hCVar_MessageMode.AddChangeHook(OnConVarChange);

	// Initial cache
	g_iPlayerFormat = g_hCVar_PlayerFormat.IntValue;
	g_iAuthIDType = g_hCVar_MsgsAuthID.IntValue;
	g_iMessageMode = g_hCVar_MessageMode.IntValue;

	LoadColors();
	AutoExecConfig();

	HookEvent("player_death", OnClientDeath);
	HookEvent("round_start",  OnRoundStart);
	HookEvent("round_end",    OnRoundEnd);

	#if defined EW4_BEACONS
	Ew4_Beacons_OnPluginStart();
	#endif

	#if defined EW4_DEBUG
	Ew4_Debug_OnPluginStart();
	#endif

	#if defined EW4_FORCEDROP
	EW_SDK_Load_GetSlot();
	#endif

	#if defined EW4_INTERFACE
	Ew4_Interface_OnPluginStart();
	#endif

	#if defined EW4_OVERRIDE_ITEM
	Ew4_OverrideItem_OnPluginStart();
	#endif

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnPluginStart();
	#endif

	#if defined EW4_SERVERCOMMAND
	Ew4_ServerCommand_OnPluginStart();
	#endif

	#if defined EW4_SPAWN_ITEMS
	Ew4_Spawn_OnPluginStart();
	#endif

	#if defined EW4_TRANSFER
	Ew4_Transfer_OnPluginStart();
	EW_SDK_Load();
	#endif

	#if defined EW4_USE_PRIORITY
	EW4_UsePriority_OnPluginStart();
	#endif

	if (g_bLate)
	{
		for (int iClient = 1; iClient <= MaxClients; iClient++)
		{
			if (!IsClientConnected(iClient))
				continue;

			SDKHook(iClient, SDKHook_WeaponEquipPost, OnWeaponPickup);
			SDKHook(iClient, SDKHook_WeaponDropPost, OnWeaponDrop);
			SDKHook(iClient, SDKHook_WeaponCanUse, OnWeaponTouch);

			#if defined EW4_RESTRICTIONS
			if (!IsClientInGame(iClient) || IsFakeClient(iClient))
				continue;

			OfflinePlayer_TrackOrUpdate(iClient, "None", true);
			Database_FetchClientBan(iClient);
			#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnConVarChange(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (convar == g_hCVar_PlayerFormat)
    	g_iPlayerFormat = g_hCVar_PlayerFormat.IntValue;
	else if (convar == g_hCVar_MsgsAuthID)
		g_iAuthIDType = g_hCVar_MsgsAuthID.IntValue;
	else if (convar == g_hCVar_MessageMode)
		g_iMessageMode = g_hCVar_MessageMode.IntValue;
	else if (convar == g_hCVar_ColorConfig)
		LoadColors();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnPluginEnd()
{
	CleanupItems();
	CleanupConfigs();

	#if defined EW4_BEACONS
	Ew4_Beacons_OnPluginEnd();
	#endif

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnPluginEnd();
	#endif
}

#if defined EW4_INTERFACE
public void OnLibraryRemoved(const char[] name)
{
	Ew4_Interface_OnLibraryRemoved(name);
}
#endif

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnMapStart()
{
	LoadConfig(g_bLate);

	#if defined EW4_BEACONS
	Ew4_Beacons_OnMapStart();
	#endif

	#if defined EW4_DEBUG
	Ew4_Debug_OnMapStart();
	#endif

	#if defined EW4_INTERFACE
	Ew4_Interface_OnMapStart();
	#endif

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnMapStart();
	#endif
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnMapEnd()
{
	CleanupItems();
	CleanupConfigs();

	#if defined EW4_BEACONS
	Ew4_Beacons_OnMapEnd();
	#endif
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void LoadColors()
{
	g_clr.Reset();

	char sConfig[32], sFilePath[PLATFORM_MAX_PATH];
	g_hCVar_ColorConfig.GetString(sConfig, sizeof(sConfig));
	BuildPath(Path_SM, sFilePath, sizeof(sFilePath), "configs/entwatch/colors/%s.cfg", sConfig);

	KeyValues kv = new KeyValues("colors");
	if (!kv.ImportFromFile(sFilePath))
	{
		LogError("[entWatch-messages] Failed to load color config. Falling back on default colors.");
		delete kv;
		return;
	}

	kv.GetString("color_tag",        g_clr.sTag,        sizeof(g_clr.sTag),        g_clr.sTag);
	kv.GetString("color_name",       g_clr.sName,       sizeof(g_clr.sName),       g_clr.sName);
	kv.GetString("color_steamid",    g_clr.sAuthID,     sizeof(g_clr.sAuthID),     g_clr.sAuthID);
	kv.GetString("color_use",        g_clr.sActivate,   sizeof(g_clr.sActivate),   g_clr.sActivate);
	kv.GetString("color_pickup",     g_clr.sPickup,     sizeof(g_clr.sPickup),     g_clr.sPickup);
	kv.GetString("color_drop",       g_clr.sDrop,       sizeof(g_clr.sDrop),       g_clr.sDrop);
	kv.GetString("color_death",      g_clr.sDeath,      sizeof(g_clr.sDeath),      g_clr.sDeath);
	kv.GetString("color_disconnect", g_clr.sDisconnect, sizeof(g_clr.sDisconnect), g_clr.sDisconnect);
	kv.GetString("color_warning",    g_clr.sWarning,    sizeof(g_clr.sWarning),    g_clr.sWarning);

	delete kv;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool LoadConfig(bool bLoopEntities = false)
{
	CleanupItems();
	CleanupConfigs();

	char sGameDirectory[128];
	GetGameFolderName(sGameDirectory, sizeof(sGameDirectory));

	char sCurrentMap[128];
	GetCurrentMap(sCurrentMap, sizeof(sCurrentMap));

	int iChar;
	while (sCurrentMap[iChar] != EOS && iChar < sizeof(sCurrentMap))
	{
		sCurrentMap[iChar] = CharToLower(sCurrentMap[iChar]);
		iChar++;
	}

	char sFilePathDefault[PLATFORM_MAX_PATH];
	char sFilePathOverride[PLATFORM_MAX_PATH];

	BuildPath(Path_SM, sFilePathDefault, sizeof(sFilePathDefault), "configs/entwatch/%s/%s.cfg", sGameDirectory, sCurrentMap);
	BuildPath(Path_SM, sFilePathOverride, sizeof(sFilePathOverride), "configs/entwatch/%s/%s.override.cfg", sGameDirectory, sCurrentMap);

	KeyValues hConfigFile = new KeyValues("items");

	if (FileExists(sFilePathOverride))
	{
		if (!hConfigFile.ImportFromFile(sFilePathOverride))
		{
			LogMessage("Unable to load config \"%s\"!", sFilePathOverride);

			delete hConfigFile;
			return false;
		}
		else LogMessage("Loaded config \"%s\"", sFilePathOverride);
	}
	else
	{
		if (!hConfigFile.ImportFromFile(sFilePathDefault))
		{
			LogMessage("Unable to load config \"%s\"!", sFilePathDefault);

			delete hConfigFile;
			return false;
		}
		else LogMessage("Loaded config \"%s\"", sFilePathDefault);
	}

	int iConfigVersion = -1;
	if ((iConfigVersion = hConfigFile.GetNum("configversion", -1)) < EW_VERSION_CONFIG)
	{
		LogMessage("Config version unsupported or not specified! (version: [%d] | required: [%d])", iConfigVersion, EW_VERSION_CONFIG);

		delete hConfigFile;
		return false;
	}

	if (hConfigFile.GotoFirstSubKey())
	{
		int iConfigID;

		do
		{
			CConfig hConfig = new CConfig();

			char sName[32], sShort[16], sColor[8], sSpawner[32];
			hConfigFile.GetString("name",       sName,    sizeof(sName));
			hConfigFile.GetString("short",      sShort,   sizeof(sShort));
			hConfigFile.GetString("color",      sColor,   sizeof(sColor));
			hConfigFile.GetString("template", sSpawner, sizeof(sSpawner));

			hConfig.SetName(sName);
			hConfig.SetShort(sShort);
			hConfig.SetColor(sColor);
			hConfig.SetSpawner(sSpawner);

			hConfig.iConfigID      = iConfigID++;
			hConfig.iHammerID      = hConfigFile.GetNum("hammerid");

			hConfig.bShowMessages  = view_as<bool>(hConfigFile.GetNum("showmessages", 1));
			hConfig.bShowInterface = view_as<bool>(hConfigFile.GetNum("showinterface", 1));
			hConfig.bAllowTransfer = view_as<bool>(hConfigFile.GetNum("allowtransfer", 1));

			if (hConfigFile.JumpToKey("buttons"))
			{
				if (hConfigFile.GotoFirstSubKey())
				{
					int iConfigButtonID;

					do
					{
						CConfigButton hConfigButton = new CConfigButton(hConfig);


						char sOutput[32], sButtonName[32];
						hConfigFile.GetString("output", sOutput, sizeof(sOutput));
						hConfigFile.GetString("name", sButtonName, sizeof(sButtonName));

						hConfigButton.SetOutput(sOutput);
						hConfigButton.SetName(sButtonName);

						hConfigButton.iConfigID = iConfigButtonID++;
						hConfigButton.iHammerID = hConfigFile.GetNum("hammerid");
						hConfigButton.iType     = hConfigFile.GetNum("type");
						hConfigButton.iMode     = hConfigFile.GetNum("mode");
						hConfigButton.iMaxUses  = hConfigFile.GetNum("maxuses");

						hConfigButton.flButtonCooldown = hConfigFile.GetFloat("cooldown");
						hConfigButton.flItemCooldown   = hConfigFile.GetFloat("itemcooldown");

						hConfigButton.bShowActivate = view_as<bool>(hConfigFile.GetNum("showactivate", 0));
						hConfigButton.bShowCooldown = view_as<bool>(hConfigFile.GetNum("showcooldown", 0));

						hConfig.hButtons.Push(hConfigButton);
					}
					while (hConfigFile.GotoNextKey());

					hConfigFile.GoBack();
				}

				hConfigFile.GoBack();
			}

			if (hConfigFile.JumpToKey("triggers"))
			{
				if (hConfigFile.GotoFirstSubKey())
				{
					int iConfigTriggerID;

					do
					{
						CConfigTrigger hConfigTrigger = new CConfigTrigger(hConfig);

						hConfigTrigger.iConfigID = iConfigTriggerID++;
						hConfigTrigger.iHammerID = hConfigFile.GetNum("hammerid");
						hConfigTrigger.iType     = hConfigFile.GetNum("type");

						hConfig.hTriggers.Push(hConfigTrigger);
					}
					while (hConfigFile.GotoNextKey());

					hConfigFile.GoBack();
				}

				hConfigFile.GoBack();
			}

			g_hArray_Configs.Push(hConfig);
		}
		while (hConfigFile.GotoNextKey());
	}

	if (bLoopEntities)
	{
		int iEntity = INVALID_ENT_REFERENCE;
		while ((iEntity = FindEntityByClassname(iEntity, "*")) != INVALID_ENT_REFERENCE)
		{
			OnEntitySpawnPost(iEntity);
		}
	}

	#if defined EW4_SPAWN_ITEMS
	Ew4_Spawn_OnConfigLoaded();
	#endif

	delete hConfigFile;
	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void CleanupConfigs()
{
	if (!g_hArray_Configs.Length)
		return;

	for (int iConfigID; iConfigID < g_hArray_Configs.Length; iConfigID++)
	{
		CConfig hConfig = g_hArray_Configs.Get(iConfigID);

		for (int iConfigButtonID; iConfigButtonID < hConfig.hButtons.Length; iConfigButtonID++)
		{
			CConfigButton hConfigButton = hConfig.hButtons.Get(iConfigButtonID);

			delete hConfigButton;
		}

		for (int iConfigTriggerID; iConfigTriggerID < hConfig.hTriggers.Length; iConfigTriggerID++)
		{
			CConfigTrigger hConfigTrigger = hConfig.hTriggers.Get(iConfigTriggerID);

			delete hConfigTrigger;
		}

		delete hConfig;
	}

	g_hArray_Configs.Clear();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void CleanupItems()
{
	if (!g_hArray_Configs.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (!IsValidEntity(hItemButton.iButton))
			{
				delete hItemButton;
				continue;
			}

			switch (hItemButton.hConfigButton.iType)
			{
				case EW_BUTTON_TYPE_USE:
				{
					SDKUnhook(hItemButton.iButton, SDKHook_Use, OnButtonPress);
				}
				case EW_BUTTON_TYPE_OUTPUT:
				{
					char sButtonOutput[32];
					hItemButton.hConfigButton.GetOutput(sButtonOutput, sizeof(sButtonOutput));

					UnhookSingleEntityOutput(hItemButton.iButton, sButtonOutput, OnButtonOutput);
				}
				case EW_BUTTON_TYPE_COUNTERUP, EW_BUTTON_TYPE_COUNTERDOWN:
				{
					UnhookSingleEntityOutput(hItemButton.iButton, "OutValue", OnCounterOutput);
				}
			}

			delete hItemButton;
		}

		for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
		{
			CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

			if (!IsValidEntity(hItemTrigger.iTrigger))
			{
				delete hItemTrigger;
				continue;
			}

			switch (hItemTrigger.hConfigTrigger.iType)
			{
				case EW_TRIGGER_TYPE_STRIP:
				{
					SDKUnhook(hItemTrigger.iTrigger, SDKHook_StartTouch, OnTriggerTouch);
					SDKUnhook(hItemTrigger.iTrigger, SDKHook_EndTouch, OnTriggerTouch);
					SDKUnhook(hItemTrigger.iTrigger, SDKHook_Touch, OnTriggerTouch);
				}
			}

			delete hItemTrigger;
		}

		delete hItem;
	}

	g_hArray_Items.Clear();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnRoundStart(Event hEvent, const char[] sEvent, bool bDontBroadcast)
{
	g_bIntermission = false;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnRoundEnd(Event hEvent, const char[] sEvent, bool bDontBroadcast)
{
	CleanupItems();

	g_bIntermission = true;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnEntityCreated(int iEntity, const char[] sClassname)
{
	if (!IsValidEntity(iEntity))
		return;

	#if defined EW4_SERVERCOMMAND
	Ew4_ServerCommand_OnEntityCreated(iEntity, sClassname);
	#endif

	if (!g_hArray_Configs.Length)
		return;

	SDKHook(iEntity, SDKHook_SpawnPost, OnEntitySpawnPost);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnEntitySpawnPost(int iEntity)
{
	if (!IsValidEntity(iEntity) || !g_hArray_Configs.Length)
		return;

	int iHammerID = GetEntProp(iEntity, Prop_Data, "m_iHammerID");

	for (int iConfigID; iConfigID < g_hArray_Configs.Length; iConfigID++)
	{
		CConfig hConfig = g_hArray_Configs.Get(iConfigID);

		if (hConfig.iHammerID && hConfig.iHammerID == iHammerID)
		{
			bool bRegistered;

			for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
			{
				CItem hItem = g_hArray_Items.Get(iItemID);

				if (hItem.hConfig != hConfig)
					continue;

				if (!RegisterItemWeapon(hItem, iEntity))
					continue;

				bRegistered = true;
				break;
			}

			if (!bRegistered)
			{
				CItem hItem = new CItem(hConfig, g_hArray_Items.Length);

				if (!RegisterItemWeapon(hItem, iEntity))
				{
					delete hItem;
					continue;
				}

				InsertItemSorted(g_hArray_Items, hItem);

				char sItemName[64];
				hConfig.GetName(sItemName, sizeof(sItemName));
				PrintToServer("[EntWatch] Item spawned: %s | %i", sItemName, iHammerID);
				break;
			}
		}

		for (int iConfigButtonID; iConfigButtonID < hConfig.hButtons.Length; iConfigButtonID++)
		{
			CConfigButton hConfigButton = hConfig.hButtons.Get(iConfigButtonID);

			if (!hConfigButton.iHammerID || hConfigButton.iHammerID != iHammerID)
				continue;

			bool bRegistered;

			for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
			{
				CItem hItem = g_hArray_Items.Get(iItemID);

				if (hItem.hConfig != hConfig)
					continue;

				if (!RegisterItemButton(hConfigButton, hItem, iEntity))
					continue;

				bRegistered = true;
				break;
			}

			if (!bRegistered)
			{
				CItem hItem = new CItem(hConfig, g_hArray_Items.Length);

				if (!RegisterItemButton(hConfigButton, hItem, iEntity))
				{
					delete hItem;
					continue;
				}

				InsertItemSorted(g_hArray_Items, hItem);

				char sItemName[64];
				hConfig.GetName(sItemName, sizeof(sItemName));
				PrintToServer("[EntWatch] Item spawned: %s | %i", sItemName, iHammerID);
				break;
			}
		}

		for (int iConfigTriggerID; iConfigTriggerID < hConfig.hTriggers.Length; iConfigTriggerID++)
		{
			CConfigTrigger hConfigTrigger = hConfig.hTriggers.Get(iConfigTriggerID);

			if (!hConfigTrigger.iHammerID || hConfigTrigger.iHammerID != iHammerID)
				continue;

			bool bRegistered;

			for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
			{
				CItem hItem = g_hArray_Items.Get(iItemID);

				if (hItem.hConfig != hConfig)
					continue;

				if (!RegisterItemTrigger(hConfigTrigger, hItem, iEntity))
					continue;

				bRegistered = true;
				break;
			}

			if (!bRegistered)
			{
				CItem hItem = new CItem(hConfig, g_hArray_Items.Length);

				if (!RegisterItemTrigger(hConfigTrigger, hItem, iEntity))
				{
					delete hItem;
					continue;
				}

				InsertItemSorted(g_hArray_Items, hItem);

				char sItemName[64];
				hConfig.GetName(sItemName, sizeof(sItemName));
				PrintToServer("[EntWatch] Item spawned: %s | %i", sItemName, iHammerID);
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void InsertItemSorted(ArrayList hArray, CItem hItem)
{
	bool bShifted;

	for (int iShiftItemID; iShiftItemID < hArray.Length; iShiftItemID++)
	{
		CItem hShiftItem = hArray.Get(iShiftItemID);

		if (hItem.hConfig.iConfigID < hShiftItem.hConfig.iConfigID)
		{
			hArray.ShiftUp(iShiftItemID);
			hArray.Set(iShiftItemID, hItem);

			bShifted = true;
			break;
		}
	}

	if (!bShifted)
		hArray.Push(hItem);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool RegisterItemWeapon(CItem hItem, int iWeapon)
{
	if (!IsValidEntity(iWeapon))
		return false;

	if (hItem.iWeapon != INVALID_ENT_REFERENCE || hItem.iState != EW_ENTITY_STATE_INITIAL)
		return false;

	hItem.iWeapon = iWeapon;
	hItem.iState  = EW_ENTITY_STATE_SPAWNED;

	int iOwner = INVALID_ENT_REFERENCE;
	if ((iOwner = GetEntPropEnt(iWeapon, Prop_Data, "m_hOwnerEntity")) != INVALID_ENT_REFERENCE && IsValidClient(iOwner))
	{
		hItem.iClient = iOwner;
		hItem.iState  = EW_ENTITY_STATE_EQUIPPED;

		Forward_OnClientItemWeaponInteract(hItem.iClient, hItem, EW_ENTITY_STATE_EQUIPPED);
	}

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool RegisterItemButton(CConfigButton hConfigButton, CItem hItem, int iButton)
{
	if (!IsValidEntity(iButton) || HasDuplicateItemButton(hConfigButton, hItem))
		return false;

	switch (hConfigButton.iType)
	{
		case EW_BUTTON_TYPE_USE:
		{
			SDKHook(iButton, SDKHook_Use, OnButtonPress);
		}
		case EW_BUTTON_TYPE_OUTPUT:
		{
			char sButtonOutput[32];
			hConfigButton.GetOutput(sButtonOutput, sizeof(sButtonOutput));

			HookSingleEntityOutput(iButton, sButtonOutput, OnButtonOutput);
		}
		case EW_BUTTON_TYPE_COUNTERUP, EW_BUTTON_TYPE_COUNTERDOWN:
		{
			HookSingleEntityOutput(iButton, "OutValue", OnCounterOutput);
		}
	}

	CItemButton hItemButton = new CItemButton(hConfigButton, hItem);
	hItemButton.iButton = iButton;
	hItemButton.iState  = EW_ENTITY_STATE_SPAWNED;

	if (hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN || hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
	{
		int iCounterMax = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMax"));
		int iCounterMin = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMin"));
		hConfigButton.iMaxUses = iCounterMax - iCounterMin;

		if (hConfigButton.iMode == EW_BUTTON_MODE_COUNTERVALUE)
		{
			if (hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
				hItemButton.iCurrentUses = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue")) - iCounterMin;
			else if (hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
				hItemButton.iCurrentUses = iCounterMax - RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue"));
		}
		else
		{
			if (hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
				hItemButton.iCurrentUses = iCounterMax - RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue"));
			else if (hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
				hItemButton.iCurrentUses = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue")) - iCounterMin;
		}
	}

	bool bShifted;

	for (int iShiftItemButtonID; iShiftItemButtonID < hItem.hButtons.Length; iShiftItemButtonID++)
	{
		CItemButton hShiftItemButton = hItem.hButtons.Get(iShiftItemButtonID);

		if (hConfigButton.iConfigID < hShiftItemButton.hConfigButton.iConfigID)
		{
			hItem.hButtons.ShiftUp(iShiftItemButtonID);
			hItem.hButtons.Set(iShiftItemButtonID, hItemButton);

			bShifted = true;
			break;
		}
	}

	if (!bShifted)
		hItem.hButtons.Push(hItemButton);

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool HasDuplicateItemButton(CConfigButton hConfigButton, CItem hItem)
{
	if (!hItem.hButtons.Length)
		return false;

	for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
	{
		CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

		if (hItemButton.hConfigButton == hConfigButton)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool RegisterItemTrigger(CConfigTrigger hConfigTrigger, CItem hItem, int iTrigger)
{
	if (!IsValidEntity(iTrigger) || HasDuplicateItemTrigger(hConfigTrigger, hItem))
		return false;

	switch (hConfigTrigger.iType)
	{
		case EW_TRIGGER_TYPE_STRIP:
		{
			SDKHook(iTrigger, SDKHook_StartTouch, OnTriggerTouch);
			SDKHook(iTrigger, SDKHook_EndTouch, OnTriggerTouch);
			SDKHook(iTrigger, SDKHook_Touch, OnTriggerTouch);
		}
	}

	CItemTrigger hItemTrigger = new CItemTrigger(hConfigTrigger, hItem);
	hItemTrigger.iTrigger = iTrigger;
	hItemTrigger.iState   = EW_ENTITY_STATE_SPAWNED;

	bool bShifted;

	for (int iShiftItemTriggerID; iShiftItemTriggerID < hItem.hTriggers.Length; iShiftItemTriggerID++)
	{
		CItemTrigger hShiftItemTrigger = hItem.hTriggers.Get(iShiftItemTriggerID);

		if (hConfigTrigger.iConfigID < hShiftItemTrigger.hConfigTrigger.iConfigID)
		{
			hItem.hTriggers.ShiftUp(iShiftItemTriggerID);
			hItem.hTriggers.Set(iShiftItemTriggerID, hItemTrigger);

			bShifted = true;
			break;
		}
	}

	if (!bShifted)
		hItem.hTriggers.Push(hItemTrigger);

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool HasDuplicateItemTrigger(CConfigTrigger hConfigTrigger, CItem hItem)
{
	if (!hItem.hTriggers.Length)
		return false;

	for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
	{
		CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

		if (hItemTrigger.hConfigTrigger == hConfigTrigger)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnEntityDestroyed(int iEntity)
{
	if (!IsValidEntity(iEntity) || !g_hArray_Items.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iWeapon != INVALID_ENT_REFERENCE && hItem.iWeapon == iEntity)
		{
			int iPrevClient = hItem.iClient;
			hItem.iClient = INVALID_ENT_REFERENCE;
			hItem.iWeapon = INVALID_ENT_REFERENCE;
			hItem.iState  = EW_ENTITY_STATE_DESTROYED;

			Forward_OnClientItemWeaponInteract(iPrevClient, hItem, EW_ENTITY_STATE_DESTROYED);
		}

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton != INVALID_ENT_REFERENCE && hItemButton.iButton == iEntity)
			{
				hItemButton.iButton = INVALID_ENT_REFERENCE;
				hItemButton.iState  = EW_ENTITY_STATE_DESTROYED;
			}
		}

		for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
		{
			CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

			if (hItemTrigger.iTrigger != INVALID_ENT_REFERENCE && hItemTrigger.iTrigger == iEntity)
			{
				hItemTrigger.iTrigger = INVALID_ENT_REFERENCE;
				hItemTrigger.iState   = EW_ENTITY_STATE_DESTROYED;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnClientPutInServer(int iClient)
{
	SDKHook(iClient, SDKHook_WeaponEquipPost, OnWeaponPickup);
	SDKHook(iClient, SDKHook_WeaponDropPost, OnWeaponDrop);
	SDKHook(iClient, SDKHook_WeaponCanUse, OnWeaponTouch);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnClientCookiesCached(int iClient)
{
	#if defined EW4_INTERFACE
	Ew4_Interface_OnClientCookiesCached(iClient);
	#endif
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnClientPostAdminCheck(int client)
{
	#if defined EW4_INTERFACE
	Ew4_Interface_OnClientPostAdminCheck(client);
	#endif

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnClientPostAdminCheck(client);
	#endif
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnClientDisconnect(int iClient)
{
	#if defined EW4_INTERFACE
	Ew4_Interface_OnClientDisconnect(iClient);
	#endif

	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnClientDisconnect(iClient);
	#endif

	if (!g_hArray_Items.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient != INVALID_ENT_REFERENCE && hItem.iClient == iClient)
		{
			hItem.iClient = INVALID_ENT_REFERENCE;
			hItem.iState = EW_ENTITY_STATE_DROPPED;

			Forward_OnClientItemWeaponInteract(iClient, hItem, EW_WEAPON_INTERACTION_DISCONNECT);
		}
	}
}

#if defined EW4_USE_PRIORITY
public Action OnPlayerRunCmd(int iClient, int& iButtons, int& iImpulse, float vel[3], float angles[3])
{
    EW4_UsePriority_OnPlayerRunCmd(iClient, iButtons, angles);
    return Plugin_Continue;
}
#endif

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnClientDeath(Event hEvent, const char[] sEvent, bool bDontBroadcast)
{
	int iClient = GetClientOfUserId(hEvent.GetInt("userid"));

	if (!IsValidClient(iClient) || !g_hArray_Items.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient != INVALID_ENT_REFERENCE && hItem.iClient == iClient)
		{
			hItem.iClient = INVALID_ENT_REFERENCE;
			hItem.iState = EW_ENTITY_STATE_DROPPED;

			Forward_OnClientItemWeaponInteract(iClient, hItem, EW_WEAPON_INTERACTION_DEATH);
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnWeaponPickup(int iClient, int iWeapon)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iWeapon) || !g_hArray_Items.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iWeapon != INVALID_ENT_REFERENCE && hItem.iWeapon == iWeapon)
		{
			hItem.iClient = iClient;
			hItem.iState = EW_ENTITY_STATE_EQUIPPED;

			Forward_OnClientItemWeaponInteract(iClient, hItem, EW_WEAPON_INTERACTION_PICKUP);
			return;
		}
	}

}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void OnWeaponDrop(int iClient, int iWeapon)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iWeapon) || !g_hArray_Items.Length)
		return;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iWeapon != INVALID_ENT_REFERENCE && hItem.iWeapon == iWeapon)
		{
			hItem.iClient = INVALID_ENT_REFERENCE;
			hItem.iState = EW_ENTITY_STATE_DROPPED;

			Forward_OnClientItemWeaponInteract(iClient, hItem, EW_WEAPON_INTERACTION_DROP);
			return;
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnGameFrame()
{
	g_flGameFrameTime = GetGameTime();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action OnButtonPress(int iButton, int iClient)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iButton) || !g_hArray_Items.Length)
		return Plugin_Handled;

	if (HasEntProp(iButton, Prop_Data, "m_bLocked") &&
		GetEntProp(iButton, Prop_Data, "m_bLocked"))
		return Plugin_Handled;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient == INVALID_ENT_REFERENCE || hItem.iClient != iClient)
			continue;

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton == INVALID_ENT_REFERENCE || hItemButton.iButton != iButton)
				continue;

			if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_USE)
			{
				if (HasEntProp(iButton, Prop_Data, "m_flWait"))
				{
					if (hItemButton.flWaitTime < g_flGameFrameTime)
						hItemButton.flWaitTime = g_flGameFrameTime + GetEntPropFloat(iButton, Prop_Data, "m_flWait");
					else
						return Plugin_Handled;
				}

				return ProcessButtonPress(iClient, hItem, hItemButton);
			}
		}
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action OnButtonOutput(const char[] sOutput, int iButton, int iClient, float flDelay)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iButton) || !g_hArray_Items.Length)
		return Plugin_Handled;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient == INVALID_ENT_REFERENCE || hItem.iClient != iClient)
			continue;

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton == INVALID_ENT_REFERENCE || hItemButton.iButton != iButton)
				continue;

			if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_OUTPUT)
			{
				char sButtonOutput[32];
				hItemButton.hConfigButton.GetOutput(sButtonOutput, sizeof(sButtonOutput));

				if (StrEqual(sOutput, sButtonOutput, false))
					return ProcessButtonPress(iClient, hItem, hItemButton);
			}
		}
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Separate output hook for math_counter hook because iClient can be invalid
//----------------------------------------------------------------------------------------------------
stock Action OnCounterOutput(const char[] sOutput, int iButton, int iClient, float flDelay)
{
	if (!IsValidEntity(iButton) || !g_hArray_Items.Length)
		return Plugin_Continue;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient == INVALID_ENT_REFERENCE)
			continue;

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton != INVALID_ENT_REFERENCE && hItemButton.iButton == iButton)
				return ProcessCounterValue(iClient, hItem, hItemButton);
		}
	}

	return Plugin_Continue;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action ProcessButtonPress(int iClient, CItem hItem, CItemButton hItemButton)
{
	if (hItem.flReadyTime > g_flGameFrameTime)
		return Plugin_Handled;

	bool bResult = true;
	Call_StartForward(g_hFwd_OnClientItemButtonCanInteract);
	Call_PushCell(iClient);
	Call_PushCell(hItemButton);
	Call_Finish(bResult);

	if (!bResult)
		return Plugin_Handled;

	switch (hItemButton.hConfigButton.iMode)
	{
		case EW_BUTTON_MODE_COOLDOWN:
		{
			if (hItemButton.flReadyTime < g_flGameFrameTime)
				hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
			else
				return Plugin_Handled;
		}
		case EW_BUTTON_MODE_MAXUSES:
		{
			if (hItemButton.flReadyTime < g_flGameFrameTime && hItemButton.iCurrentUses < hItemButton.hConfigButton.iMaxUses)
			{
				hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
				hItemButton.iCurrentUses++;
			}
			else
				return Plugin_Handled;
		}
		case EW_BUTTON_MODE_COOLDOWN_CHARGES:
		{
			if (hItemButton.flReadyTime < g_flGameFrameTime)
			{
				hItemButton.iCurrentUses++;

				if (hItemButton.iCurrentUses >= hItemButton.hConfigButton.iMaxUses)
				{
					hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
					hItemButton.iCurrentUses = 0;
				}
			}
			else
				return Plugin_Handled;
		}
	}

	hItem.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flItemCooldown;

	Forward_OnClientItemButtonInteract(iClient, hItemButton);
	return Plugin_Continue;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action ProcessCounterValue(int iClient, CItem hItem, CItemButton hItemButton)
{
	if (hItem.flReadyTime > g_flGameFrameTime)
		return Plugin_Continue;

	int iNewCurrentUses = 0;

	switch (hItemButton.hConfigButton.iMode)
	{
		case EW_BUTTON_MODE_COOLDOWN:
		{
			if (hItemButton.flReadyTime < g_flGameFrameTime)
				hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
			else
				return Plugin_Continue;
		}
		case EW_BUTTON_MODE_MAXUSES:
		{
			int iCounterMax = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMax"));
			int iCounterMin = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMin"));
			hItemButton.hConfigButton.iMaxUses = iCounterMax - iCounterMin;

			if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
				iNewCurrentUses = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue")) - iCounterMin;
			else if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
				iNewCurrentUses = iCounterMax - RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue"));

			if (iNewCurrentUses <= hItemButton.iCurrentUses)
			{
				hItemButton.iCurrentUses = iNewCurrentUses;
				return Plugin_Continue;
			}

			hItemButton.iCurrentUses = iNewCurrentUses;
			hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
		}
		case EW_BUTTON_MODE_COOLDOWN_CHARGES:
		{
			int iCounterMax = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMax"));
			int iCounterMin = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMin"));
			hItemButton.hConfigButton.iMaxUses = iCounterMax - iCounterMin;

			if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
				iNewCurrentUses = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue")) - iCounterMin;
			else if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
				iNewCurrentUses = iCounterMax - RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue"));

			if (iNewCurrentUses <= hItemButton.iCurrentUses)
			{
				hItemButton.iCurrentUses = iNewCurrentUses;
				return Plugin_Continue;
			}

			hItemButton.iCurrentUses = iNewCurrentUses;

			if (hItemButton.iCurrentUses >= hItemButton.hConfigButton.iMaxUses)
				hItemButton.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flButtonCooldown;
		}
		case EW_BUTTON_MODE_COUNTERVALUE:
		{
			int iCounterMax = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMax"));
			int iCounterMin = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_flMin"));
			hItemButton.hConfigButton.iMaxUses = iCounterMax - iCounterMin;

			if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
				hItemButton.iCurrentUses = RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue")) - iCounterMin;
			else if (hItemButton.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERUP)
				hItemButton.iCurrentUses = iCounterMax - RoundFloat(GetEntPropFloat(hItemButton.iButton, Prop_Data, "m_OutValue"));
		}
	}

	hItem.flReadyTime = g_flGameFrameTime + hItemButton.hConfigButton.flItemCooldown;

	Forward_OnClientItemButtonInteract(iClient, hItemButton);
	return Plugin_Continue;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action OnTriggerTouch(int iTrigger, int iClient)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iTrigger) || !g_hArray_Items.Length)
		return Plugin_Handled;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
		{
			CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

			if (hItemTrigger.iTrigger == INVALID_ENT_REFERENCE || hItemTrigger.iTrigger != iTrigger)
				continue;

			if (hItemTrigger.hConfigTrigger.iType == EW_TRIGGER_TYPE_STRIP)
			{
				if (g_bIntermission)
					return Plugin_Handled;

				bool bResult = true;
				Call_StartForward(g_hFwd_OnClientItemTriggerCanInteract);
				Call_PushCell(iClient);
				Call_PushCell(hItemTrigger);
				Call_Finish(bResult);

				if (!bResult)
					return Plugin_Handled;

				Forward_OnClientItemTriggerInteract(iClient, hItemTrigger);
				return Plugin_Continue;
			}
		}
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action OnWeaponTouch(int iClient, int iWeapon)
{
	if (!IsValidClient(iClient) || !IsValidEntity(iWeapon) || !g_hArray_Items.Length)
		return Plugin_Continue;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iWeapon == INVALID_ENT_REFERENCE || hItem.iWeapon != iWeapon)
			continue;

		if (g_bIntermission)
			return Plugin_Handled;

		bool bResult = true;
		Call_StartForward(g_hFwd_OnClientItemWeaponCanInteract);
		Call_PushCell(iClient);
		Call_PushCell(hItem);
		Call_Finish(bResult);

		if (bResult)
			return Plugin_Continue;
		else
			return Plugin_Handled;
	}

	return Plugin_Continue;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock bool IsValidClient(int iClient)
{
	return ((1 <= iClient <= MaxClients) && IsClientConnected(iClient));
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public any Native_LoadConfig(Handle hPlugin, int iNumParams)
{
	bool bLoopEntities = GetNativeCell(1);

	return LoadConfig(bLoopEntities);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public any Native_GetItemsArray(Handle hPlugin, int iNumParams)
{
	return g_hArray_Items;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public any Native_GetConfigsArray(Handle hPlugin, int iNumParams)
{
	return g_hArray_Configs;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public any Native_IsEntityItem(Handle hPlugin, int iNumParams)
{
	if (!g_hArray_Items.Length)
		return false;

	int iEntity = GetNativeCell(1);
	if (!IsValidEdict(iEntity) && IsValidEntity(iEntity) && g_hArray_Items.Length)
		return false;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iWeapon != INVALID_ENT_REFERENCE && hItem.iWeapon == iEntity)
			return true;

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton != INVALID_ENT_REFERENCE && hItemButton.iButton == iEntity)
				return true;
		}

		for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
		{
			CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

			if (hItemTrigger.iTrigger != INVALID_ENT_REFERENCE && hItemTrigger.iTrigger == iEntity)
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public any Native_ClientHasItem(Handle hPlugin, int iNumParams)
{
	if (!g_hArray_Items.Length)
		return false;

	int iClient = GetNativeCell(1);
	if (!IsValidClient(iClient))
		return false;

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient != INVALID_ENT_REFERENCE && hItem.iClient == iClient)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Forwards
//----------------------------------------------------------------------------------------------------
stock void Forward_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType)
{
	Call_StartForward(g_hFwd_OnClientItemWeaponInteract);
	Call_PushCell(iClient);
	Call_PushCell(hItem);
	Call_PushCell(iInteractionType);
	Call_Finish();

	API_OnClientItemWeaponInteract(iClient, hItem, iInteractionType);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Forwards
//----------------------------------------------------------------------------------------------------
stock void Forward_OnClientItemButtonInteract(int iClient, CItemButton hItemButton)
{
	Call_StartForward(g_hFwd_OnClientItemButtonInteract);
	Call_PushCell(iClient);
	Call_PushCell(hItemButton);
	Call_Finish();

	API_OnClientItemButtonInteract(iClient, hItemButton);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Forwards
//----------------------------------------------------------------------------------------------------
stock void Forward_OnClientItemTriggerInteract(int iClient, CItemTrigger hItemTrigger)
{
	Call_StartForward(g_hFwd_OnClientItemTriggerInteract);
	Call_PushCell(iClient);
	Call_PushCell(hItemTrigger);
	Call_Finish();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void API_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType)
{
	#if defined EW4_RESTRICTIONS
	Ew4_Restrictions_OnClientItemWeaponInteract(iClient, hItem, iInteractionType);
	#endif

	#if defined EW4_FORCEDROP
	Ew4_Forcedrop_OnClientItemWeaponInteract(iClient, hItem, iInteractionType);
	#endif

	if (!hItem.hConfig.bShowMessages)
		return;

	if (iClient == INVALID_ENT_REFERENCE)
		return;

	char sPlayerInfo[128];
	FormatPlayerInfo(iClient, sPlayerInfo, sizeof(sPlayerInfo));

	char sTranslation[32], sColor[8];
	switch (iInteractionType)
	{
		case EW_WEAPON_INTERACTION_DROP:
		{
			Format(sTranslation, sizeof(sTranslation), "Item Drop");
			Format(sColor, sizeof(sColor), g_clr.sDrop);
		}
		case EW_WEAPON_INTERACTION_DEATH:
		{
			Format(sTranslation, sizeof(sTranslation), "Item Death");
			Format(sColor, sizeof(sColor), g_clr.sDeath);
		}
		case EW_WEAPON_INTERACTION_PICKUP:
		{
			Format(sTranslation, sizeof(sTranslation), "Item Pickup");
			Format(sColor, sizeof(sColor), g_clr.sPickup);
		}
		case EW_WEAPON_INTERACTION_DISCONNECT:
		{
			Format(sTranslation, sizeof(sTranslation), "Item Disconnect");
			Format(sColor, sizeof(sColor), g_clr.sDisconnect);
		}
	}

	char sItemName[32];
	hItem.hConfig.GetName(sItemName, sizeof(sItemName));

	char sItemColor[8];
	hItem.hConfig.GetColor(sItemColor, sizeof(sItemColor));

	PrintChatMessage(iClient, "{#%s}%t %s {#%s}%t {#%s}%s",
		g_clr.sTag, "EW_Tag",
		sPlayerInfo,
		sColor, sTranslation,
		sItemColor, sItemName);
	}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void API_OnClientItemButtonInteract(int iClient, CItemButton hItemButton)
{
	if (!hItemButton.hConfigButton.bShowActivate)
		return;

	if (!iClient)
		return;

	char sPlayerInfo[128];
	FormatPlayerInfo(iClient, sPlayerInfo, sizeof(sPlayerInfo));

	char sItemName[32], sButtonName[32];
	hItemButton.hItem.hConfig.GetName(sItemName, sizeof(sItemName));
	hItemButton.hConfigButton.GetName(sButtonName, sizeof(sButtonName));

	char sItemColor[8];
	hItemButton.hItem.hConfig.GetColor(sItemColor, sizeof(sItemColor));

	if (strlen(sButtonName) != 0)
		PrintChatMessage(iClient, "{#%s}%t %s {#%s}%t {#%s}%s {#%s}(%s)",
			g_clr.sTag,     "EW_Tag",
			sPlayerInfo,
			g_clr.sActivate, "Item Activate",
			sItemColor,              sItemName,
			sItemColor,              sButtonName);
	else
		PrintChatMessage(iClient, "{#%s}%t %s {#%s}%t {#%s}%s",
			g_clr.sTag,     "EW_Tag",
			sPlayerInfo,
			g_clr.sActivate, "Item Activate",
			sItemColor,              sItemName);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void PrintChatMessage(int iClient, const char[] sMessage, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMessage, 3);

	int iTeam = GetClientTeam(iClient);

	switch (g_iMessageMode)
	{
		case 2:
		{
			for (int i = 1; i <= MaxClients; i++)
			{
				if (!IsClientInGame(i))
					continue;

				if (GetClientTeam(i) == iTeam || CheckCommandAccess(i, "", ADMFLAG_GENERIC))
					CPrintToChat(i, sBuffer);
			}
		}
		case 3:
		{
			for (int i = 1; i <= MaxClients; i++)
			{
				if (!IsClientInGame(i))
					continue;

				if (GetClientTeam(i) == iTeam)
					CPrintToChat(i, sBuffer);
			}
		}
		default: CPrintToChatAll(sBuffer);
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose: Formats player info based on g_iPlayerFormat settings
//----------------------------------------------------------------------------------------------------
stock void FormatPlayerInfo(int iClient, char[] sBuffer, int iMaxLen)
{
	char sClientName[MAX_NAME_LENGTH];
	GetClientName(iClient, sClientName, sizeof(sClientName));

	char sClientAuth[32];
	AuthIdType authType = view_as<AuthIdType>(g_iAuthIDType);
	GetClientAuthId(iClient, authType, sClientAuth, sizeof(sClientAuth), false);

	// Normalize auth string regardless of format
	switch (authType)
	{
		case AuthId_Steam3, AuthId_Engine:
		{
			ReplaceString(sClientAuth, sizeof(sClientAuth), "[", "");
			ReplaceString(sClientAuth, sizeof(sClientAuth), "]", "");
		}
		case AuthId_Steam2:
		{
			// Shorten STEAM_X:Y:Z → X:Y:Z — shift pointer by 6 chars in-place
			strcopy(sClientAuth, sizeof(sClientAuth), sClientAuth[6]);
		}
	}

	int iUserID = GetClientUserId(iClient);

	switch (g_iPlayerFormat)
	{
		case 0: // Name only
			Format(sBuffer, iMaxLen, "{#%s}%s",
				g_clr.sName, sClientName);

		case 1: // Name + UserID
			Format(sBuffer, iMaxLen,
				"{#%s}%s {#%s}({#%s}#%d{#%s})",
				g_clr.sName,    sClientName,
				g_clr.sWarning,
				g_clr.sAuthID,  iUserID,
				g_clr.sWarning);

		case 2: // Name + SteamID
			Format(sBuffer, iMaxLen, "{#%s}%s {#%s}({#%s}%s{#%s})",
				g_clr.sName,    sClientName,
				g_clr.sWarning,
				g_clr.sAuthID,  sClientAuth,
				g_clr.sWarning);

		default: // Name + UserID + SteamID
			Format(sBuffer, iMaxLen, "{#%s}%s {#%s}({#%s}#%d {#%s}| {#%s}%s{#%s})",
				g_clr.sName,    sClientName,
				g_clr.sWarning,
				g_clr.sAuthID,  iUserID,
				g_clr.sWarning,
				g_clr.sAuthID,  sClientAuth,
				g_clr.sWarning);
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose: Load all SDK calls from entWatch.games gamedata
//----------------------------------------------------------------------------------------------------
stock void EW_SDK_Load()
{
	EW_SDK_Load_GetSlot();
	EW_SDK_Load_OnPickedUp();
	EW_SDK_Load_BumpWeapon();
}

//----------------------------------------------------------------------------------------------------
// Purpose: Setup SDKCall for CBaseCombatWeapon::GetSlot
//----------------------------------------------------------------------------------------------------
static bool EW_SDK_Load_GetSlot()
{
	static bool bLoaded = false;
	if (bLoaded)
		return true;

	GameData hGameConf;
	if ((hGameConf = new GameData("entWatch.games")) == null)
	{
		SetFailState("Failed to load \"entWatch.games\" game config!");
		return false;
	}

	StartPrepSDKCall(SDKCall_Entity);
	if (!PrepSDKCall_SetFromConf(hGameConf, SDKConf_Virtual, "CBaseCombatWeapon::GetSlot"))
	{
		delete hGameConf;
		SetFailState("Failed to setup SDKCall \"SDKCall_GetSlot\"!");
		return false;
	}

	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((SDKCall_GetSlot = EndPrepSDKCall()) == null)
	{
		delete hGameConf;
		SetFailState("Failed to end SDKCall \"SDKCall_GetSlot\"!");
		return false;
	}

	delete hGameConf;

	bLoaded = true;
	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Setup SDKCall for CBaseCombatWeapon::OnPickedUp
//----------------------------------------------------------------------------------------------------
static bool EW_SDK_Load_OnPickedUp()
{
	static bool bLoaded = false;
	if (bLoaded)
		return true;

	GameData hGameConf;
	if ((hGameConf = new GameData("entWatch.games")) == null)
	{
		SetFailState("Failed to load \"entWatch.games\" game config!");
		return false;
	}

	StartPrepSDKCall(SDKCall_Entity);
	if (!PrepSDKCall_SetFromConf(hGameConf, SDKConf_Virtual, "CBaseCombatWeapon::OnPickedUp"))
	{
		delete hGameConf;
		SetFailState("Failed to setup SDKCall \"SDKCall_OnPickedUp\"!");
		return false;
	}

	PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
	if ((SDKCall_OnPickedUp = EndPrepSDKCall()) == null)
	{
		delete hGameConf;
		SetFailState("Failed to end SDKCall \"SDKCall_OnPickedUp\"!");
		return false;
	}

	delete hGameConf;

	bLoaded = true;
	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Setup SDKCall for CBasePlayer::BumpWeapon
//----------------------------------------------------------------------------------------------------
static bool EW_SDK_Load_BumpWeapon()
{
	static bool bLoaded = false;
	if (bLoaded)
		return true;

	GameData hGameConf;
	if ((hGameConf = new GameData("entWatch.games")) == null)
	{
		SetFailState("Failed to load \"entWatch.games\" game config!");
		return false;
	}

	StartPrepSDKCall(SDKCall_Player);
	if (!PrepSDKCall_SetFromConf(hGameConf, SDKConf_Virtual, "CBasePlayer::BumpWeapon"))
	{
		delete hGameConf;
		SetFailState("Failed to setup SDKCall \"SDKCall_BumpWeapon\"!");
		return false;
	}

	PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
	if ((SDKCall_BumpWeapon = EndPrepSDKCall()) == null)
	{
		delete hGameConf;
		SetFailState("Failed to end SDKCall \"SDKCall_BumpWeapon\"!");
		return false;
	}

	delete hGameConf;

	bLoaded = true;
	return true;
}