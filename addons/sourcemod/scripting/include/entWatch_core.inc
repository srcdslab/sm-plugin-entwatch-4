// ============================================================================
//
//  entWatch-4 API
//
// ============================================================================

#if defined _entWatch_included
    #endinput
#endif
#define _entWatch_included


// ============================================================================
//  Versioning
// ============================================================================

#define EW_VERSION          "4.2.2"
#define EW_VERSION_CONFIG   2


// ============================================================================
//  Constants
// ============================================================================

/**
 * Entity lifecycle states.
 * Returned by CItem.GetState() to describe where an item currently is.
 */
#define EW_ENTITY_STATE_INITIAL     0
#define EW_ENTITY_STATE_SPAWNED     1
#define EW_ENTITY_STATE_EQUIPPED    2
#define EW_ENTITY_STATE_DROPPED     3
#define EW_ENTITY_STATE_DESTROYED   4

/**
 * Weapon interaction event types.
 * Passed as iInteractionType in EW_OnClientItemWeaponInteract.
 */
#define EW_WEAPON_INTERACTION_DROP          1
#define EW_WEAPON_INTERACTION_DEATH         2
#define EW_WEAPON_INTERACTION_PICKUP        3
#define EW_WEAPON_INTERACTION_DISCONNECT    4

/**
 * Button activation types.
 * Defines how a tracked button entity is triggered.
 */
#define EW_BUTTON_TYPE_NONE         0
#define EW_BUTTON_TYPE_USE          1
#define EW_BUTTON_TYPE_OUTPUT       2
#define EW_BUTTON_TYPE_COUNTERUP    3
#define EW_BUTTON_TYPE_COUNTERDOWN  4

/**
 * Button cooldown/usage modes.
 * Defines the usage and cooldown behaviour of a tracked button.
 */
#define EW_BUTTON_MODE_NONE             0
#define EW_BUTTON_MODE_COOLDOWN         1
#define EW_BUTTON_MODE_MAXUSES          2
#define EW_BUTTON_MODE_COOLDOWN_CHARGES 3
#define EW_BUTTON_MODE_COUNTERVALUE     4

/**
 * Trigger entity roles.
 * Defines the purpose of a tracked trigger entity.
 */
#define EW_TRIGGER_TYPE_NONE    0
#define EW_TRIGGER_TYPE_STRIP   1
#define EW_TRIGGER_TYPE_BUTTON  2
#define EW_TRIGGER_TYPE_HURT    3


// ============================================================================
//  Class Includes
// ============================================================================

#include "../classes/CConfig.inc"
#include "../classes/CConfigButton.inc"
#include "../classes/CConfigTrigger.inc"

#include "../classes/CItem.inc"
#include "../classes/CItemButton.inc"
#include "../classes/CItemTrigger.inc"


// ============================================================================
//  Forwards — Interaction Hooks (Blocking)
//
//  These forwards are called BEFORE an interaction occurs.
//  Returning false will block the interaction from being processed.
// ============================================================================

/**
 * Called before a client interacts with a tracked weapon entity.
 * Return false to block the interaction.
 *
 * @param iClient       Client index.
 * @param hItem         CItem handle for the item being interacted with.
 *
 * @return              true to allow, false to block.
 */
forward bool EW_OnClientItemWeaponCanInteract(int iClient, CItem hItem);

/**
 * Called before a client activates a tracked button entity.
 * Return false to block the activation.
 *
 * @param iClient       Client index.
 * @param hItemButton   CItemButton handle for the button being activated.
 *
 * @return              true to allow, false to block.
 */
forward bool EW_OnClientItemButtonCanInteract(int iClient, CItemButton hItemButton);

/**
 * Called before a client touches a tracked trigger entity.
 * Return false to prevent the trigger from firing.
 *
 * @param iClient           Client index.
 * @param hItemTrigger      CItemTrigger handle for the trigger being touched.
 *
 * @return              true to allow, false to block.
 */
