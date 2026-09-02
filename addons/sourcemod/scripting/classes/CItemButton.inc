#if defined entWatch_class_item_button_included
	#endinput
#endif

#define entWatch_class_item_button_included

// ArrayList-backed: one slot per field, direct indexed cell reads instead of a
// string hash per access. See CConfig.inc for the rationale. No string fields,
// so the default single-cell block size is enough.
enum
{
	CItemButton_hConfigButton = 0,
	CItemButton_hItem,
	CItemButton_iButton,
	CItemButton_iState,
	CItemButton_iCurrentUses,
	CItemButton_flWaitTime,
	CItemButton_flReadyTime,
	CItemButton_SLOTS
};

methodmap CItemButton < ArrayList
{
	public CItemButton(CConfigButton hConfigButton = null, CItem hItem = null)
	{
		ArrayList hData = new ArrayList();
		hData.Resize(CItemButton_SLOTS);

		hData.Set(CItemButton_hConfigButton, hConfigButton);
		hData.Set(CItemButton_hItem,         hItem);

		hData.Set(CItemButton_iButton,       INVALID_ENT_REFERENCE);
		hData.Set(CItemButton_iState,        EW_ENTITY_STATE_INITIAL);
		hData.Set(CItemButton_iCurrentUses,  0);

		hData.Set(CItemButton_flWaitTime,    0.0);
		hData.Set(CItemButton_flReadyTime,   0.0);

		return view_as<CItemButton>(hData);
	}

	property CConfigButton hConfigButton
	{
		public get() { return view_as<CConfigButton>(this.Get(CItemButton_hConfigButton)); }
		public set(CConfigButton hValue) { this.Set(CItemButton_hConfigButton, hValue); }
	}

	property CItem hItem
	{
		public get() { return view_as<CItem>(this.Get(CItemButton_hItem)); }
		public set(CItem hValue) { this.Set(CItemButton_hItem, hValue); }
	}

	property int iButton
	{
		public get() { return this.Get(CItemButton_iButton); }
		public set(int iValue) { this.Set(CItemButton_iButton, iValue); }
	}

	property int iState
	{
		public get() { return this.Get(CItemButton_iState); }
		public set(int iValue) { this.Set(CItemButton_iState, iValue); }
	}

	property int iCurrentUses
	{
		public get() { return this.Get(CItemButton_iCurrentUses); }
		public set(int iValue) { this.Set(CItemButton_iCurrentUses, iValue); }
	}

	property float flWaitTime
	{
		public get() { return view_as<float>(this.Get(CItemButton_flWaitTime)); }
		public set(float flValue) { this.Set(CItemButton_flWaitTime, flValue); }
	}

	property float flReadyTime
	{
		public get() { return view_as<float>(this.Get(CItemButton_flReadyTime)); }
		public set(float flValue) { this.Set(CItemButton_flReadyTime, flValue); }
	}

	//------------------------------------------------------------------------------------------------
	// Read the button's live math_counter and, for a counter-typed button, write its current use
	// count into iUses. Always refreshes hConfigButton.iMaxUses from the counter bounds.
	//
	// Returns true and writes iUses only when iType is COUNTERUP / COUNTERDOWN; for any other
	// type it returns false and leaves iUses untouched (matching the original call sites, which
	// only assigned inside an explicit `if type == UP / DOWN`).
	//
	// The counter exposes m_flMin / m_flMax / m_OutValue. iMaxUses is the span (max - min). How
	// m_OutValue maps to a use count depends on the counter direction and on which meaning the
	// caller wants:
	//
	//   bValueSemantics = false  ("uses consumed", the default)
	//       COUNTERDOWN: max - out        COUNTERUP: out - min
	//   bValueSemantics = true   (raw counter value, EW_BUTTON_MODE_COUNTERVALUE)
	//       COUNTERDOWN: out - min        COUNTERUP: max - out
	//------------------------------------------------------------------------------------------------
	public bool ReadCounterUses(int &iUses, bool bValueSemantics = false)
	{
		int iButton = this.iButton;

		int iMax = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMax"));
		int iMin = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMin"));

		this.hConfigButton.iMaxUses = iMax - iMin;

		if (this.hConfigButton.iType != EW_BUTTON_TYPE_COUNTERDOWN &&
			this.hConfigButton.iType != EW_BUTTON_TYPE_COUNTERUP)
			return false;

		int iOut = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue"));

		if (this.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN)
			iUses = bValueSemantics ? (iOut - iMin) : (iMax - iOut);
		else
			iUses = bValueSemantics ? (iMax - iOut) : (iOut - iMin);

		return true;
	}
}
