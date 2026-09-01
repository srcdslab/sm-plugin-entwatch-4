//====================================================================================================
//
// Name: [entWatch] Override Item
// Author: .Rushaway
// Description: Override item functions of [entWatch]
//
//====================================================================================================
// Recommended Sourcemod Version: 1.12.0.7225 or above
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: Register the sm_ewset* / sm_ewlockbutton admin commands
//----------------------------------------------------------------------------------------------------
public void Ew4_OverrideItem_OnPluginStart()
{
	RegAdminCmd("sm_ewsetcooldown",  Command_SetCooldown,    ADMFLAG_CONFIG, "Set button cooldown by HammerID");
	RegAdminCmd("sm_ewsetmaxuses",   Command_SetMaxUses,     ADMFLAG_CONFIG, "Set button max uses by HammerID");
	RegAdminCmd("sm_ewsetmode",      Command_SetMode,        ADMFLAG_CONFIG, "Set button mode by HammerID");
	RegAdminCmd("sm_ewsetname",      Command_SetName,        ADMFLAG_CONFIG, "Set item display name by HammerID");
	RegAdminCmd("sm_ewsetshortname", Command_SetShortName,   ADMFLAG_CONFIG, "Set item short name by HammerID");
	RegAdminCmd("sm_ewlockbutton",   Command_LockButton,     ADMFLAG_CONFIG, "Lock/unlock button by HammerID");
}

public Action Command_SetCooldown(int iClient, int iArgs)
{
	EW4_SetCooldown(iClient, iArgs);
	return Plugin_Handled;
}

public Action Command_SetMaxUses(int iClient, int iArgs)
{
	EW4_SetMaxUses(iClient, iArgs);
	return Plugin_Handled;
}

public Action Command_SetMode(int iClient, int iArgs)
{
	EW4_SetMode(iClient, iArgs);
	return Plugin_Handled;
}

public Action Command_LockButton(int iClient, int iArgs)
{
	EW4_LockButton(iClient, iArgs);
	return Plugin_Handled;
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Set button cooldown by HammerID
// ----------------------------------------------------------------------------------------------------
stock void EW4_SetCooldown(int iClient, int iArgs)
{
	if (iArgs < 2)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_SetCooldown_Usage");
		return;
	}

	char sHammerID[16], sCooldown[16];
	GetCmdArg(1, sHammerID, sizeof(sHammerID));
	GetCmdArg(2, sCooldown, sizeof(sCooldown));

	int iHammerID = StringToInt(sHammerID);
	CItemButton hBtn = EW4_GetButton(iHammerID);

	if (hBtn == null)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_NoButton", iHammerID);
		return;
	}

	float flCooldown = StringToFloat(sCooldown);
	if (flCooldown < 0.0)
		flCooldown = 0.0;

	bool bForce = false;
	if (iArgs >= 3)
	{
		char sForce[4];
		GetCmdArg(3, sForce, sizeof(sForce));
		bForce = StringToInt(sForce) == 1;
	}

	hBtn.hConfigButton.flButtonCooldown = flCooldown;
	if (bForce)
		hBtn.flReadyTime = GetGameTime() + flCooldown;

	char sName[32];
	hBtn.hConfigButton.hConfig.GetName(sName, sizeof(sName));
	CReplyToCommand(iClient, "%t", "EW_Override_SetCooldown_Done", sName, flCooldown);
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Set button max uses by HammerID
// ----------------------------------------------------------------------------------------------------
stock void EW4_SetMaxUses(int iClient, int iArgs)
{
	if (iArgs < 2)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_SetMaxUses_Usage");
		return;
	}

	char sHammerID[16], sMaxUses[16];
	GetCmdArg(1, sHammerID, sizeof(sHammerID));
	GetCmdArg(2, sMaxUses,  sizeof(sMaxUses));

	int iHammerID = StringToInt(sHammerID);
	int iMaxUses  = StringToInt(sMaxUses);
	if (iMaxUses < 0) iMaxUses = 0;

	CItemButton hBtn = EW4_GetButton(iHammerID);
	if (hBtn == null)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_NoButton", iHammerID);
		return;
	}

	hBtn.hConfigButton.iMaxUses = iMaxUses;

	char sName[32];
	hBtn.hConfigButton.hConfig.GetName(sName, sizeof(sName));
	CReplyToCommand(iClient, "%t", "EW_Override_SetMaxUses_Done", sName, iMaxUses);
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Set button mode by HammerID
// ----------------------------------------------------------------------------------------------------
stock void EW4_SetMode(int iClient, int iArgs)
{
	if (iArgs < 5)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_SetMode_Usage");
		return;
	}

	char sHammerID[16], sType[8], sMode[8], sCooldown[16], sMaxUses[16];
	GetCmdArg(1, sHammerID, sizeof(sHammerID));
	GetCmdArg(2, sType,     sizeof(sType));
	GetCmdArg(3, sMode,     sizeof(sMode));
	GetCmdArg(4, sCooldown, sizeof(sCooldown));
	GetCmdArg(5, sMaxUses,  sizeof(sMaxUses));

	int   iHammerID = StringToInt(sHammerID);
	int   iType     = StringToInt(sType);
	int   iMode     = StringToInt(sMode);
	float flCooldown      = StringToFloat(sCooldown);
	int   iMaxUses  = StringToInt(sMaxUses);

	if (iType < 0 || iType > 4)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_InvalidType");
		return;
	}

	if (iMode < 0 || iMode > 4)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_InvalidMode");
		return;
	}

	CItemButton hBtn = EW4_GetButton(iHammerID);
	if (hBtn == null)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_NoButton", iHammerID);
		return;
	}

	if (flCooldown < 0.0)
		flCooldown = 0.0;
	if (iMaxUses < 0)
		iMaxUses = 0;

	hBtn.hConfigButton.iType            = iType;
	hBtn.hConfigButton.iMode            = iMode;
	hBtn.hConfigButton.flButtonCooldown = flCooldown;
	hBtn.hConfigButton.iMaxUses         = iMaxUses;

	char sName[32];
	hBtn.hConfigButton.hConfig.GetName(sName, sizeof(sName));
	CReplyToCommand(iClient, "%t", "EW_Override_SetMode_Done", sName, iType, iMode, flCooldown, iMaxUses);
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Set item name by HammerID
// ----------------------------------------------------------------------------------------------------
public Action Command_SetName(int iClient, int iArgs)
{
	if (iArgs < 2)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_SetName_Usage");
		return Plugin_Handled;
	}

	char sArg1[16], sArgs[256], sNewName[32];
	GetCmdArgString(sArgs, sizeof(sArgs));
	int iLen = BreakString(sArgs, sArg1, sizeof(sArg1));
	if (iLen == -1)
		iLen = 0;

	int iHammerID = StringToInt(sArg1);
	FormatEx(sNewName, sizeof(sNewName), "%s", sArgs[iLen]);
	TrimString(sNewName);
	StripQuotes(sNewName);

	ArrayList hItems = EW_GetItemsArray();
	if (hItems == null)
		return Plugin_Handled;

	for (int i = 0; i < hItems.Length; i++)
	{
		CItem hItem = hItems.Get(i);
		if (hItem == null)
			continue;

		CConfig hConfig = hItem.hConfig;
		if (hConfig == null || hConfig.iHammerID != iHammerID)
			continue;

		hConfig.SetName(sNewName);
		CReplyToCommand(iClient, "%t", "EW_Override_SetName_Done", iHammerID, sNewName);
		return Plugin_Handled;
	}

	CReplyToCommand(iClient, "%t", "EW_Override_NoItem", iHammerID);
	return Plugin_Handled;
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Set item short name by HammerID
// ----------------------------------------------------------------------------------------------------
public Action Command_SetShortName(int iClient, int iArgs)
{
	if (iArgs < 2)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_SetShortName_Usage");
		return Plugin_Handled;
	}

	char sArg1[16], sArgs[256], sNewShort[32];
	GetCmdArgString(sArgs, sizeof(sArgs));
	int iLen = BreakString(sArgs, sArg1, sizeof(sArg1));
	if (iLen == -1)
		iLen = 0;

	int iHammerID = StringToInt(sArg1);
	FormatEx(sNewShort, sizeof(sNewShort), "%s", sArgs[iLen]);
	TrimString(sNewShort);
	StripQuotes(sNewShort);

	ArrayList hItems = EW_GetItemsArray();
	if (hItems == null)
		return Plugin_Handled;

	for (int i = 0; i < hItems.Length; i++)
	{
		CItem hItem = hItems.Get(i);
		if (hItem == null)
			continue;

		CConfig hConfig = hItem.hConfig;
		if (hConfig == null || hConfig.iHammerID != iHammerID)
			continue;

		hConfig.SetShort(sNewShort);
		CReplyToCommand(iClient, "%t", "EW_Override_SetShortName_Done", iHammerID, sNewShort);
		return Plugin_Handled;
	}

	CReplyToCommand(iClient, "%t", "EW_Override_NoItem", iHammerID);
	return Plugin_Handled;
}