forward bool EW_OnClientItemTriggerCanInteract(int iClient, CItemTrigger hItemTrigger);


// ============================================================================
//  Forwards — Interaction Notifications (Post)
//
//  These forwards are called AFTER an interaction has been processed.
//  They are informational only and cannot block anything.
// ============================================================================

/**
 * Called after a client interacts with a tracked weapon entity.
 *
 * @param iClient           Client index.
 * @param hItem             CItem handle for the affected item.
 * @param iInteractionType  One of the EW_WEAPON_INTERACTION_* constants.
 *
 * @noreturn
 */
forward void EW_OnClientItemWeaponInteract(int iClient, CItem hItem, int iInteractionType);

/**
 * Called after a client activates a tracked button entity.
 *
 * @param iClient       Client index.
 * @param hItemButton   CItemButton handle for the activated button.
 *
 * @noreturn
 */
forward void EW_OnClientItemButtonInteract(int iClient, CItemButton hItemButton);

/**
 * Called after a client touches a tracked trigger entity.
 *
 * @param iClient           Client index.
 * @param hItemTrigger      CItemTrigger handle for the touched trigger.
 *
 * @noreturn
 */
forward void EW_OnClientItemTriggerInteract(int iClient, CItemTrigger hItemTrigger);


// ============================================================================
//  Forwards — Admin Actions
//
//  Called after an admin performs a privileged action, either via
//  in-game command or programmatically through a native.
// ============================================================================

/**
 * Called after an admin transfers an item to a player.
 *
 * @param iAdmin        Admin client index (0 = server).
 * @param hItem         CItem handle for the transferred item.
 * @param iReceiver     Client index who received the item.
 *
 * @noreturn
 */
forward void EW_OnAdminTransferItem(int iAdmin, CItem hItem, int iReceiver);

/**
 * Called after an admin spawns an item for a player.
 *
 * @param iAdmin        Admin client index (0 = server).
 * @param sItemName     Config name of the spawned item.
 * @param iReceiver     Client index who received the item.
 *
 * @noreturn
 */
forward void EW_OnAdminSpawnItem(int iAdmin, const char[] sItemName, int iReceiver);

/**
 * Called after a client is restricted (eban) by an admin.
 *
 * @param iAdmin        Admin client index (0 = server).
 * @param iClient       Client index who was restricted.
 * @param iDuration     Duration in minutes. -1 = session, 0 = permanent, >0 = minutes.
 * @param sReason       Reason string supplied by the admin.
 *
 * @noreturn
 */
forward void EW_OnClientRestricted(int iAdmin, int iClient, int iDuration, const char[] sReason);

/**
 * Called after a client is unrestricted (eunban) by an admin.
 *
 * @param iAdmin        Admin client index (0 = server).
 * @param iClient       Client index who was unrestricted.
 * @param sReason       Reason string supplied by the admin.
 *
 * @noreturn
 */
forward void EW_OnClientUnrestricted(int iAdmin, int iClient, const char[] sReason);


// ============================================================================
//  Forwards — Broadcast Events
//
//  Called when entWatch broadcasts an admin action to all connected clients.
//  Useful for custom logging, Discord relays, or UI overlays.
// ============================================================================

/**
 * Called when a restriction is broadcast to all connected clients.
 *
 * @param iAdmin        Admin client index.
 * @param iClient       Client index being restricted.
 * @param iDuration     Duration in minutes. -1 = session, 0 = permanent, >0 = minutes.
 * @param sReason       Restriction reason.
 * @param sAdminName    Display name of the admin.
 * @param sSteamID      SteamID of the admin.
 *
 * @noreturn
 */
forward void EW_OnRestrictBroadcast(int iAdmin, int iClient, int iDuration, const char[] sReason, const char[] sAdminName, const char[] sSteamID);

