//====================================================================================================
//
// Name: [entWatch] Interface - Admin Menus
// Author: zaCade, Prometheum, koen, .Rushaway
// Description: All admin menu handlers for [entWatch] interface.
//
// Included by: entWatch_interface.sp
//
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: Register the entWatch category and its items in the SourceMod admin top-menu.
//          Called from OnAdminMenuReady() in the main plugin.
//----------------------------------------------------------------------------------------------------
void EWM_RegisterAdminMenu(TopMenu hTopMenu)
{
	TopMenuObject hMenuObj = AddToTopMenu(hTopMenu, "EntWatch_Commands", TopMenuObject_Category, EWM_Menu_AM_Handler, INVALID_TOPMENUOBJECT);

	if (hMenuObj == INVALID_TOPMENUOBJECT)
		return;

	AddToTopMenu(hTopMenu, "EntWatch_ItemList", TopMenuObject_Item, EWM_Menu_AM_ItemList_Handler, hMenuObj, "sm_eitemlist", ADMFLAG_BAN);

	#if defined EW4_RESTRICTIONS
	AddToTopMenu(hTopMenu, "EntWatch_EBanList",   TopMenuObject_Item, EWM_Menu_AM_EBanList_Handler,   hMenuObj, "sm_eban",   ADMFLAG_BAN);
	AddToTopMenu(hTopMenu, "EntWatch_EUnBanList", TopMenuObject_Item, EWM_Menu_AM_EUnBanList_Handler, hMenuObj, "sm_eunban", ADMFLAG_BAN);
	#endif

	#if defined EW4_SPAWN_ITEMS
	AddToTopMenu(hTopMenu, "EntWatch_SpawnItemList", TopMenuObject_Item, EWM_Menu_AM_SpawnItemList_Handler, hMenuObj, "sm_espawnitem", ADMFLAG_BAN);
	#endif
}

//====================================================================================================
// TOP MENU HANDLERS
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: Category header
//----------------------------------------------------------------------------------------------------
public void EWM_Menu_AM_Handler(TopMenu hMenu, TopMenuAction hAction, TopMenuObject hObj, int iClient, char[] sBuffer, int iMaxLen)
{
	if (hAction == TopMenuAction_DisplayTitle)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_Category_Title");
	else if (hAction == TopMenuAction_DisplayOption)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_Category_Title");
}

