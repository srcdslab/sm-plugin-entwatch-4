#if defined entWatch_class_item_included
	#endinput
#endif

#define entWatch_class_item_included

// ArrayList-backed: one slot per field, direct indexed cell reads instead of a
// string hash per access. See CConfig.inc for the rationale. No string fields,
// so the default single-cell block size is enough.
enum
{
	CItem_hConfig = 0,
	CItem_iCreateID,
	CItem_iClient,
	CItem_iWeapon,
	CItem_iState,
	CItem_flReadyTime,
	CItem_hButtons,
	CItem_hTriggers,
	CItem_SLOTS
};

methodmap CItem < ArrayList
{
	public CItem(CConfig hConfig = null, int iCreateID = -1)
	{
		ArrayList hData = new ArrayList();
		hData.Resize(CItem_SLOTS);

		hData.Set(CItem_hConfig,     hConfig);

		hData.Set(CItem_iCreateID,   iCreateID);
		hData.Set(CItem_iClient,     INVALID_ENT_REFERENCE);
		hData.Set(CItem_iWeapon,     INVALID_ENT_REFERENCE);
		hData.Set(CItem_iState,      EW_ENTITY_STATE_INITIAL);

		hData.Set(CItem_flReadyTime, 0.0);

		hData.Set(CItem_hButtons,    new ArrayList());
		hData.Set(CItem_hTriggers,   new ArrayList());

		return view_as<CItem>(hData);
	}

	property CConfig hConfig
	{
		public get() { return view_as<CConfig>(this.Get(CItem_hConfig)); }
		public set(CConfig hValue) { this.Set(CItem_hConfig, hValue); }
	}

	property int iCreateID
	{
		public get() { return this.Get(CItem_iCreateID); }
		public set(int iValue) { this.Set(CItem_iCreateID, iValue); }
	}

	property int iClient
	{
		public get() { return this.Get(CItem_iClient); }
		public set(int iValue) { this.Set(CItem_iClient, iValue); }
	}

	property int iWeapon
	{
		public get() { return this.Get(CItem_iWeapon); }
		public set(int iValue) { this.Set(CItem_iWeapon, iValue); }
	}

	property int iState
	{
		public get() { return this.Get(CItem_iState); }
		public set(int iValue) { this.Set(CItem_iState, iValue); }
	}

	property float flReadyTime
	{
		public get() { return view_as<float>(this.Get(CItem_flReadyTime)); }
		public set(float flValue) { this.Set(CItem_flReadyTime, flValue); }
	}

	property ArrayList hButtons
	{
		public get() { return view_as<ArrayList>(this.Get(CItem_hButtons)); }
		public set(ArrayList hValue) { this.Set(CItem_hButtons, hValue); }
	}

	property ArrayList hTriggers
	{
		public get() { return view_as<ArrayList>(this.Get(CItem_hTriggers)); }
		public set(ArrayList hValue) { this.Set(CItem_hTriggers, hValue); }
	}

	// Full memory teardown: frees the contained CItemButton/CItemTrigger entries
	// and the hButtons/hTriggers containers allocated by the constructor.
	// SourcePawn has no destructors, so the caller still frees the CItem itself:
	//     hItem.Delete();
	//     delete hItem;
	// (A bare `delete` on a CItem only frees the backing ArrayList.)
	// Entries are typed as ArrayList because this include is processed before
	// CItemButton.inc / CItemTrigger.inc.
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