// ----------------------------------------------------------------------------------------------------
// Purpose: Lock or unlock a button by HammerID
// ----------------------------------------------------------------------------------------------------
stock void EW4_LockButton(int iClient, int iArgs)
{
	if (iArgs < 2)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_LockButton_Usage");
		return;
	}

	char sHammerID[16], sLock[4];
	GetCmdArg(1, sHammerID, sizeof(sHammerID));
	GetCmdArg(2, sLock,     sizeof(sLock));

	int  iHammerID = StringToInt(sHammerID);
	bool bLock     = StringToInt(sLock) == 1;

	CItemButton hBtn = EW4_GetButton(iHammerID);
	if (hBtn == null)
	{
		CReplyToCommand(iClient, "%t", "EW_Override_NoButton", iHammerID);
		return;
	}

	hBtn.flReadyTime = bLock ? (GetGameTime() + 99999.0) : 0.0;

	char sName[32];
	hBtn.hConfigButton.hConfig.GetName(sName, sizeof(sName));

	char sState[32];
	FormatEx(sState, sizeof(sState), "%t", bLock ? "EW_Override_Locked" : "EW_Override_Unlocked");
	CReplyToCommand(iClient, "%t", "EW_Override_LockButton_Done", sName, sState);
}

//----------------------------------------------------------------------------------------------------
// Helper : find a CItemButton by item HammerID
//----------------------------------------------------------------------------------------------------
static CItemButton EW4_GetButton(int iButtonHammerID)
{
	ArrayList hItems = EW_GetItemsArray();
	if (hItems == null)
		return null;

	for (int i = 0; i < hItems.Length; i++)
	{
		CItem hItem = hItems.Get(i);
		if (hItem == null)
			continue;

		ArrayList hButtons = hItem.hButtons;
		if (hButtons == null)
			continue;

		for (int j = 0; j < hButtons.Length; j++)
		{
			CItemButton hBtn = hButtons.Get(j);
			if (hBtn == null)
				continue;

			if (hBtn.hConfigButton.iHammerID == iButtonHammerID)
				return hBtn;
		}
	}
	return null;
}
