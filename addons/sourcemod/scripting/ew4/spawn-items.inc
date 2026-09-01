//====================================================================================================
//
// Name: [entWatch] Spawn Items
// Author: .Rushaway
// Description: Item spawn logic - native implementation and admin command
//
//====================================================================================================

/* FORWARDS */
GlobalForward g_hFwd_OnAdminSpawnItem;

/* ARRAYS */
static ArrayList s_hSpawnableItems;

//----------------------------------------------------------------------------------------------------
// Purpose: Register natives and library before anything else loads
//----------------------------------------------------------------------------------------------------
public void Ew4_Spawn_AskPluginLoad2()
{
	CreateNative("EW_SpawnItem", Native_SpawnItem);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Create the admin-spawn forward and register sm_espawnitem
//----------------------------------------------------------------------------------------------------
public void Ew4_Spawn_OnPluginStart()
{
	g_hFwd_OnAdminSpawnItem = new GlobalForward("EW_OnAdminSpawnItem", ET_Ignore, Param_Cell, Param_String, Param_Cell);

	RegAdminCmd("sm_espawnitem", Command_SpawnItem, ADMFLAG_BAN);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Called after config is loaded - build the spawnable item list from configs
//----------------------------------------------------------------------------------------------------
public void Ew4_Spawn_OnConfigLoaded()
{
	delete s_hSpawnableItems;
	s_hSpawnableItems = new ArrayList();

	ArrayList hConfigs = EW_GetConfigsArray();

	for (int i = 0; i < hConfigs.Length; i++)
	{
		CConfig hConfig = hConfigs.Get(i);

		char sSpawner[64], sName[64];
		hConfig.GetSpawner(sSpawner, sizeof(sSpawner));
		hConfig.GetName(sName, sizeof(sName));

		if (sSpawner[0] != '\0')
			s_hSpawnableItems.Push(hConfig);
	}
}

//----------------------------------------------------------------------------------------------------
// Purpose: Core spawn logic.
//          Finds the point_template matching the item's spawner config, optionally strips the
//          receiver's weapons, then uses env_entity_maker to spawn the item at their feet.
//          Returns true on success, false on any validation failure.
//----------------------------------------------------------------------------------------------------
bool SpawnItemForClient(int iAdmin, int iTarget, int iConfigIndex, bool bStrip)
{
	if (!s_hSpawnableItems || iConfigIndex < 0 || iConfigIndex >= s_hSpawnableItems.Length)
		return false;

	if (iTarget < 1 || iTarget > MaxClients || !IsClientInGame(iTarget))
		return false;

	CConfig hConfig = s_hSpawnableItems.Get(iConfigIndex);

	char sSpawner[64];
	hConfig.GetSpawner(sSpawner, sizeof(sSpawner));

	int iEntity = -1, iPT = -1;
	char sPTName[64];
	while ((iEntity = FindEntityByClassname(iEntity, "point_template")) != -1)
	{
		GetEntPropString(iEntity, Prop_Data, "m_iName", sPTName, sizeof(sPTName));
		if (StrEqual(sPTName, sSpawner))
		{
			iPT = iEntity;
			break;
		}
	}

	if (iPT == -1)
	{
		CReplyToCommand(iAdmin, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Spawn_PTConfigError");
		return false;
	}

	if (bStrip)
	{
		for (int iSlot = 0; iSlot < 5; iSlot++)
		{
			int iWeapon;
			while ((iWeapon = GetPlayerWeaponSlot(iTarget, iSlot)) != -1)
			{
				if (IsValidEntity(iWeapon) && IsValidEdict(iWeapon))
				{
					RemovePlayerItem(iTarget, iWeapon);
					AcceptEntityInput(iWeapon, "Kill");
				}
			}
		}
	}

	float vOrigin[3];
	GetClientAbsOrigin(iTarget, vOrigin);
	vOrigin[2] += 20.0;

	int iMaker = CreateEntityByName("env_entity_maker");
	if (iMaker == -1)
	{
		CReplyToCommand(iAdmin, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Spawn_MakerError");
		return false;
	}

	DispatchKeyValue(iMaker, "EntityTemplate", sSpawner);
	DispatchKeyValue(iMaker, "spawnflags", "0");
	DispatchSpawn(iMaker);
	TeleportEntity(iMaker, vOrigin, NULL_VECTOR, NULL_VECTOR);
	AcceptEntityInput(iMaker, "ForceSpawn");
	AcceptEntityInput(iMaker, "Kill");

	char sName[64];
	hConfig.GetName(sName, sizeof(sName));

	CPrintToChatAll("{#%s}%t {#%s}%N {#%s}%t {#%s}%s {#%s}%t {#%s}%N", g_clr.sTag, "EW_Tag", g_clr.sName, iAdmin, g_clr.sWarning, "Item Spawn", g_clr.sName, sName, g_clr.sWarning, "EW_On", g_clr.sName, iTarget);

	Call_StartForward(g_hFwd_OnAdminSpawnItem);
	Call_PushCell(iAdmin);
	Call_PushString(sName);
	Call_PushCell(iTarget);
	Call_Finish();

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Admin command - sm_espawnitem <receiver> <itemname> [<strip 0/1>]
//
//   Searches items by name/short, spawns directly if unambiguous,
//   shows a disambiguation menu if multiple items match.
//----------------------------------------------------------------------------------------------------
public Action Command_SpawnItem(int iClient, int iArgs)
{
	if (!IsClientInGame(iClient))
		return Plugin_Handled;

	if (!s_hSpawnableItems || !s_hSpawnableItems.Length)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Spawn_NoSpawnableItems");
		return Plugin_Handled;
	}

	if (GetCmdArgs() < 2)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Spawn_Usage");
		return Plugin_Handled;
	}

	char sTarget[64], sItemName[64], sStrip[8];
	GetCmdArg(1, sTarget, sizeof(sTarget));
	GetCmdArg(2, sItemName, sizeof(sItemName));

	bool bStrip = false;
	if (GetCmdArgs() >= 3)
	{
		GetCmdArg(3, sStrip, sizeof(sStrip));
		bStrip = (StringToInt(sStrip) == 1);
	}

	int iTarget = FindTarget(iClient, sTarget, true);
	if (iTarget == -1)
		return Plugin_Handled;

	ArrayList hMatches = new ArrayList();

	for (int i = 0; i < s_hSpawnableItems.Length; i++)
	{
		CConfig hConfig = s_hSpawnableItems.Get(i);
		char sName[64], sShort[64];
		hConfig.GetName(sName, sizeof(sName));
		hConfig.GetShort(sShort, sizeof(sShort));

		if (StrContains(sName, sItemName, false) != -1 || StrContains(sShort, sItemName, false) != -1)
			hMatches.Push(i);
	}

	if (hMatches.Length == 0)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Spawn_NoMatchingItems");
		delete hMatches;
		return Plugin_Handled;
	}
	else if (hMatches.Length == 1)
	{
		SpawnItemForClient(iClient, iTarget, hMatches.Get(0), bStrip);
	}
	else
	{
		Menu hMenu = new Menu(Menu_SpawnItemAmbiguity_Handler);
		hMenu.SetTitle("Select item to spawn:");

		for (int i = 0; i < hMatches.Length; i++)
		{
			int iIdx = hMatches.Get(i);
			CConfig hConfig = s_hSpawnableItems.Get(iIdx);

			char sName[64], sInfo[32];
			hConfig.GetName(sName, sizeof(sName));
			FormatEx(sInfo, sizeof(sInfo), "%d/%d/%d", GetClientUserId(iTarget), iIdx, bStrip ? 1 : 0);
			hMenu.AddItem(sInfo, sName);
		}

		hMenu.Display(iClient, MENU_TIME_FOREVER);
	}

	delete hMatches;
	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Disambiguation menu callback - sm_espawnitem with multiple matches
//----------------------------------------------------------------------------------------------------
public int Menu_SpawnItemAmbiguity_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
	{
		delete hMenu;
	}
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sParts[3][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, "/", sParts, 3, 16);

		int  iTarget    = GetClientOfUserId(StringToInt(sParts[0]));
		int  iItemIndex = StringToInt(sParts[1]);
		bool bStrip     = (StringToInt(sParts[2]) == 1);

		if (!iTarget || !IsClientInGame(iTarget))
		{
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
			return 0;
		}

		SpawnItemForClient(iClient, iTarget, iItemIndex, bStrip);
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Native - EW_SpawnItem(int iAdmin, int iItemID, int iReceiver, bool bStrip)
//----------------------------------------------------------------------------------------------------
public int Native_SpawnItem(Handle hPlugin, int iNumParams)
{
	int  iAdmin    = GetNativeCell(1);
	int  iItemID   = GetNativeCell(2);
	int  iReceiver = GetNativeCell(3);
	bool bStrip    = view_as<bool>(GetNativeCell(4));

	return SpawnItemForClient(iAdmin, iReceiver, iItemID, bStrip);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Accessor for spawnable items array - returns an ArrayList of CConfig handles for
//          configs with valid spawner entries. Used by menu interface to show spawnable items.
//          Note that this is NOT the same as the items array returned by EW_GetItemsArray(), which
//          contains CItem instances for currently tracked entities.
//----------------------------------------------------------------------------------------------------
stock ArrayList Ew4_Spawn_GetSpawnableItems()
{
	return s_hSpawnableItems;
}
