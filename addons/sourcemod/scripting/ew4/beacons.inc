//====================================================================================================
//
// Name: [entWatch] Beacon
// Author: zaCade, Prometheum, koen, .Rushaway
// Description: Handle the beacons of [entWatch]
//
//====================================================================================================
// Requires Sourcemod Version: 1.10.0.6531 or above
//====================================================================================================

#include <sdktools_tempents>
#include <sdktools_tempents_stocks>

/* BOOLEANS */
bool g_bEnableSpawnedBeacon = true;
bool g_bEnableDroppedBeacon = true;

/* INTEGERS */
int g_iBeamSprite = -1;
int g_iHaloSprite = -1;

/* CONVARS */
ConVar g_hCVar_EnableSpawnedBeacon = null;
ConVar g_hCVar_EnableDroppedBeacon = null;

/* HANDLES */
Handle g_hBeaconTimer = null;

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Beacons_OnPluginStart()
{
	g_hCVar_EnableSpawnedBeacon = CreateConVar("sm_ebeacons_spawnedbeacons", "1", "Enable beacons on not yet equipped items.", FCVAR_NONE, true, 0.0, true, 1.0);
	g_hCVar_EnableDroppedBeacon = CreateConVar("sm_ebeacons_droppedbeacons", "1", "Enable beacons on dropped items.",          FCVAR_NONE, true, 0.0, true, 1.0);
	g_hCVar_EnableSpawnedBeacon.AddChangeHook(OnCvarChanged);
	g_hCVar_EnableDroppedBeacon.AddChangeHook(OnCvarChanged);

	g_bEnableSpawnedBeacon = g_hCVar_EnableSpawnedBeacon.BoolValue;
	g_bEnableDroppedBeacon = g_hCVar_EnableDroppedBeacon.BoolValue;

	AutoExecConfig();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Beacons_OnPluginEnd()
{
	StopBeaconTimer();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Beacons_OnMapStart()
{
	LoadGameConfig();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Beacons_OnMapEnd()
{
	StopBeaconTimer();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
void OnCvarChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (convar == g_hCVar_EnableSpawnedBeacon)
		g_bEnableSpawnedBeacon = g_hCVar_EnableSpawnedBeacon.BoolValue;
	else if (convar == g_hCVar_EnableDroppedBeacon)
		g_bEnableDroppedBeacon = g_hCVar_EnableDroppedBeacon.BoolValue;

	EvaluateBeaconTimer();
}

//----------------------------------------------------------------------------------------------------
// Purpose: Centralized function to start the beacon timer safely
//----------------------------------------------------------------------------------------------------
void StartBeaconTimer()
{
	// Prevent multiple timer creation
	if (g_hBeaconTimer != null)
		return;

	// Only start if at least one beacon type is enabled
	if (!g_bEnableSpawnedBeacon && !g_bEnableDroppedBeacon)
		return;

	g_hBeaconTimer = CreateTimer(1.0, OnBeacons, _, TIMER_FLAG_NO_MAPCHANGE|TIMER_REPEAT);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Centralized function to stop the beacon timer safely
//----------------------------------------------------------------------------------------------------
void StopBeaconTimer()
{
	if (g_hBeaconTimer != null)
	{
		delete g_hBeaconTimer;
		g_hBeaconTimer = null;
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose: Evaluate if timer should be running and start/stop accordingly
//----------------------------------------------------------------------------------------------------
void EvaluateBeaconTimer()
{
	if (!g_bEnableSpawnedBeacon && !g_bEnableDroppedBeacon)
	{
		StopBeaconTimer();
		return;
	}

	ArrayList hItems = EW_GetItemsArray();
	if (!hItems || !hItems.Length)
	{
		StopBeaconTimer();
		return;
	}

	bool bNeedsBeacon = false;
	for (int iItemID = 0; iItemID < hItems.Length; iItemID++)
	{
		CItem hItem = hItems.Get(iItemID);

		if (!IsValidEntity(hItem.iWeapon))
			continue;

		if ((hItem.iState == EW_ENTITY_STATE_SPAWNED && g_bEnableSpawnedBeacon) || (hItem.iState == EW_ENTITY_STATE_DROPPED && g_bEnableDroppedBeacon))
		{
			bNeedsBeacon = true;
			break;
		}
	}

	// Start or stop timer based on need
	if (bNeedsBeacon)
		StartBeaconTimer();
	else
		StopBeaconTimer();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock void LoadGameConfig()
{
	GameData hGameConf;
	if ((hGameConf = new GameData("funcommands.games")) == null)
	{
		SetFailState("Couldn't load \"funcommands.games\" game config!");
		return;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	if (hGameConf.GetKeyValue("SpriteBeam", sBuffer, sizeof(sBuffer)) && sBuffer[0])
		g_iBeamSprite = PrecacheModel(sBuffer);

	if (hGameConf.GetKeyValue("SpriteHalo", sBuffer, sizeof(sBuffer)) && sBuffer[0])
		g_iHaloSprite = PrecacheModel(sBuffer);

	delete hGameConf;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Forward called when an item state changes
//----------------------------------------------------------------------------------------------------
public void EW_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType)
{
	EvaluateBeaconTimer();
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
stock Action OnBeacons(Handle hTimer)
{
	ArrayList hItems = EW_GetItemsArray();

	if (!hItems || !hItems.Length)
	{
		g_hBeaconTimer = null;
		return Plugin_Stop;
	}

	bool bHasActiveBeacons = false;

	for (int iItemID = 0; iItemID < hItems.Length; iItemID++)
	{
		CItem hItem = hItems.Get(iItemID);

		if (!IsValidEntity(hItem.iWeapon))
			continue;

		if ((hItem.iState == EW_ENTITY_STATE_SPAWNED && g_bEnableSpawnedBeacon) || (hItem.iState == EW_ENTITY_STATE_DROPPED && g_bEnableDroppedBeacon))
		{
			bHasActiveBeacons = true;

			float flOrigin[3];
			GetEntPropVector(hItem.iWeapon, Prop_Data, "m_vecOrigin", flOrigin);

			flOrigin[2] += 2;

			char sColor[8];
			hItem.hConfig.GetColor(sColor, sizeof(sColor));

			int iColorDecimal;
			StringToIntEx(sColor, iColorDecimal, 16);

			int iColor[4];
			iColor[0] = iColorDecimal >> 16 & 0xFF;
			iColor[1] = iColorDecimal >> 8  & 0xFF;
			iColor[2] = iColorDecimal >> 0  & 0xFF;
			iColor[3] = 0xFF;

			TE_SetupBeamRingPoint(flOrigin, 10.0, 50.0, g_iBeamSprite, g_iHaloSprite, 0, 10, 0.6, 4.0, 0.5, iColor, 10, 0);
			TE_SendToAll();
		}
	}

	if (!bHasActiveBeacons || (!g_bEnableSpawnedBeacon && !g_bEnableDroppedBeacon))
	{
		g_hBeaconTimer = null;
		return Plugin_Stop;
	}

	return Plugin_Continue;
}
