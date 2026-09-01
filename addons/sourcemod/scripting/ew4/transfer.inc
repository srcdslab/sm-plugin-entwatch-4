//====================================================================================================
//
// Name: [entWatch] Transfer
// Author: zaCade, Prometheum, koen, .Rushaway
// Description: Item transfer logic — native implementation and admin command
//
//====================================================================================================

/* FORWARDS */
GlobalForward g_hFwd_OnAdminTransferItem;

//----------------------------------------------------------------------------------------------------
// Purpose: Register natives and library before anything else loads
//----------------------------------------------------------------------------------------------------
public void Ew4_Transfer_AskPluginLoad2()
{
	CreateNative("EW_TransferItem", Native_TransferItem);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Transfer_OnPluginStart()
{
	g_hFwd_OnAdminTransferItem = new GlobalForward("EW_OnAdminTransferItem", ET_Ignore, Param_Cell, Param_Cell, Param_Cell);

	RegAdminCmd("sm_etransfer", Command_TransferItem, ADMFLAG_BAN);

	EW_SDK_Load();
}

//----------------------------------------------------------------------------------------------------
// Purpose: Core transfer logic.
//          Handles both "item held by a player" and "item on the ground" cases.
//          Returns true on success, false on any validation failure.
//----------------------------------------------------------------------------------------------------
bool Internal_TransferItem(int iAdmin, int iItemID, int iReceiver, bool bMessage)
{
	ArrayList hItems = EW_GetItemsArray();

	if (!hItems || iItemID < 0 || iItemID >= hItems.Length)
		return false;

	if (iReceiver < 1 || iReceiver > MaxClients || !IsClientInGame(iReceiver))
		return false;

	CItem hItem = hItems.Get(iItemID);

	if (hItem.iWeapon == INVALID_ENT_REFERENCE || !IsValidEntity(hItem.iWeapon))
		return false;

	char sName[32];
	hItem.hConfig.GetName(sName, sizeof(sName));

	char sItemColor[8];
	hItem.hConfig.GetColor(sItemColor, sizeof(sItemColor));

	if (hItem.iClient != INVALID_ENT_REFERENCE)
	{
		// ---- Item is held by a player ----
		int iTarget = hItem.iClient;

		if (GetClientTeam(iTarget) != GetClientTeam(iReceiver))
			return false;

		if (iTarget == iReceiver)
			return false;

		SDKHooks_DropWeapon(iTarget, hItem.iWeapon, NULL_VECTOR, NULL_VECTOR, false);

		char sWeaponClass[32];
		GetEntityClassname(hItem.iWeapon, sWeaponClass, sizeof(sWeaponClass));
		GivePlayerItem(iTarget, sWeaponClass);

		FixedEquipPlayerWeapon(iReceiver, hItem.iWeapon);

		if (bMessage)
		{
			CPrintToChatAll("{#%s}%t {#%s}%N {#%s}%t {#%s}%s {#%s}%t {#%s}%N {#%s}%t {#%s}%N",
				g_clr.sTag,     "EW_Tag",
				g_clr.sName,    iAdmin,
				g_clr.sWarning, "Item Transfer",
				sItemColor,     sName,
				g_clr.sWarning, "EW_For",
				g_clr.sName,    iTarget,
				g_clr.sWarning, "EW_To",
				g_clr.sName,    iReceiver);
			LogAction(iAdmin, iTarget, "%L transfered %s from %L to %L.", iAdmin, sName, iTarget, iReceiver);
		}
	}
	else
	{
		// ---- Item is on the ground ----
		FixedEquipPlayerWeapon(iReceiver, hItem.iWeapon);

		if (bMessage)
		{
			CPrintToChatAll("{#%s}%t {#%s}%N {#%s}%t {#%s}%s {#%s}%t {#%s}%N",
				g_clr.sTag,     "EW_Tag",
				g_clr.sName,    iAdmin,
				g_clr.sWarning, "Item Transfer",
				sItemColor,     sName,
				g_clr.sWarning, "EW_To",
				g_clr.sName,    iReceiver);
			LogAction(iAdmin, -1, "%L transfered %s to %L.", iAdmin, sName, iReceiver);
		}
	}

	// Fire forward so third-party plugins can react
	Call_StartForward(g_hFwd_OnAdminTransferItem);
	Call_PushCell(iAdmin);
	Call_PushCell(hItem);
	Call_PushCell(iReceiver);
	Call_Finish();

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Admin command — sm_etransfer <#userid/name/$itemname> <#userid/name>
//
//   $itemname  → transfer the first item whose name/short matches the pattern
//   player arg → transfer ALL items held by that player to the receiver
//----------------------------------------------------------------------------------------------------
public Action Command_TransferItem(int iClient, int iArgs)
{
	if (GetCmdArgs() < 2)
	{
		CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Transfer_Usage");
		return Plugin_Handled;
	}

	char sArguments[2][32];
	GetCmdArg(1, sArguments[0], sizeof(sArguments[]));
	GetCmdArg(2, sArguments[1], sizeof(sArguments[]));

	int iReceiver;
	if ((iReceiver = FindTarget(iClient, sArguments[1], true)) == -1)
		return Plugin_Handled;

	// ---- Transfer by item name/short ($itemname) ----
	if (strncmp(sArguments[0], "$", 1, false) == 0)
	{
		strcopy(sArguments[0], sizeof(sArguments[]), sArguments[0][1]);

		ArrayList hItems = EW_GetItemsArray();
		bool bTransfered;

		for (int iItemID; iItemID < hItems.Length; iItemID++)
		{
			CItem hItem = hItems.Get(iItemID);

			char sName[32], sShort[32];
			hItem.hConfig.GetName(sName, sizeof(sName));
			hItem.hConfig.GetShort(sShort, sizeof(sShort));

			if (StrContains(sName, sArguments[0], false) != -1 || StrContains(sShort, sArguments[0], false) != -1)
			{
				if (Internal_TransferItem(iClient, iItemID, iReceiver, true))
				{
					bTransfered = true;
					break;
				}
			}
		}

		if (!bTransfered)
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Transfer_NoTransferableItems");
	}
	else
	{
		// ---- Transfer ALL items from a source player ----
		int iTarget;
		if ((iTarget = FindTarget(iClient, sArguments[0], true)) == -1)
			return Plugin_Handled;

		ArrayList hItems = EW_GetItemsArray();
		bool bTransfered;

		for (int iItemID; iItemID < hItems.Length; iItemID++)
		{
			CItem hItem = hItems.Get(iItemID);

			if (hItem.iClient != INVALID_ENT_REFERENCE && hItem.iClient == iTarget)
			{
				if (Internal_TransferItem(iClient, iItemID, iReceiver, false))
					bTransfered = true;
			}
		}

		if (!bTransfered)
			CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Transfer_TargetNoTransferableItems");
		else
			CPrintToChatAll("{#%s}%t {#%s}%N {#%s}%t {#%s}%N {#%s}%t {#%s}%N",
				g_clr.sTag,     "EW_Tag",
				g_clr.sName,    iClient,
				g_clr.sWarning, "Item Transfer All From",
				g_clr.sName,    iTarget,
				g_clr.sWarning, "EW_To",
				g_clr.sName,    iReceiver);
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Equip a weapon on a client, properly replacing any existing weapon in the same slot.
//          Uses SDK calls to correctly trigger OnPickedUp so entWatch core tracks ownership.
//----------------------------------------------------------------------------------------------------
stock void FixedEquipPlayerWeapon(int iClient, int iWeapon)
{
	int iWeaponSlot   = SDKCall(SDKCall_GetSlot, iWeapon);
	int iWeaponInSlot = GetPlayerWeaponSlot(iClient, iWeaponSlot);

	if (iWeaponInSlot != -1)
		SDKHooks_DropWeapon(iClient, iWeaponInSlot, NULL_VECTOR, NULL_VECTOR, false);

	if (SDKCall(SDKCall_BumpWeapon, iClient, iWeapon))
		SDKCall(SDKCall_OnPickedUp, iWeapon, iClient);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Native — EW_TransferItem(int iAdmin, int iItemID, int iReceiver, bool bMessage)
//----------------------------------------------------------------------------------------------------
public int Native_TransferItem(Handle hPlugin, int iNumParams)
{
	int  iAdmin    = GetNativeCell(1);
	int  iItemID   = GetNativeCell(2);
	int  iReceiver = GetNativeCell(3);
	bool bMessage  = view_as<bool>(GetNativeCell(4));

	return Internal_TransferItem(iAdmin, iItemID, iReceiver, bMessage);
}
