//====================================================================================================
//
// Name: [entWatch] Use Priority
// Author: .Rushaway, koen
// Description: Pressing +use fires the player's item button
//
//====================================================================================================

/* BOOLEANS */
bool g_bUsePriority = true;

/* INTEGERS */
int g_iUsePriority_LastButtons[MAXPLAYERS + 1];

/* CONVARS */
ConVar g_hCVar_UsePriority;

void EW4_UsePriority_OnPluginStart()
{
	g_hCVar_UsePriority      = CreateConVar("entwatch_use_priority", "1", "Enable/Disable forced pressing of the button.", _, true, 0.0, true, 1.0);
	g_hCVar_UsePriority.AddChangeHook(Cvar_UsePriority_Changed);
	g_bUsePriority = g_hCVar_UsePriority.BoolValue;
}

public void Cvar_UsePriority_Changed(ConVar convar, const char[] oldValue, const char[] newValue)
{
	g_bUsePriority = convar.BoolValue;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Reset player slot button state on disconnect
//----------------------------------------------------------------------------------------------------
void EW4_UsePriority_OnClientDisconnect(int iClient)
{
	g_iUsePriority_LastButtons[iClient] = 0;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Check if item button can be pressed
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
// Purpose: Returns the item's use button whose centre is closest to the player's view direction.
//          The buttons are parented to the carried item, so no range check is needed. Works from
//          entity origins and bounds only, so non-solid parented buttons count.
//----------------------------------------------------------------------------------------------------
int UsePriority_FindAimedButton(int iClient, const float angles[3], CItem hItem)
{
	float vEye[3], vForward[3];
	GetClientEyePosition(iClient, vEye);
	GetAngleVectors(angles, vForward, NULL_VECTOR, NULL_VECTOR);

	int iBestButton = INVALID_ENT_REFERENCE;
	float flBestDot = -2.0;

	int iButtonCount = hItem.hButtons.Length;
	for (int iItemButtonID = 0; iItemButtonID < iButtonCount; iItemButtonID++)
	{
		CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

		if (!UsePriority_IsUseButton(hItemButton))
			continue;

		int iButton = hItemButton.iButton;

		float vOrigin[3], vMins[3], vMaxs[3];
		GetEntPropVector(iButton, Prop_Data, "m_vecAbsOrigin", vOrigin);
		GetEntPropVector(iButton, Prop_Send, "m_vecMins", vMins);
		GetEntPropVector(iButton, Prop_Send, "m_vecMaxs", vMaxs);

		float vCenter[3], vDir[3];
		for (int i = 0; i < 3; i++)
			vCenter[i] = vOrigin[i] + (vMins[i] + vMaxs[i]) * 0.5;

		MakeVectorFromPoints(vEye, vCenter, vDir);
		NormalizeVector(vDir, vDir);
		float flDot = GetVectorDotProduct(vForward, vDir);

		if (flDot <= flBestDot)
			continue;

		iBestButton = iButton;
		flBestDot = flDot;
	}

	return iBestButton;
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

		if (iUseButtons > 1)
		{
			int iAimedButton = UsePriority_FindAimedButton(iClient, angles, hItem);
			if (iAimedButton != INVALID_ENT_REFERENCE)
				AcceptEntityInput(iAimedButton, "Use", iClient, iClient);
		}
	}

	return Plugin_Continue;
}
