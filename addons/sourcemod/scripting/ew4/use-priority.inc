//====================================================================================================
//
// Name: [entWatch] Use Priority
// Author: .Rushaway
// Description: Pressing +use fires the player's item button, even while in the crowd.
//
//====================================================================================================

/* BOOLEANS */
bool g_bUsePriority = true;

/* INTEGERS */
int g_iUsePriority_LastButtons[MAXPLAYERS + 1];

/* FLOATS */
float g_flRange = 50.0;

/* CONVARS */
ConVar g_hCVar_UsePriority;
ConVar g_hCVar_UsePriorityRange;

//----------------------------------------------------------------------------------------------------
// Purpose: Create the use-priority cvars and cache their values
//----------------------------------------------------------------------------------------------------
void EW4_UsePriority_OnPluginStart()
{
	g_hCVar_UsePriority      = CreateConVar("entwatch_use_priority", "1", "Enable/Disable forced pressing of the button.", _, true, 0.0, true, 1.0);
	g_hCVar_UsePriorityRange = CreateConVar("entwatch_use_priority_range", "50.0", "Range (units) of the hull trace used to pick a button on items that have more than one use button.", _, true, 0.0, true, 512.0);

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
// Purpose: Forget the previous button state so the next player in this slot starts clean
//----------------------------------------------------------------------------------------------------
void EW4_UsePriority_OnClientDisconnect(int iClient)
{
	g_iUsePriority_LastButtons[iClient] = 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: True if this item button can be forced through +use
//----------------------------------------------------------------------------------------------------
bool UsePriority_IsUseButton(CItemButton hItemButton)
{
	if (hItemButton.hConfigButton.iType != EW_BUTTON_TYPE_USE)
		return false;

	if (hItemButton.hConfigButton.iMode == EW_BUTTON_MODE_NONE || hItemButton.hConfigButton.iMode == EW_BUTTON_MODE_COUNTERVALUE)
		return false;

	return hItemButton.iButton != INVALID_ENT_REFERENCE && IsValidEntity(hItemButton.iButton);
}

//----------------------------------------------------------------------------------------------------
// Purpose: True if a hull swept from the player's eyes along their view hits the button
//----------------------------------------------------------------------------------------------------
bool UsePriority_IsAimingAt(int iClient, const float angles[3], int iButton)
{
	float vEye[3], vForward[3], vEnd[3];
	GetClientEyePosition(iClient, vEye);
	GetAngleVectors(angles, vForward, NULL_VECTOR, NULL_VECTOR);

	vEnd[0] = vEye[0] + vForward[0] * g_flRange;
	vEnd[1] = vEye[1] + vForward[1] * g_flRange;
	vEnd[2] = vEye[2] + vForward[2] * g_flRange;

	float vMins[3] = {-16.0, -16.0, -16.0};
	float vMaxs[3] = { 16.0,  16.0,  16.0};

	TR_ClipRayHullToEntity(vEye, vEnd, vMins, vMaxs, MASK_ALL, iButton);
	return TR_DidHit();
}

//----------------------------------------------------------------------------------------------------
// Purpose: On the tick +use is pressed, fire Use on the player's item button. An item with a single
//          use button fires wherever the player looks; an item with several fires the one aimed at.
//          Cooldowns, max uses and m_flWait are enforced by OnButtonPress.
//----------------------------------------------------------------------------------------------------
public Action EW4_UsePriority_OnPlayerRunCmd(int iClient, int& iButtons, float angles[3])
{
	bool bPressed = (iButtons & IN_USE) && !(g_iUsePriority_LastButtons[iClient] & IN_USE);
	g_iUsePriority_LastButtons[iClient] = iButtons;

	if (!g_bUsePriority || !bPressed || !g_hArray_Items.Length)
		return Plugin_Continue;

	for (int iItemID = 0; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (hItem.iClient == INVALID_ENT_REFERENCE || hItem.iClient != iClient)
			continue;

		int iUseButtons = 0;
		int iSingleButton = INVALID_ENT_REFERENCE;
		int iButtonCount = hItem.hButtons.Length;

		for (int iItemButtonID = 0; iItemButtonID < iButtonCount; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (!UsePriority_IsUseButton(hItemButton))
				continue;

			iUseButtons++;
			iSingleButton = hItemButton.iButton;
		}

		if (iUseButtons == 1)
		{
			AcceptEntityInput(iSingleButton, "Use", iClient, iClient);
			continue;
		}

		for (int iItemButtonID = 0; iUseButtons > 1 && iItemButtonID < iButtonCount; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			if (!UsePriority_IsUseButton(hItemButton) || !UsePriority_IsAimingAt(iClient, angles, hItemButton.iButton))
				continue;

			AcceptEntityInput(hItemButton.iButton, "Use", iClient, iClient);
			break;
		}
	}

	return Plugin_Continue;
}
