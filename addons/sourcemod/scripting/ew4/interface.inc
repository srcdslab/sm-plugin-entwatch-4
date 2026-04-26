//====================================================================================================
//
// Name: [entWatch] Interface
// Author: zaCade, Prometheum, koen, .Rushaway
// Description: HUD display, client preferences and admin menus for [entWatch]
//
//====================================================================================================
// Requires Sourcemod Version: 1.10.0.6531 or above
//====================================================================================================
#include <adminmenu>
#include <clientprefs_stocks>

/* BOOLEANS */
bool g_bIsAdmin[MAXPLAYERS+1];
bool g_bInterfaceHidden[MAXPLAYERS+1];

/* INTEGERS */
int g_iInterfaceMode = 0;

/* CONVARS */
ConVar g_hCVar_InterfaceMode;

/* COOKIES */
Cookie_Stocks g_hCookie_InterfaceHidden;

/* HANDLES */ 
TopMenu g_hAdminMenu;

// Sub-module - all admin panel menus
#include "menu-interface.inc"

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnPluginStart()
{
	g_hCVar_InterfaceMode = CreateConVar("sm_einterface_mode", "3", "Entwatch interface display mode (1 = All items, 2 = Admin all items / Team items only, 3 = Team items only)", FCVAR_NONE, true, 1.0, true, 3.0);
	g_hCVar_InterfaceMode.AddChangeHook(OnInterfaceCvarChanged);

	// Cache initial value
	g_iInterfaceMode = g_hCVar_InterfaceMode.IntValue;

	g_hCookie_InterfaceHidden = new Cookie_Stocks("EW_InterfaceHidden", "", CookieAccess_Private);

	RegConsoleCmd("sm_hud", Command_ToggleHUD);

	// Register in admin top-menu if already available
	TopMenu topmenu;
	if (LibraryExists("adminmenu") && ((topmenu = GetAdminTopMenu()) != null))
		OnAdminMenuReady(topmenu);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Cvar change hook - keep cached integer in sync
//----------------------------------------------------------------------------------------------------
void OnInterfaceCvarChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	g_iInterfaceMode = g_hCVar_InterfaceMode.IntValue;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void OnAdminMenuReady(Handle aTopMenu)
{
	TopMenu topmenu = TopMenu.FromHandle(aTopMenu);

	if (g_hAdminMenu == topmenu)
		return;

	g_hAdminMenu = topmenu;

	// Delegate registration to the menus sub-module
	EWM_RegisterAdminMenu(g_hAdminMenu);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnLibraryRemoved(const char[] name)
{
	if (strcmp(name, "adminmenu", false) == 0)
		g_hAdminMenu = null;
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnMapStart()
{
	CreateTimer(1.0, OnDisplayHUD, INVALID_HANDLE, TIMER_FLAG_NO_MAPCHANGE|TIMER_REPEAT);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnClientCookiesCached(int iClient)
{
	g_bInterfaceHidden[iClient] = g_hCookie_InterfaceHidden.GetBool(iClient);
}

//----------------------------------------------------------------------------------------------------
// Purpose: Cache admin status per client (reset on disconnect)
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnClientPostAdminCheck(int iClient)
{
	g_bIsAdmin[iClient] = CheckCommandAccess(iClient, "", ADMFLAG_BAN);
}

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Interface_OnClientDisconnect(int iClient)
{
	g_bIsAdmin[iClient]         = false;
	g_bInterfaceHidden[iClient] = false;
}

//----------------------------------------------------------------------------------------------------
// Purpose: sm_hud - toggle HUD visibility, persisted via cookie
//----------------------------------------------------------------------------------------------------
public Action Command_ToggleHUD(int iClient, int iArgs)
{
	g_bInterfaceHidden[iClient] = !g_bInterfaceHidden[iClient];
	g_hCookie_InterfaceHidden.SetBool(iClient, g_bInterfaceHidden[iClient]);

	char sMessage[256];
	FormatEx(sMessage, sizeof(sMessage), "%t", g_bInterfaceHidden[iClient] ? "EW_Chat_HUD_Hidden" : "EW_Chat_HUD_Visible");
	CPrintToChat(iClient, "{#%s}%t {#%s}%s", g_clr.sTag, "EW_Tag", g_clr.sWarning, sMessage);

	return Plugin_Handled;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Format the cooldown string for a single button.
//          Returns true if a string was written, false if the button has no cooldown to show.
//----------------------------------------------------------------------------------------------------
stock bool FormatButtonCooldown(char[] sBuffer, const int iMaxLength, CItemButton hItemButton, float flNow)
{
	switch (hItemButton.hConfigButton.iMode)
	{
		case EW_BUTTON_MODE_COOLDOWN:
		{
			if (hItemButton.flReadyTime > flNow)
				Format(sBuffer, iMaxLength, "%d", RoundToNearest(hItemButton.flReadyTime - flNow));
			else
				Format(sBuffer, iMaxLength, "R");

			return true;
		}
		case EW_BUTTON_MODE_MAXUSES:
		{
			if (hItemButton.iCurrentUses >= hItemButton.hConfigButton.iMaxUses)
				Format(sBuffer, iMaxLength, "E");
			else if (hItemButton.flReadyTime > flNow)
				Format(sBuffer, iMaxLength, "%d", RoundToNearest(hItemButton.flReadyTime - flNow));
			else
				Format(sBuffer, iMaxLength, "%d/%d", hItemButton.iCurrentUses, hItemButton.hConfigButton.iMaxUses);

			return true;
		}
		case EW_BUTTON_MODE_COOLDOWN_CHARGES:
		{
			if (hItemButton.flReadyTime > flNow)
				Format(sBuffer, iMaxLength, "%d", RoundToNearest(hItemButton.flReadyTime - flNow));
			else
				Format(sBuffer, iMaxLength, "%d/%d", hItemButton.iCurrentUses, hItemButton.hConfigButton.iMaxUses);

			return true;
		}
		case EW_BUTTON_MODE_COUNTERVALUE:
		{
			if (hItemButton.iCurrentUses <= 0)
				Format(sBuffer, iMaxLength, "E");
			else
				Format(sBuffer, iMaxLength, "%d/%d", hItemButton.iCurrentUses, hItemButton.hConfigButton.iMaxUses);

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Concatenate cooldown strings for all visible buttons of an item.
//          Returns true if at least one button produced output.
//----------------------------------------------------------------------------------------------------
stock bool FormatItemCooldowns(char[] sBuffer, const int iMaxLength, CItem hItem, float flNow)
{
	bool bHasCooldown = false;

	for (int iItemButtonID; iItemButtonID < hItem.hButtons.Length; iItemButtonID++)
	{
		CItemButton hItemButton = hItem.hButtons.Get(iItemButtonID);

		if (!hItemButton.hConfigButton.bShowCooldown)
			continue;

		char sCooldown[8];
		if (!FormatButtonCooldown(sCooldown, sizeof(sCooldown), hItemButton, flNow))
			continue;

		if (!bHasCooldown)
		{
			bHasCooldown = true;
			StrCat(sBuffer, iMaxLength, sCooldown);
		}
		else
		{
			StrCat(sBuffer, iMaxLength, "|");
			StrCat(sBuffer, iMaxLength, sCooldown);
		}
	}

	return bHasCooldown;
}

//----------------------------------------------------------------------------------------------------
// Purpose: Repeating timer - build paged HUD buffers and send KeyHintText to each client.
//
// Pages layout:
//   [0] = All items   (used for mode 1, or mode 2 admins)
//   [1] = T  items    (team 2)
//   [2] = CT items    (team 3)
//----------------------------------------------------------------------------------------------------
stock Action OnDisplayHUD(Handle hTimer)
{
	if (!g_hArray_Items || !g_hArray_Items.Length)
		return Plugin_Continue;

	// iHUDPages[pageID]          = current page index being filled
	// iHUDLens[pageID][page]     = byte count of that page's buffer
	// sHUDPages[pageID][page]    = text buffer
	int  iHUDPages[3];
	int  iHUDLens[3][8];
	char sHUDPages[3][8][255];

	float flNow = GetGameTime();

	for (int iItemID; iItemID < g_hArray_Items.Length; iItemID++)
	{
		CItem hItem = g_hArray_Items.Get(iItemID);

		if (!hItem.hConfig.bShowInterface || hItem.iClient == INVALID_ENT_REFERENCE)
			continue;

		char sShort[16];
		hItem.hConfig.GetShort(sShort, sizeof(sShort));

		char sCooldowns[16];
		bool bCooldowns = FormatItemCooldowns(sCooldowns, sizeof(sCooldowns), hItem, flNow);

		char sLine[64];
		if (bCooldowns)
			Format(sLine, sizeof(sLine), "%s [%s]: %N", sShort, sCooldowns, hItem.iClient);
		else
			Format(sLine, sizeof(sLine), "%s [N/A]: %N", sShort, hItem.iClient);

		int iLineLen = strlen(sLine) + 1; // +1 for trailing newline

		int iTeam = GetClientTeam(hItem.iClient);

		// ---- Team pages (T = slot 1, CT = slot 2) ----
		if (iTeam == 2 || iTeam == 3)
		{
			int iPageSlot = iTeam - 1; // 2 → 1, 3 → 2

			if (iHUDLens[iPageSlot][iHUDPages[iPageSlot]] + iLineLen >= sizeof(sHUDPages[][]))
				iHUDPages[iPageSlot]++;

			int p = iHUDPages[iPageSlot];
			StrCat(sHUDPages[iPageSlot][p], sizeof(sHUDPages[][]), sLine);
			StrCat(sHUDPages[iPageSlot][p], sizeof(sHUDPages[][]), "\n");
			iHUDLens[iPageSlot][p] += iLineLen;
		}

		// ---- All-items page (slot 0) ----
		if (iHUDLens[0][iHUDPages[0]] + iLineLen >= sizeof(sHUDPages[][]))
			iHUDPages[0]++;

		int p0 = iHUDPages[0];
		StrCat(sHUDPages[0][p0], sizeof(sHUDPages[][]), sLine);
		StrCat(sHUDPages[0][p0], sizeof(sHUDPages[][]), "\n");
		iHUDLens[0][p0] += iLineLen;
	}

	// ---- Page rotation: advance every 5 ticks ----
	static int iPageInterval;
	static int iPageCurrent[3];

	if (iPageInterval >= 5)
	{
		for (int iPageID; iPageID < 3; iPageID++)
		{
			if (iPageCurrent[iPageID] >= iHUDPages[iPageID])
				iPageCurrent[iPageID] = 0;
			else
				iPageCurrent[iPageID]++;
		}
		iPageInterval = 0;
	}
	else
	{
		iPageInterval++;
	}

	// ---- Detect protobuf user message type once per tick ----
	bool bIsProto = (
		GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available
		&& GetUserMessageType() == UM_Protobuf
	);

	// ---- Send to each eligible client ----
	for (int iClient = 1; iClient <= MaxClients; iClient++)
	{
		if (!IsClientInGame(iClient))
			continue;

		if (IsFakeClient(iClient) && !IsClientSourceTV(iClient))
			continue;

		if (g_bInterfaceHidden[iClient])
			continue;

		int iPagePanel;

		switch (g_iInterfaceMode)
		{
			case 1:
			{
				// All clients see all items
				iPagePanel = 0;
			}
			case 2:
			{
				// Admins see all items, others see their team only
				if (g_bIsAdmin[iClient])
					iPagePanel = 0;
				else
				{
					int t = GetClientTeam(iClient);
					iPagePanel = (t == 2) ? 1 : (t == 3) ? 2 : 0;
				}
			}
			case 3:
			{
				// Everyone sees their team only
				int t = GetClientTeam(iClient);
				iPagePanel = (t == 2) ? 1 : (t == 3) ? 2 : 0;
			}
		}

		if (!sHUDPages[iPagePanel][iPageCurrent[iPagePanel]][0])
			continue;

		Handle hMessage = StartMessageOne("KeyHintText", iClient);

		if (bIsProto)
			PbAddString(hMessage, "hints", sHUDPages[iPagePanel][iPageCurrent[iPagePanel]]);
		else
		{
			BfWriteByte(hMessage, 1);
			BfWriteString(hMessage, sHUDPages[iPagePanel][iPageCurrent[iPagePanel]]);
		}

		EndMessage();
	}

	return Plugin_Continue;
}