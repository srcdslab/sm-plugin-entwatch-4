//====================================================================================================
//
// Name: [entWatch] Forcedrop
// Author: zaCade, Prometheum, koen
// Description: Handle the forced dropping of weapons of [entWatch]
//
//====================================================================================================
// Requires Sourcemod Version: ?
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose:
//----------------------------------------------------------------------------------------------------
public void Ew4_Forcedrop_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType)
{
	if ((iInteractionType != EW_WEAPON_INTERACTION_DEATH) && (iInteractionType != EW_WEAPON_INTERACTION_DISCONNECT))
		return;

	if (0 <= SDKCall(SDKCall_GetSlot, hItem.iWeapon) < 2)
		SDKHooks_DropWeapon(iClient, hItem.iWeapon, NULL_VECTOR, NULL_VECTOR, false); // Using this causes ammo to disapear on the weapon
}