/**
 * Called when an unrestriction is broadcast to all connected clients.
 *
 * @param iAdmin        Admin client index.
 * @param iClient       Client index being unrestricted.
 * @param sReason       Unrestriction reason.
 * @param sAdminName    Display name of the admin.
 * @param sSteamID      SteamID of the admin.
 *
 * @noreturn
 */
forward void EW_OnUnrestrictBroadcast(int iAdmin, int iClient, const char[] sReason, const char[] sAdminName, const char[] sSteamID);

/**
 * Called when a restriction for an offline player is broadcast to all connected clients.
 *
 * @param iAdmin        Admin client index.
 * @param iClient       Offline client index (may be invalid/disconnected).
 * @param sReason       Restriction reason.
 * @param sAdminName    Display name of the admin.
 * @param sSteamID      SteamID of the admin.
 * @param sPlayerName   Display name of the offline player.
 *
 * @noreturn
 */
forward void EW_OnOfflineRestrictBroadcast(int iAdmin, int iClient, const char[] sReason, const char[] sAdminName, const char[] sSteamID, const char[] sPlayerName);


// ============================================================================
//  Natives — Core
// ============================================================================

/**
 * Reloads the entWatch configuration from disk.
 *
 * @param bLoopEntities     If true, re-scans all world entities after loading
 *                          to re-register any tracked items. Default: false.
 *
 * @return                  true on success, false if the config failed to load.
 */
native bool EW_LoadConfig(bool bLoopEntities = false);

/**
 * Returns the live array of all currently tracked items.
 *
 * @return      ArrayList of CItem handles.
 * @note        This handle is owned by entWatch-core. Do NOT close it.
 */
native ArrayList EW_GetItemsArray();

/**
 * Returns the array of all loaded item configuration entries.
 *
 * @return      ArrayList of CConfig handles.
 * @note        This handle is owned by entWatch-core. Do NOT close it.
 */
native ArrayList EW_GetConfigsArray();

/**
 * Checks whether the given entity is currently tracked by entWatch.
 *
 * @param iEntity   Entity index to query.
 *
 * @return          true if tracked, false otherwise.
 */
native bool EW_IsEntityItem(int iEntity);

/**
 * Checks whether the given client is currently holding any tracked item.
 *
 * @param iClient   Client index to query.
 *
 * @return          true if the client holds at least one tracked item, false otherwise.
 */
native bool EW_ClientHasItem(int iClient);


// ============================================================================
//  Natives — Admin Actions
// ============================================================================

/**
 * Transfers a tracked item to a client.
 *
 * @param iAdmin        Admin client index authorising the transfer (0 = server).
 * @param iItemID       Index of the item in EW_GetItemsArray().
 * @param iReceiver     Client index who will receive the item.
 * @param bMessage      If true, broadcasts a chat/log message. Default: true.
 *
 * @return              true on success, false if the transfer could not be completed.
 */
native bool EW_TransferItem(int iAdmin, int iItemID, int iReceiver, bool bMessage = true);

/**
 * Spawns a tracked item for a client.
 *
 * @param iAdmin        Admin client index authorising the spawn (0 = server).
 * @param iItemID       Index of the item config in EW_GetConfigsArray().
 * @param iReceiver     Client index who will receive the spawned item.
 * @param bStrip        If true, strips the receiver's weapons before spawning. Default: false.
 *
 * @return              true on success, false if the spawn could not be completed.
 */
native bool EW_SpawnItem(int iAdmin, int iItemID, int iReceiver, bool bStrip = false);


// ============================================================================
//  Natives — Restriction System
// ============================================================================


/**
 * Restricts (ebans) a client from interacting with tracked items.
 *
 * @param iAdmin        Admin client index issuing the restriction (0 = server).
 * @param iClient       Client index to restrict.
 * @param iDuration     Duration in minutes. -1 = session, 0 = permanent, >0 = minutes.
 * @param sReason       Reason string to store and display (max 64 chars).
 *
 * @return              true on success, false on error.
 */
