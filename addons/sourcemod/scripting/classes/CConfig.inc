#if defined entWatch_class_config_included
	#endinput
#endif

#define entWatch_class_config_included

//------------------------------------------------------------------------------------------------
// The entWatch class layer (CConfig / CConfigButton / CConfigTrigger / CItem / CItemButton /
// CItemTrigger) is backed by a fixed-layout ArrayList instead of a StringMap. Every field owns
// one ArrayList slot (one block), addressed by the CConfig_* enum below, so a field access is a
// direct indexed cell read instead of the StringMap's per-access string hash + bucket walk.
// These accessors sit on the plugin's hottest paths (OnButtonPress / OnButtonOutput /
// OnCounterOutput / OnTriggerTouch / OnWeaponTouch / OnDisplayHUD / OnBeacons / OnEntitySpawnPost).
//
// The class stays a Handle: EW_GetConfigsArray() / EW_GetItemsArray() still return ArrayLists of
// these, the interaction forwards still pass them by cell, and `hEntry == null` still works, so
// no consumer (module or external plugin) has to change.
//
// Slots are 8 cells (32 bytes) wide so a slot can alternatively hold one of this class's char[32]
// string fields (sName / sSpawner) through ArrayList.GetString / ArrayList.SetString.
//------------------------------------------------------------------------------------------------
enum
{
	CConfig_sName = 0,
	CConfig_sShort,
	CConfig_sColor,
	CConfig_sSpawner,
	CConfig_iConfigID,
	CConfig_iHammerID,
	CConfig_bShowMessages,
	CConfig_bShowInterface,
	CConfig_bAllowTransfer,
	CConfig_hButtons,
	CConfig_hTriggers,
	CConfig_SLOTS
};

methodmap CConfig < ArrayList
{
	public CConfig()
	{
		ArrayList hData = new ArrayList(8);
		hData.Resize(CConfig_SLOTS);

		hData.SetString(CConfig_sName,    "");
		hData.SetString(CConfig_sShort,   "");
		hData.SetString(CConfig_sColor,   "");
		hData.SetString(CConfig_sSpawner, "");

		hData.Set(CConfig_iConfigID,      0);
		hData.Set(CConfig_iHammerID,      0);

		hData.Set(CConfig_bShowMessages,  false);
		hData.Set(CConfig_bShowInterface, false);

		hData.Set(CConfig_bAllowTransfer, true);

		hData.Set(CConfig_hButtons,       new ArrayList());
		hData.Set(CConfig_hTriggers,      new ArrayList());

		return view_as<CConfig>(hData);
	}

	public bool GetName(char[] sBuffer, int iLength)
	{
		this.GetString(CConfig_sName, sBuffer, iLength);
		return true;
	}

	public bool SetName(const char[] sBuffer)
	{
		this.SetString(CConfig_sName, sBuffer);
		return true;
	}

	public bool GetShort(char[] sBuffer, int iLength)
	{
		this.GetString(CConfig_sShort, sBuffer, iLength);
		return true;
	}

	public bool SetShort(const char[] sBuffer)
	{
		this.SetString(CConfig_sShort, sBuffer);
		return true;
	}

	public bool GetColor(char[] sBuffer, int iLength)
	{
		this.GetString(CConfig_sColor, sBuffer, iLength);
		return true;
	}

	public bool SetColor(const char[] sBuffer)
	{
		this.SetString(CConfig_sColor, sBuffer);
		return true;
	}

	public bool GetSpawner(char[] sBuffer, int iLength)
	{
		this.GetString(CConfig_sSpawner, sBuffer, iLength);
		return true;
	}

	public bool SetSpawner(const char[] sBuffer)
	{
		this.SetString(CConfig_sSpawner, sBuffer);
		return true;
	}

	property int iConfigID
	{
		public get() { return this.Get(CConfig_iConfigID); }
		public set(int iValue) { this.Set(CConfig_iConfigID, iValue); }
	}

	property int iHammerID
	{
		public get() { return this.Get(CConfig_iHammerID); }
		public set(int iValue) { this.Set(CConfig_iHammerID, iValue); }
	}

	property bool bShowMessages
	{
		public get() { return view_as<bool>(this.Get(CConfig_bShowMessages)); }
		public set(bool bValue) { this.Set(CConfig_bShowMessages, bValue); }
	}

	property bool bShowInterface
	{
		public get() { return view_as<bool>(this.Get(CConfig_bShowInterface)); }
		public set(bool bValue) { this.Set(CConfig_bShowInterface, bValue); }
	}

	property bool bAllowTransfer
	{
		public get() { return view_as<bool>(this.Get(CConfig_bAllowTransfer)); }
		public set(bool bValue) { this.Set(CConfig_bAllowTransfer, bValue); }
	}

	property ArrayList hButtons
	{
		public get() { return view_as<ArrayList>(this.Get(CConfig_hButtons)); }
		public set(ArrayList hValue) { this.Set(CConfig_hButtons, hValue); }
	}

	property ArrayList hTriggers
	{
		public get() { return view_as<ArrayList>(this.Get(CConfig_hTriggers)); }
		public set(ArrayList hValue) { this.Set(CConfig_hTriggers, hValue); }
	}

	// Full memory teardown: frees the contained CConfigButton/CConfigTrigger
	// entries and the hButtons/hTriggers containers allocated by the constructor.
	// SourcePawn has no destructors, so the caller still frees the CConfig itself:
	//     hConfig.Delete();
	//     delete hConfig;
	// Entries are typed as ArrayList because this include is processed before
	// CConfigButton.inc / CConfigTrigger.inc.
	public void Delete()
	{
		ArrayList hButtons = this.hButtons;
		if (hButtons != null)
		{
			for (int i = 0; i < hButtons.Length; i++)
			{
				ArrayList hEntry = hButtons.Get(i);
				delete hEntry;
			}

			delete hButtons;
		}

		ArrayList hTriggers = this.hTriggers;
		if (hTriggers != null)
		{
			for (int i = 0; i < hTriggers.Length; i++)
			{
				ArrayList hEntry = hTriggers.Get(i);
				delete hEntry;
			}

			delete hTriggers;
		}
	}
}
