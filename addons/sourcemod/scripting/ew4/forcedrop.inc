//====================================================================================================
//
// Name: [entWatch] Forcedrop
// Author: zaCade, Prometheum, koen
// Description: Handle the forced dropping of weapons of [entWatch]
//
//====================================================================================================
// Recommended Sourcemod Version: 1.12.0.7225 or above
//====================================================================================================

//----------------------------------------------------------------------------------------------------
// Purpose: Force-drop an item's weapon when its holder dies or disconnects
//----------------------------------------------------------------------------------------------------
public void Ew4_Forcedrop_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType)
{
	if ((iInteractionType != EW_WEAPON_INTERACTION_DEATH) && (iInteractionType != EW_WEAPON_INTERACTION_DISCONNECT))
		return;

	if (hItem.iWeapon == INVALID_ENT_REFERENCE || !IsValidEntity(hItem.iWeapon))
		return;

	// Death/disconnect processing (or another plugin) can strip the weapon without
	// routing through SDKHook_WeaponDrop, leaving hItem.iClient stale. SDKHooks_DropWeapon
	// throws "Weapon N is not owned by client M" when m_hOwner no longer matches.
	if (GetEntPropEnt(hItem.iWeapon, Prop_Send, "m_hOwner") != iClient)
		return;

	int iSlot = SDKCall(SDKCall_GetSlot, hItem.iWeapon);
	if (iSlot >= 0 && iSlot < 2)
		SDKHooks_DropWeapon(iClient, hItem.iWeapon, NULL_VECTOR, NULL_VECTOR, false); // Using this causes ammo to disapear on the weapon
}