native bool EW_ClientRestrict(int iAdmin, int iClient, int iDuration, const char[] sReason);

/**
 * Removes an active restriction (eunban) from a client.
 *
 * @param iAdmin        Admin client index lifting the restriction (0 = server).
 * @param iClient       Client index to unrestrict.
 * @param sReason       Reason string for the unrestriction log (max 64 chars).
 *
 * @return              true on success, false on error.
 */
native bool EW_ClientUnrestrict(int iAdmin, int iClient, const char[] sReason);

/**
 * Returns whether a client currently has an active entWatch restriction.
 *
 * @param iClient   Client index to query.
 *
 * @return          true if restricted, false if not, -1 on error.
 */
native bool EW_IsRestrictedClient(int iClient);

/**
 * Returns the total number of recorded bans for a client.
 *
 * @param iClient   Client index to query.
 *
 * @return          Number of bans on record, or -1 on error.
 */
native int EW_GetClientBanCount(int iClient);

/**
 * Retrieves ban information for a restricted client.
 *
 * @param iClient           Client index to query.
 * @param sAdminName        Output buffer for admin name.
 * @param iAdminNameLen     Length of admin name buffer.
 * @param sAdminSteamID     Output buffer for admin SteamID.
 * @param iAdminSteamIDLen  Length of admin SteamID buffer.
 * @param sReason           Output buffer for ban reason.
 * @param iReasonLen        Length of reason buffer.
 * @param iDuration         Reference to receive ban duration.
 * @param iIssuedAt         Reference to receive ban issue timestamp.
 * @param iTotalEbans       Reference to receive total ban count.
 *
 * @return                  1 if info was written, 0 if not restricted, -1 on error.
 */
native int EW_GetClientBanInfo(int iClient, char[] sAdminName, int iAdminNameLen, char[] sAdminSteamID, int iAdminSteamIDLen, char[] sReason, int iReasonLen, int &iDuration, int &iIssuedAt, int &iTotalEbans);

/**
 * Opens the ban-reason selection menu for a client.
 *
 * @param iAdmin        Admin client index (must be connected).
 * @param iClient       Client index to restrict.
 * @param iDuration     Duration in minutes (>= -1).
 *
 * @return              true if menu was shown, false if already restricted or error.
 */
native bool EW_ShowBanReasonMenu(int iAdmin, int iClient, int iDuration);

/**
 * Opens the unban-reason selection menu for a client.
 *
 * @param iAdmin        Admin client index (must be connected).
 * @param iClient       Client index to unrestrict.
 *
 * @return              true if menu was shown, false if not restricted or error.
 */
native bool EW_ShowUnbanReasonMenu(int iAdmin, int iClient);


// ============================================================================
//  Plugin Dependency Declaration
// ============================================================================

public SharedPlugin __pl_entWatch_core =
{
    name = "entWatch-core",
    file = "entWatch-core.smx",

#if defined REQUIRE_PLUGIN
    required = 1,
#else
    required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_entWatch_core_SetNTVOptional()
{
    MarkNativeAsOptional("EW_LoadConfig");
    MarkNativeAsOptional("EW_GetItemsArray");
    MarkNativeAsOptional("EW_GetConfigsArray");
    MarkNativeAsOptional("EW_IsEntityItem");
    MarkNativeAsOptional("EW_ClientHasItem");
    MarkNativeAsOptional("EW_TransferItem");
    MarkNativeAsOptional("EW_SpawnItem");
    MarkNativeAsOptional("EW_ClientRestrict");
    MarkNativeAsOptional("EW_ClientUnrestrict");
    MarkNativeAsOptional("EW_IsRestrictedClient");
    MarkNativeAsOptional("EW_GetClientBanCount");
    MarkNativeAsOptional("EW_GetClientBanInfo");
    MarkNativeAsOptional("EW_ShowBanReasonMenu");
    MarkNativeAsOptional("EW_ShowUnbanReasonMenu");
}
#endif