//====================================================================================================
//
// Name: [entWatch] Use Priority
// Author: .Rushaway
// Description: Player cannot press the button while in the crowd. Forcefully press the button
//
//====================================================================================================

/* BOOLEANS */
bool g_bUsePriority = true;

/* INTEGERS */
int g_iUsePriority_Client        = -1;
int g_iUsePriority_FoundButton   = INVALID_ENT_REFERENCE;
int g_iUsePriority_FoundItemIdx  = -1;
int g_iUsePriority_FoundBtnIdx   = -1;

/* FLOATS */
float g_flRange = 50.0;

/* CONVARS */
ConVar g_hCVar_UsePriority;
ConVar g_hCVar_UsePriorityRange;

//----------------------------------------------------------------------------------------------------
// Purpose: Create the use-priority cvars and cache their values
//----------------------------------------------------------------------------------------------------
stock void EW4_UsePriority_OnPluginStart()
{
	g_hCVar_UsePriority      = CreateConVar("entwatch_use_priority", "1", "Enable/Disable forced pressing of the button.", _, true, 0.0, true, 1.0);
	g_hCVar_UsePriorityRange = CreateConVar("entwatch_use_priority_range", "50.0", "Range (units) of the hull trace used to detect item buttons in front of the player.", _, true, 0.0, true, 512.0);

	g_hCVar_UsePriority.AddChangeHook(Cvar_UsePriority_Changed);
	g_hCVar_UsePriorityRange.AddChangeHook(Cvar_UsePriority_Changed);

	g_bUsePriority = g_hCVar_UsePriority.BoolValue;
	g_flRange = g_hCVar_UsePriorityRange.FloatValue;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Keep the cached use-priority toggle and range in sync with their cvars
//----------------------------------------------------------------------------------------------------
public void Cvar_UsePriority_Changed(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (convar == g_hCVar_UsePriority)
		g_bUsePriority = convar.BoolValue;
	else if (convar == g_hCVar_UsePriorityRange)
		g_flRange = convar.FloatValue;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Hull enumeration callback.
//----------------------------------------------------------------------------------------------------
public bool UsePriority_HullCallback(int iEntity, int iContentsMask)
{
	if (iEntity >= 0 && iEntity <= MaxClients)
		return true;

	for (int iItemID = 0; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient == INVALID_ENT_REFERENCE || hItem.iClient != g_iUsePriority_Client)
			continue;

		int iButtonCount = hItem.hButtons.Length;
		for (int iItemButtonID = 0; iItemButtonID < iButtonCount; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (hItemButton.iButton != iEntity)
				continue;

			if (hItemButton.hConfigButton.iType != EW_BUTTON_TYPE_USE)
				continue;

			if (hItemButton.hConfigButton.iMode == EW_BUTTON_MODE_NONE || hItemButton.hConfigButton.iMode == EW_BUTTON_MODE_COUNTERVALUE)
				continue;

			if (!IsValidEntity(iEntity))
				continue;

			g_iUsePriority_FoundButton  = iEntity;
			g_iUsePriority_FoundItemIdx = iItemID;
			g_iUsePriority_FoundBtnIdx  = iItemButtonID;
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Intercepts IN_USE button presses and forces use on tracked item buttons.
//----------------------------------------------------------------------------------------------------
public Action EW4_UsePriority_OnPlayerRunCmd(int iClient, int& iButtons, float angles[3])
{
	if (!g_bUsePriority || !(iButtons & IN_USE))
		return Plugin_Continue;

	if (!EW_ClientHasItem(iClient))
		return Plugin_Continue;

	float vEye[3], vForward[3], vEnd[3];
	GetClientEyePosition(iClient, vEye);
	GetAngleVectors(angles, vForward, NULL_VECTOR, NULL_VECTOR);

	vEnd[0] = vEye[0] + vForward[0] * g_flRange;
	vEnd[1] = vEye[1] + vForward[1] * g_flRange;
	vEnd[2] = vEye[2] + vForward[2] * g_flRange;

	float vMins[3] = {-16.0, -16.0, -16.0};
	float vMaxs[3] = { 16.0,  16.0,  16.0};

	g_iUsePriority_Client        = iClient;
	g_iUsePriority_FoundButton   = INVALID_ENT_REFERENCE;
	g_iUsePriority_FoundItemIdx  = -1;
	g_iUsePriority_FoundBtnIdx   = -1;

	TR_EnumerateEntitiesHull(vEye, vEnd, vMins, vMaxs, PARTITION_NON_STATIC_EDICTS, UsePriority_HullCallback);

	if (g_iUsePriority_FoundButton == INVALID_ENT_REFERENCE || g_iUsePriority_FoundItemIdx == -1 || g_iUsePriority_FoundBtnIdx == -1)
		return Plugin_Continue;

	CItem       hFoundItem       = g_hArray_Items.Get(g_iUsePriority_FoundItemIdx);
	CItemButton hFoundItemButton = hFoundItem.hButtons.Get(g_iUsePriority_FoundBtnIdx);

	float flNow = GetGameTime();

	if (hFoundItem.flReadyTime > flNow)
		return Plugin_Continue;

	if (hFoundItemButton.flReadyTime > flNow)
		return Plugin_Continue;

	if (hFoundItemButton.hConfigButton.iMode == EW_BUTTON_MODE_MAXUSES && hFoundItemButton.iCurrentUses >= hFoundItemButton.hConfigButton.iMaxUses)
		return Plugin_Continue;

	if (HasEntProp(hFoundItemButton.iButton, Prop_Data, "m_flWait") && hFoundItemButton.flWaitTime >= flNow)
		return Plugin_Continue;

	AcceptEntityInput(hFoundItemButton.iButton, "Use", iClient, iClient);
	return Plugin_Continue;
}
