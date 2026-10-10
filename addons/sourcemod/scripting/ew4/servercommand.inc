//====================================================================================================
//
// Name: [entWatch] Server Command
// Author: zaCade, Prometheum, & koen
// Description: Handle the filtering of servercommand messages for [entWatch]
//
//====================================================================================================
// Recommended Sourcemod Version: 1.12.0.7225 or above
//====================================================================================================

#include <dhooks>

/* DYNAMICHOOKS */
DynamicHook g_hAcceptInput;

/* CONVARS */
ConVar g_hCVar_BlockServerCommand;

/* BOOLEANS */
bool g_bBlockServerCommand;

public void Ew4_ServerCommand_OnPluginStart()
{
	g_hCVar_BlockServerCommand = CreateConVar("entwatch_block_servercommand", "1", "Block \'Command\' outputs on point_servercommand.", _, true, 0.0, true, 1.0);
	g_hCVar_BlockServerCommand.AddChangeHook(Cvar_BlockServerCommand_Changed);
	g_bBlockServerCommand = g_hCVar_BlockServerCommand.BoolValue;

	Ew4_ServerCommand_LoadGameConfig();

	int iEntity = INVALID_ENT_REFERENCE;
	while ((iEntity = FindEntityByClassname(iEntity, "point_servercommand")) != INVALID_ENT_REFERENCE)
	{
		Ew4_ServerCommand_OnEntityCreated(iEntity, "point_servercommand");
	}
}

public void Cvar_BlockServerCommand_Changed(ConVar convar, const char[] oldValue, const char[] newValue)
{
	g_bBlockServerCommand = convar.BoolValue;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Set up the AcceptInput DynamicHook from sdktools gamedata
//----------------------------------------------------------------------------------------------------
void Ew4_ServerCommand_LoadGameConfig()
{
	GameData hGameConf;
	if ((hGameConf = new GameData("sdktools.games")) == null)
	{
		SetFailState("Couldn't load \"sdktools.games\" game config!");
		return;
	}

	// bool CBaseEntity::AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID )
	int iAcceptInputOffset;
	if ((iAcceptInputOffset = hGameConf.GetOffset("AcceptInput")) == -1)
	{
		delete hGameConf;
		SetFailState("hGameConf.GetOffset(\"AcceptInput\") failed!");
		return;
	}

	if ((g_hAcceptInput = new DynamicHook(iAcceptInputOffset, HookType_Entity, ReturnType_Bool, ThisPointer_CBaseEntity)) == null)
	{
		delete hGameConf;
		SetFailState("new DynamicHook(iAcceptInputOffset (%d), HookType_Entity, ReturnType_Bool, ThisPointer_CBaseEntity) failed!", iAcceptInputOffset);
		return;
	}

	g_hAcceptInput.AddParam(HookParamType_CharPtr);
	g_hAcceptInput.AddParam(HookParamType_CBaseEntity);
	g_hAcceptInput.AddParam(HookParamType_CBaseEntity);
	g_hAcceptInput.AddParam(HookParamType_Object, 20, DHookPass_ByVal|DHookPass_ODTOR|DHookPass_OCTOR|DHookPass_OASSIGNOP); //variant_t is a union of 12 (float[3]) plus two int type params 12 + 8 = 20
	g_hAcceptInput.AddParam(HookParamType_Int);

	delete hGameConf;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Hook AcceptInput on each newly created point_servercommand entity
//----------------------------------------------------------------------------------------------------
public void Ew4_ServerCommand_OnEntityCreated(int iEntity, const char[] sClassname)
{
	if (!IsValidEntity(iEntity) || !StrEqual(sClassname, "point_servercommand", false))
		return;

	g_hAcceptInput.HookEntity(Hook_Pre, iEntity, Ew4_ServerCommand_OnAcceptInput);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Block the "Command" input when it originates from an entWatch item
//----------------------------------------------------------------------------------------------------
public MRESReturn Ew4_ServerCommand_OnAcceptInput(int iEntity, DHookReturn hReturn, DHookParam hParams)
{
	if (!g_bBlockServerCommand)
		return MRES_Ignored;

	char sInputName[128];
	hParams.GetString(1, sInputName, sizeof(sInputName));

	if (!StrEqual(sInputName, "Command", false))
		return MRES_Ignored;

	int iCaller = INVALID_ENT_REFERENCE;
	if (!hParams.IsNull(3))
		iCaller = hParams.Get(3);

	if (IsValidEntity(iCaller) && EW_IsEntityItem(iCaller))
	{
		hReturn.Value = false;
		return MRES_Supercede;
	}

	return MRES_Ignored;
}
