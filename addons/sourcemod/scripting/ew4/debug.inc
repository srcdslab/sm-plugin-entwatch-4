//====================================================================================================
//
// Name: [entWatch] Debug
// Author: zaCade, Prometheum, koen
// Description: Handle the debug functions of [entWatch]
//
//====================================================================================================
// Recommended Sourcemod Version: 1.12.0.7225 or above
//====================================================================================================

/* INTEGERS */
int g_iBeamSpriteDebug = -1;
int g_iHaloSpriteDebug = -1;

//----------------------------------------------------------------------------------------------------
// Purpose: Register the sm_edebug_* admin commands
//----------------------------------------------------------------------------------------------------
public void Ew4_Debug_OnPluginStart()
{
	RegAdminCmd("sm_edebug_printitems",   Command_PrintItems,   ADMFLAG_CONFIG);
	RegAdminCmd("sm_edebug_printconfigs", Command_PrintConfigs, ADMFLAG_CONFIG);

	RegAdminCmd("sm_edebug_loadconfig",   Command_LoadConfig,   ADMFLAG_CONFIG);
	RegAdminCmd("sm_edebug_traceitems",   Command_TraceItems,   ADMFLAG_CONFIG);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Precache the debug trace sprites for the new map
//----------------------------------------------------------------------------------------------------
public void Ew4_Debug_OnMapStart()
{
	Ew4_Debug_LoadGameConfig();
}

//----------------------------------------------------------------------------------------------------
// Purpose: Precache the beam/halo sprites from funcommands gamedata
//----------------------------------------------------------------------------------------------------
void Ew4_Debug_LoadGameConfig()
{
	GameData hGameConf;
	if ((hGameConf = new GameData("funcommands.games")) == null)
	{
		SetFailState("Couldn't load \"funcommands.games\" game config!");
		return;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	if (hGameConf.GetKeyValue("SpriteBeam", sBuffer, sizeof(sBuffer)) && sBuffer[0])
		g_iBeamSpriteDebug = PrecacheModel(sBuffer);

	if (hGameConf.GetKeyValue("SpriteHalo", sBuffer, sizeof(sBuffer)) && sBuffer[0])
		g_iHaloSpriteDebug = PrecacheModel(sBuffer);

	delete hGameConf;
}

//----------------------------------------------------------------------------------------------------
// Purpose: sm_edebug_printitems - dump the live items array to the client console
//----------------------------------------------------------------------------------------------------
public Action Command_PrintItems(int iClient, int iArgs)
{
	int iArgConfigID = -1;

	if (iArgs >= 1)
	{
		char sArgConfigID[4];
		GetCmdArg(1, sArgConfigID, sizeof(sArgConfigID));

		iArgConfigID = StringToInt(sArgConfigID);
	}

	ArrayList hItems = EW_GetItemsArray();

	for (int iItemID; iItemID < hItems.Length; iItemID++)
	{
		CItem hItem = hItems.Get(iItemID);

		if (iArgConfigID != -1 && hItem.hConfig.iConfigID != iArgConfigID)
			continue;

		char sClientName[MAX_NAME_LENGTH];
		if (IsValidClient(hItem.iClient))
			GetClientName(hItem.iClient, sClientName, sizeof(sClientName));

		char sWeaponClass[32];
		if (IsValidEntity(hItem.iWeapon))
			GetEntityClassname(hItem.iWeapon, sWeaponClass, sizeof(sWeaponClass));

		PrintToConsole(iClient, "Item [%d|%d]\n\tiClient: %d <%s>\n\tiWeapon: %d <%s>\n\tiState: %d\n\tflReadyTime: %f",
			hItem.hConfig.iConfigID,
			hItem.iCreateID,
			hItem.iClient,
			sClientName,
			hItem.iWeapon,
			sWeaponClass,
			hItem.iState,
			hItem.flReadyTime
		);

		for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
		{
			CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

			char sButtonClass[32];
			if (IsValidEntity(hItemButton.iButton))
				GetEntityClassname(hItemButton.iButton, sButtonClass, sizeof(sButtonClass));

			PrintToConsole(iClient, "\tButton [%d]\n\t\tiButton: %d <%s>\n\t\tiState: %d\n\t\tiCurrentUses: %d\n\t\tflWaitTime: %f\n\t\tflReadyTime: %f",
				hItemButton.hConfigButton.iConfigID,
				hItemButton.iButton,
				sButtonClass,
				hItemButton.iState,
				hItemButton.iCurrentUses,
				hItemButton.flWaitTime,
				hItemButton.flReadyTime
			);
		}

		for (int iItemTriggerID; iItemTriggerID < hItem.hTriggers.Length; iItemTriggerID++)
		{
			CItemTrigger hItemTrigger = hItem.hTriggers.Get(iItemTriggerID);

			char sTriggerClass[32];
			if (IsValidEntity(hItemTrigger.iTrigger))
				GetEntityClassname(hItemTrigger.iTrigger, sTriggerClass, sizeof(sTriggerClass));

			PrintToConsole(iClient, "\tTrigger [%d]\n\t\tiTrigger: %d <%s>\n\t\tiState: %d",
				hItemTrigger.hConfigTrigger.iConfigID,
				hItemTrigger.iTrigger,
				sTriggerClass,
				hItemTrigger.iState
			);
		}
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: sm_edebug_printconfigs - dump the loaded configs array to the client console
//----------------------------------------------------------------------------------------------------
public Action Command_PrintConfigs(int iClient, int iArgs)
{
	int iArgConfigID = -1;

	if (iArgs >= 1)
	{
		char sArgConfigID[4];
		GetCmdArg(1, sArgConfigID, sizeof(sArgConfigID));

		iArgConfigID = StringToInt(sArgConfigID);
	}

	ArrayList hConfigs = EW_GetConfigsArray();

	for (int iConfigID; iConfigID < hConfigs.Length; iConfigID++)
	{
		CConfig hConfig = hConfigs.Get(iConfigID);

		if (iArgConfigID != -1 && hConfig.iConfigID != iArgConfigID)
			continue;

		char sName[EW_MAX_NAME_LENGTH];
		hConfig.GetName(sName, sizeof(sName));

		char sShort[EW_MAX_SHORTNAME_LENGTH];
		hConfig.GetShort(sShort, sizeof(sShort));

		char sColor[EW_MAX_COLOR_LENGTH];
		hConfig.GetColor(sColor, sizeof(sColor));

		PrintToConsole(iClient, "Config\n\tsName: %s\n\tsShort: %s\n\tsColor: %s\n\tiConfigID: %d\n\tiHammerID: %d\n\tbShowMessages: %d\n\tbShowInterface: %d",
			sName,
			sShort,
			sColor,
			hConfig.iConfigID,
			hConfig.iHammerID,
			view_as<int>(hConfig.bShowMessages),
			view_as<int>(hConfig.bShowInterface)
		);

		for (int iConfigButtonID; iConfigButtonID < hConfig.hButtons.Length; iConfigButtonID++)
		{
			CConfigButton hConfigButton = hConfig.hButtons.Get(iConfigButtonID);

			char sOutput[32];
			hConfigButton.GetOutput(sOutput, sizeof(sOutput));

			PrintToConsole(iClient, "\tButton\n\t\tsOutput: %s\n\t\tiConfigID: %d\n\t\tiHammerID: %d\n\t\tiType: %d\n\t\tiMode: %d\n\t\tiMaxUses: %d\n\t\tflButtonCooldown: %f\n\t\tflItemCooldown: %f\n\t\tbShowActivate: %d\n\t\tbShowCooldown: %d",
				sOutput,
				hConfigButton.iConfigID,
				hConfigButton.iHammerID,
				hConfigButton.iType,
				hConfigButton.iMode,
				hConfigButton.iMaxUses,
				hConfigButton.flButtonCooldown,
				hConfigButton.flItemCooldown,
				view_as<int>(hConfigButton.bShowActivate),
				view_as<int>(hConfigButton.bShowCooldown)
			);
		}

		for (int iConfigTriggerID; iConfigTriggerID < hConfig.hTriggers.Length; iConfigTriggerID++)
		{
			CConfigTrigger hConfigTrigger = hConfig.hTriggers.Get(iConfigTriggerID);

			PrintToConsole(iClient, "\tTrigger\n\t\tiConfigID: %d\n\t\tiHammerID: %d\n\t\tiType: %d",
				hConfigTrigger.iConfigID,
				hConfigTrigger.iHammerID,
				hConfigTrigger.iType
			);
		}
	}

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: sm_edebug_loadconfig - reload the current map's item config
//----------------------------------------------------------------------------------------------------
public Action Command_LoadConfig(int iClient, int iArgs)
{
	bool bSuccessful = EW_LoadConfig(true);

	CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Debug_LoadConfig_Result", bSuccessful ? "Successful" : "Failed");
	LogAction(iClient, -1, "\"%L\" loaded map config.", iClient);

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: sm_edebug_traceitems - draw a beam from the admin to every spawned/dropped item
//----------------------------------------------------------------------------------------------------
public Action Command_TraceItems(int iClient, int iArgs)
{
	if (!IsValidClient(iClient))
		return Plugin_Handled;

	ArrayList hItems = EW_GetItemsArray();

	for (int iItemID; iItemID < hItems.Length; iItemID++)
	{
		CItem hItem = hItems.Get(iItemID);

		if (!IsValidEntity(hItem.iWeapon))
			continue;

		if (hItem.iState == EW_ENTITY_STATE_SPAWNED || hItem.iState == EW_ENTITY_STATE_DROPPED)
		{
			float flOriginStart[3];
			GetClientAbsOrigin(iClient, flOriginStart);

			float flOriginEnd[3];
			GetEntPropVector(hItem.iWeapon, Prop_Data, "m_vecOrigin", flOriginEnd);

			flOriginStart[2] += 2;
			flOriginEnd[2] += 2;

			char sColor[EW_MAX_COLOR_LENGTH];
			hItem.hConfig.GetColor(sColor, sizeof(sColor));

			int iColorDecimal;
			StringToIntEx(sColor, iColorDecimal, 16);

			int iColor[4];
			iColor[0] = iColorDecimal >> 16 & 0xFF;
			iColor[1] = iColorDecimal >> 8  & 0xFF;
			iColor[2] = iColorDecimal >> 0  & 0xFF;
			iColor[3] = 0xFF;

			TE_SetupBeamPoints(flOriginStart, flOriginEnd, g_iBeamSpriteDebug, g_iHaloSpriteDebug, 0, 10, 5.0, 2.0, 2.0, 0, 0.5, iColor, 10);
			TE_SendToClient(iClient);
		}
	}

	CPrintToChat(iClient, "{#%s}%t {#%s}%t", g_clr.sTag, "EW_Tag", g_clr.sWarning, "EW_Debug_TraceItems_Done");
	LogAction(iClient, -1, "\"%L\" traced items.", iClient);

	return Plugin_Handled;
}