//----------------------------------------------------------------------------------------------------
// Purpose: Item list entry
//----------------------------------------------------------------------------------------------------
public void EWM_Menu_AM_ItemList_Handler(TopMenu hMenu, TopMenuAction hAction, TopMenuObject hObj, int iClient, char[] sBuffer, int iMaxLen)
{
	if (hAction == TopMenuAction_DisplayOption)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_ItemList");
	else if (hAction == TopMenuAction_SelectOption)
		EWM_Menu_PM_ItemList(iClient);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Restrict player entry
//----------------------------------------------------------------------------------------------------
public void EWM_Menu_AM_EBanList_Handler(TopMenu hMenu, TopMenuAction hAction, TopMenuObject hObj, int iClient, char[] sBuffer, int iMaxLen)
{
	if (hAction == TopMenuAction_DisplayOption)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_EBanList");
	else if (hAction == TopMenuAction_SelectOption)
		EWM_Menu_PM_EBanList(iClient);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Restriction list entry
//----------------------------------------------------------------------------------------------------
public void EWM_Menu_AM_EUnBanList_Handler(TopMenu hMenu, TopMenuAction hAction, TopMenuObject hObj, int iClient, char[] sBuffer, int iMaxLen)
{
	if (hAction == TopMenuAction_DisplayOption)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_EUnBanList");
	else if (hAction == TopMenuAction_SelectOption)
		EWM_Menu_PM_EUnBanList(iClient);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Spawn item entry
//----------------------------------------------------------------------------------------------------
public void EWM_Menu_AM_SpawnItemList_Handler(TopMenu hMenu, TopMenuAction hAction, TopMenuObject hObj, int iClient, char[] sBuffer, int iMaxLen)
{
	if (hAction == TopMenuAction_DisplayOption)
		Format(sBuffer, iMaxLen, "%t", "EW_AdminMenu_SpawnItem");
	else if (hAction == TopMenuAction_SelectOption)
		EWM_Menu_PM_SpawnItemList(iClient);
}

//====================================================================================================
// ITEM LIST + INFO + TRANSFER
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: Show all transferable items with their current owner
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_ItemList(int iClient)
{
	ArrayList hItems = EW_GetItemsArray();

	if (!hItems)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_NoItems");
		return;
	}

	Menu hMenu = new Menu(EWM_Menu_PM_ItemList_Handler);
	char sTitle_ItemList[64];
	Format(sTitle_ItemList, sizeof(sTitle_ItemList), "%t", "EW_PM_ItemList_Title");
	hMenu.SetTitle(sTitle_ItemList);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	for (int iItemID; iItemID < hItems.Length; iItemID++)
	{
		CItem hItem = hItems.Get(iItemID);

		if (!hItem.hConfig.bAllowTransfer)
			continue;

		char sName[EW_MAX_NAME_LENGTH], sDisplay[64], sID[10];
		hItem.hConfig.GetName(sName, sizeof(sName));
		IntToString(iItemID, sID, sizeof(sID));

		if (hItem.iClient != INVALID_ENT_REFERENCE)
			Format(sDisplay, sizeof(sDisplay), "%s (%N)", sName, hItem.iClient);
		else
			Format(sDisplay, sizeof(sDisplay), "%s (%t)", sName, "EW_PM_ItemList_NoOwner");

		hMenu.AddItem(sID, sDisplay);
		bFound = true;
	}

	if (!bFound)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_NoItems");
		delete hMenu;
		return;
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_ItemList_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		g_hAdminMenu.Display(iClient, TopMenuPosition_LastCategory);
	else if (hAction == MenuAction_Select)
	{
		char sID[10];
		hMenu.GetItem(iParam2, sID, sizeof(sID));
		EWM_Menu_PM_ItemInfo(iClient, StringToInt(sID));
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Per-item detail menu - Restrict Player / Transfer Item
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_ItemInfo(int iClient, int iItemID)
{
	ArrayList hItems = EW_GetItemsArray();

	if (!hItems || iItemID < 0 || iItemID >= hItems.Length)
		return;

	CItem hItem = hItems.Get(iItemID);

	char sName[EW_MAX_NAME_LENGTH], sTitle[64];
	hItem.hConfig.GetName(sName, sizeof(sName));
	Format(sTitle, sizeof(sTitle), "%t %s", "EW_PM_ItemInfo_TitlePrefix", sName);

	Menu hMenu = new Menu(EWM_Menu_PM_ItemInfo_Handler);
	hMenu.SetTitle(sTitle);
	hMenu.ExitBackButton = true;

	// Restrict option - only if item has an owner and restrictions plugin is loaded
	if (hItem.iClient != INVALID_ENT_REFERENCE && LibraryExists("entWatch-restrictions"))
	{
		char sEbanInfo[32];
		Format(sEbanInfo, sizeof(sEbanInfo), "eban;%d", GetClientUserId(hItem.iClient));

		char sRestrictLabel[64];
		Format(sRestrictLabel, sizeof(sRestrictLabel), "%t", "EW_PM_ItemInfo_RestrictPlayer");
		hMenu.AddItem(sEbanInfo, sRestrictLabel);
	}

	char sTransferInfo[32];
	Format(sTransferInfo, sizeof(sTransferInfo), "transfer;%d", iItemID);

	char sTransferLabel[64];
	Format(sTransferLabel, sizeof(sTransferLabel), "%t", "EW_PM_ItemInfo_TransferItem");
	hMenu.AddItem(sTransferInfo, sTransferLabel);

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_ItemInfo_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_ItemList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sSplit[2][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, ";", sSplit, 2, 16);

		if (strcmp(sSplit[0], "eban") == 0)
		{
			int iTarget = GetClientOfUserId(StringToInt(sSplit[1]));

			if (iTarget && IsClientInGame(iTarget))
				EWM_Menu_PM_EBanDuration(iClient, iTarget);
			else
				CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
		}
		else if (strcmp(sSplit[0], "transfer") == 0)
		{
			EWM_Menu_PM_ItemTransfer(iClient, StringToInt(sSplit[1]));
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Select a receiver to transfer the item to
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_ItemTransfer(int iClient, int iItemID)
{
	ArrayList hItems = EW_GetItemsArray();

	if (!hItems || iItemID < 0 || iItemID >= hItems.Length)
		return;

	CItem hItem = hItems.Get(iItemID);

	Menu hMenu = new Menu(EWM_Menu_PM_ItemTransfer_Handler);
	char sNoPlayers_Transfer[64];
	Format(sNoPlayers_Transfer, sizeof(sNoPlayers_Transfer), "%t", "EW_PM_Transfer_NoValidPlayers");
	hMenu.AddItem("", sNoPlayers_Transfer, ITEMDRAW_DISABLED);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || !IsPlayerAlive(i) || i == hItem.iClient)
			continue;

		if (LibraryExists("entWatch-restrictions") && EW_IsRestrictedClient(i))
			continue;

		if (hItem.iClient != INVALID_ENT_REFERENCE && GetClientTeam(i) != GetClientTeam(hItem.iClient))
			continue;

		char sInfo[32], sName[MAX_NAME_LENGTH];
		Format(sInfo, sizeof(sInfo), "%d;%d", iItemID, GetClientUserId(i));
		GetClientName(i, sName, sizeof(sName));
		hMenu.AddItem(sInfo, sName);
		bFound = true;
	}

	if (!bFound)
	{
		char sNoPlayers_SpawnTarget[64];
		Format(sNoPlayers_SpawnTarget, sizeof(sNoPlayers_SpawnTarget), "%t", "EW_PM_SpawnTarget_NoPlayers");
		hMenu.AddItem("", sNoPlayers_SpawnTarget, ITEMDRAW_DISABLED);
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_ItemTransfer_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_ItemList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sSplit[2][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, ";", sSplit, 2, 16);

		int iItemID   = StringToInt(sSplit[0]);
		int iReceiver = GetClientOfUserId(StringToInt(sSplit[1]));

		if (iReceiver == 0)
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_ReceiverDisconnected");
		else if (!EW_TransferItem(iClient, iItemID, iReceiver))
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_TransferFailed");

		// Always return to item list after action
		EWM_Menu_PM_ItemList(iClient);
	}

	return 0;
}

//====================================================================================================
// RESTRICT PLAYER (EBAN)
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: List of unrestricted players to restrict
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_EBanList(int iClient)
{
	Menu hMenu = new Menu(EWM_Menu_PM_EBanList_Handler);
	char sTitle_EBanList[64];
	Format(sTitle_EBanList, sizeof(sTitle_EBanList), "%t", "EW_PM_EBanList_Title");
	hMenu.SetTitle(sTitle_EBanList);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i) || EW_IsRestrictedClient(i))
			continue;

		char sUserId[16], sName[64];
		IntToString(GetClientUserId(i), sUserId, sizeof(sUserId));
		Format(sName, sizeof(sName), "%N (#%d)", i, GetClientUserId(i));
		hMenu.AddItem(sUserId, sName);
		bFound = true;
	}

	if (!bFound)
	{
		char sNoPlayers_EBan[64];
		Format(sNoPlayers_EBan, sizeof(sNoPlayers_EBan), "%t", "EW_PM_EBanList_NoPlayers");
		hMenu.AddItem("", sNoPlayers_EBan, ITEMDRAW_DISABLED);
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_EBanList_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		g_hAdminMenu.Display(iClient, TopMenuPosition_LastCategory);
	else if (hAction == MenuAction_Select)
	{
		char sUserId[16];
		hMenu.GetItem(iParam2, sUserId, sizeof(sUserId));
		int iTarget = GetClientOfUserId(StringToInt(sUserId));

		if (iTarget)
			EWM_Menu_PM_EBanDuration(iClient, iTarget);
		else
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Duration picker for a new restriction
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_EBanDuration(int iClient, int iTarget)
{
	Menu hMenu = new Menu(EWM_Menu_PM_EBanDuration_Handler);

	char sTitle_Dur[64];
	Format(sTitle_Dur, sizeof(sTitle_Dur), "%t", "EW_PM_EBanDuration_Title", iTarget);
	hMenu.SetTitle(sTitle_Dur);
	hMenu.ExitBackButton = true;

	char sTargetId[16];
	IntToString(GetClientUserId(iTarget), sTargetId, sizeof(sTargetId));

	// Format: "<duration_minutes>;<userid>"
	// -1 = session, 0 = permanent
	char sDurSession[64];
	Format(sDurSession, sizeof(sDurSession), "%t", "EW_PM_EBanDuration_Session");

	char sInfo[64];
	Format(sInfo, sizeof(sInfo), "-1;%s", sTargetId);
	hMenu.AddItem(sInfo, sDurSession);

	char sDur10m[32], sDur1h[32], sDur1d[32], sDur1w[32], sDur1mo[32], sDurPerm[32];
	Format(sDur10m,  sizeof(sDur10m),  "%t", "EW_Menu_OfflineDuration_10min");
	Format(sDur1h,   sizeof(sDur1h),   "%t", "EW_Menu_OfflineDuration_1Hour");
	Format(sDur1d,   sizeof(sDur1d),   "%t", "EW_Menu_OfflineDuration_1Day");
	Format(sDur1w,   sizeof(sDur1w),   "%t", "EW_Menu_OfflineDuration_1Week");
	Format(sDur1mo,  sizeof(sDur1mo),  "%t", "EW_Menu_OfflineDuration_1Month");
	Format(sDurPerm, sizeof(sDurPerm), "%t", "EW_Menu_OfflineDuration_Permanent");

	Format(sInfo, sizeof(sInfo), "10;%s", sTargetId);
	hMenu.AddItem(sInfo, sDur10m);
	Format(sInfo, sizeof(sInfo), "60;%s", sTargetId);
	hMenu.AddItem(sInfo, sDur1h);
	Format(sInfo, sizeof(sInfo), "1440;%s", sTargetId);
	hMenu.AddItem(sInfo, sDur1d);
	Format(sInfo, sizeof(sInfo), "10080;%s", sTargetId);
	hMenu.AddItem(sInfo, sDur1w);
	Format(sInfo, sizeof(sInfo), "40320;%s", sTargetId);
	hMenu.AddItem(sInfo, sDur1mo);
	Format(sInfo, sizeof(sInfo), "0;%s", sTargetId);
	hMenu.AddItem(sInfo, sDurPerm);

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_EBanDuration_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_EBanList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sSplit[2][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, ";", sSplit, 2, 16);

		int iDuration = StringToInt(sSplit[0]);
		int iTarget   = GetClientOfUserId(StringToInt(sSplit[1]));

		if (iTarget)
			EW_ShowBanReasonMenu(iClient, iTarget, iDuration);
		else
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
	}

	return 0;
}

//====================================================================================================
// UNRESTRICT PLAYER (EUNBAN)
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: List of currently restricted players
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_EUnBanList(int iClient)
{
	Menu hMenu = new Menu(EWM_Menu_PM_EUnBanList_Handler);
	char sTitle_EUnBanList[64];
	Format(sTitle_EUnBanList, sizeof(sTitle_EUnBanList), "%t", "EW_PM_EUnBanList_Title");
	hMenu.SetTitle(sTitle_EUnBanList);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i) || !EW_IsRestrictedClient(i))
			continue;

		char sUserId[16], sName[64];
		IntToString(GetClientUserId(i), sUserId, sizeof(sUserId));
		Format(sName, sizeof(sName), "%N (#%d)", i, GetClientUserId(i));
		hMenu.AddItem(sUserId, sName);
		bFound = true;
	}

	if (!bFound)
	{
		char sNoPlayers_EUnBan[64];
		Format(sNoPlayers_EUnBan, sizeof(sNoPlayers_EUnBan), "%t", "EW_PM_EUnBanList_NoPlayers");
		hMenu.AddItem("", sNoPlayers_EUnBan, ITEMDRAW_DISABLED);
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_EUnBanList_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		g_hAdminMenu.Display(iClient, TopMenuPosition_LastCategory);
	else if (hAction == MenuAction_Select)
	{
		char sUserId[16];
		hMenu.GetItem(iParam2, sUserId, sizeof(sUserId));
		int iTarget = GetClientOfUserId(StringToInt(sUserId));

		if (iTarget)
			EWM_Menu_PM_EBanInfo(iClient, iTarget);
		else
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Show restriction details + unrestrict button
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_EBanInfo(int iClient, int iTarget)
{
	char sAdminName[MAX_NAME_LENGTH], sAdminSteamID[EW_MAX_STEAMID_LENGTH], sReason[EW_MAX_REASON_LENGTH];
	int  iDuration, iIssuedAt, iTotal;

	if (EW_GetClientBanInfo(iTarget, sAdminName, sizeof(sAdminName), sAdminSteamID, sizeof(sAdminSteamID), sReason, sizeof(sReason), iDuration, iIssuedAt, iTotal) <= 0)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_BanInfoFailed");
		return;
	}

	Menu hMenu = new Menu(EWM_Menu_PM_EBanInfo_Handler);

	char sTitle_BanInfo[64];
	Format(sTitle_BanInfo, sizeof(sTitle_BanInfo), "%t", "EW_PM_EBanInfo_Title", iTarget);
	hMenu.SetTitle(sTitle_BanInfo);
	hMenu.ExitBackButton = true;

	char sLine[128];

	Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_Admin", sAdminName);
	hMenu.AddItem("", sLine, ITEMDRAW_DISABLED);

	if (iDuration == -1)
		Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_Duration_Session");
	else if (iDuration == 0)
		Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_Duration_Permanent");
	else
		Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_Duration_Timed", iDuration);
	hMenu.AddItem("", sLine, ITEMDRAW_DISABLED);

	Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_Reason", sReason);
	hMenu.AddItem("", sLine, ITEMDRAW_DISABLED);

	Format(sLine, sizeof(sLine), "%t", "EW_PM_EBanInfo_TotalEbans", iTotal);
	hMenu.AddItem("", sLine, ITEMDRAW_DISABLED);

	char sTargetId[16];
	IntToString(GetClientUserId(iTarget), sTargetId, sizeof(sTargetId));

	char sUnrestrict[64];
	Format(sUnrestrict, sizeof(sUnrestrict), "%t", "EW_PM_EBanInfo_UnrestrictButton");
	hMenu.AddItem(sTargetId, sUnrestrict);

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_EBanInfo_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_EUnBanList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sUserId[16];
		hMenu.GetItem(iParam2, sUserId, sizeof(sUserId));
		int iTarget = GetClientOfUserId(StringToInt(sUserId));

		if (iTarget)
			EW_ShowUnbanReasonMenu(iClient, iTarget);
		else
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
	}

	return 0;
}

//====================================================================================================
// SPAWN ITEM
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: List all spawnable items (those with a non-empty spawner config)
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_SpawnItemList(int iClient)
{
	Menu hMenu = new Menu(EWM_Menu_PM_SpawnItemList_Handler);
	char sTitle_SpawnList[64];
	Format(sTitle_SpawnList, sizeof(sTitle_SpawnList), "%t", "EW_PM_SpawnItemList_Title");
	hMenu.SetTitle(sTitle_SpawnList);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	#if defined EW4_SPAWN_ITEMS
	ArrayList hSpawnable = Ew4_Spawn_GetSpawnableItems();

	if (hSpawnable)
	{
		for (int i = 0; i < hSpawnable.Length; i++)
		{
			CConfig hConfig = hSpawnable.Get(i);

			char sName[EW_MAX_NAME_LENGTH], sID[16];
			hConfig.GetName(sName, sizeof(sName));
			IntToString(i, sID, sizeof(sID));
			hMenu.AddItem(sID, sName);
			bFound = true;
		}
	}
	#endif

	if (!bFound)
	{
		char sNoItems_Spawn[64];
		Format(sNoItems_Spawn, sizeof(sNoItems_Spawn), "%t", "EW_PM_SpawnItemList_NoItems");
		hMenu.AddItem("", sNoItems_Spawn, ITEMDRAW_DISABLED);
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_SpawnItemList_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		g_hAdminMenu.Display(iClient, TopMenuPosition_LastCategory);
	else if (hAction == MenuAction_Select)
	{
		char sID[16];
		hMenu.GetItem(iParam2, sID, sizeof(sID));
		EWM_Menu_PM_SpawnItemStrip(iClient, StringToInt(sID));
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Ask whether to strip the receiver's weapons before spawning
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_SpawnItemStrip(int iClient, int iItemIndex)
{
	Menu hMenu = new Menu(EWM_Menu_PM_SpawnItemStrip_Handler);
	char sTitle_Strip[64];
	Format(sTitle_Strip, sizeof(sTitle_Strip), "%t", "EW_PM_SpawnStrip_Title");
	hMenu.SetTitle(sTitle_Strip);
	hMenu.ExitBackButton = true;

	char sInfo[64], sStripYes[64], sStripNo[64];
	Format(sStripYes, sizeof(sStripYes), "%t", "EW_PM_SpawnStrip_Yes");
	Format(sStripNo,  sizeof(sStripNo),  "%t", "EW_PM_SpawnStrip_No");
	Format(sInfo, sizeof(sInfo), "%d/1", iItemIndex);
	hMenu.AddItem(sInfo, sStripYes);
	Format(sInfo, sizeof(sInfo), "%d/0", iItemIndex);
	hMenu.AddItem(sInfo, sStripNo);

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_SpawnItemStrip_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_SpawnItemList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sParts[2][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, "/", sParts, 2, 16);

		int iItemIndex = StringToInt(sParts[0]);
		int iStrip     = StringToInt(sParts[1]);

		EWM_Menu_PM_SpawnItemTarget(iClient, iItemIndex, iStrip);
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Select which player receives the spawned item
//----------------------------------------------------------------------------------------------------
void EWM_Menu_PM_SpawnItemTarget(int iClient, int iItemIndex, int iStrip)
{
	Menu hMenu = new Menu(EWM_Menu_PM_SpawnItemTarget_Handler);
	char sTitle_SpawnTarget[64];
	Format(sTitle_SpawnTarget, sizeof(sTitle_SpawnTarget), "%t", "EW_PM_SpawnTarget_Title");
	hMenu.SetTitle(sTitle_SpawnTarget);
	hMenu.ExitBackButton = true;

	bool bFound = false;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i))
			continue;

		if (GetClientTeam(i) != 2 && GetClientTeam(i) != 3)
			continue;

		if (LibraryExists("entWatch-restrictions") && EW_IsRestrictedClient(i))
			continue;

		char sInfo[32], sName[MAX_NAME_LENGTH];
		Format(sInfo, sizeof(sInfo), "%d/%d/%d", iItemIndex, iStrip, GetClientUserId(i));
		GetClientName(i, sName, sizeof(sName));
		hMenu.AddItem(sInfo, sName);
		bFound = true;
	}

	if (!bFound)
	{
		char sNoPlayers_SpawnTarget[64];
		Format(sNoPlayers_SpawnTarget, sizeof(sNoPlayers_SpawnTarget), "%t", "EW_PM_SpawnTarget_NoPlayers");
		hMenu.AddItem("", sNoPlayers_SpawnTarget, ITEMDRAW_DISABLED);
	}

	hMenu.Display(iClient, MENU_TIME_FOREVER);
}

public int EWM_Menu_PM_SpawnItemTarget_Handler(Menu hMenu, MenuAction hAction, int iClient, int iParam2)
{
	if (hAction == MenuAction_End)
		delete hMenu;
	else if (hAction == MenuAction_Cancel && iParam2 == MenuCancel_ExitBack)
		EWM_Menu_PM_SpawnItemList(iClient);
	else if (hAction == MenuAction_Select)
	{
		char sInfo[32], sParts[3][16];
		hMenu.GetItem(iParam2, sInfo, sizeof(sInfo));
		ExplodeString(sInfo, "/", sParts, 3, 16);

		int  iItemIndex = StringToInt(sParts[0]);
		bool bStrip     = (StringToInt(sParts[1]) == 1);
		int  iTarget    = GetClientOfUserId(StringToInt(sParts[2]));

		if (!iTarget || !IsClientInGame(iTarget))
		{
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_PlayerNoLongerValid");
			EWM_Menu_PM_SpawnItemList(iClient);
			return 0;
		}

		if (!EW_SpawnItem(iClient, iItemIndex, iTarget, bStrip))
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Menu_SpawnFailed");
	}

	return 0;
}
