#if defined entWatch_class_item_button_included
	#endinput
#endif

#define entWatch_class_item_button_included

methodmap CItemButton < StringMap
{
	public CItemButton(CConfigButton hConfigButton = null, CItem hItem = null)
	{
		StringMap stringMap = new StringMap();

		stringMap.SetValue("hConfigButton",     hConfigButton);
		stringMap.SetValue("hItem",             hItem);

		stringMap.SetValue("iButton",           INVALID_ENT_REFERENCE);
		stringMap.SetValue("iState",            EW_ENTITY_STATE_INITIAL);
		stringMap.SetValue("iCurrentUses",      0);

		stringMap.SetValue("flWaitTime",        0.0);
		stringMap.SetValue("flReadyTime",       0.0);

		return view_as<CItemButton>(stringMap);
	}

	property CConfigButton hConfigButton
	{
		public get()
		{
			CConfigButton hValue;
			if (this.GetValue("hConfigButton", hValue))
				return hValue;

			return null;
		}
		public set(CConfigButton hValue)
		{
			this.SetValue("hConfigButton", hValue);
		}
	}

	property CItem hItem
	{
		public get()
		{
			CItem hValue;
			if (this.GetValue("hItem", hValue))
				return hValue;

			return null;
		}
		public set(CItem hValue)
		{
			this.SetValue("hItem", hValue);
		}
	}

	property int iButton
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iButton", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iButton", iValue);
		}
	}

	property int iState
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iState", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iState", iValue);
		}
	}

	property int iCurrentUses
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iCurrentUses", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iCurrentUses", iValue);
		}
	}

	property float flWaitTime
	{
		public get()
		{
			float flValue;
			if (this.GetValue("flWaitTime", flValue))
				return flValue;

			return -1.0;
		}
		public set(float flValue)
		{
			this.SetValue("flWaitTime", flValue);
		}
	}

	property float flReadyTime
	{
		public get()
		{
			float flValue;
			if (this.GetValue("flReadyTime", flValue))
				return flValue;

			return -1.0;
		}
		public set(float flValue)
		{
			this.SetValue("flReadyTime", flValue);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Read the button's live math_counter and return its current use count, refreshing
	// hConfigButton.iMaxUses from the counter bounds as a side effect.
	//
	// The counter exposes m_flMin / m_flMax / m_OutValue. iMaxUses is always the span
	// (max - min). How m_OutValue maps to a use count depends on the counter direction and
	// on which meaning the caller wants:
	//
	//   bValueSemantics = false  ("uses consumed", the default)
	//       COUNTERDOWN: max - out        COUNTERUP: out - min
	//   bValueSemantics = true   (raw counter value, EW_BUTTON_MODE_COUNTERVALUE)
	//       COUNTERDOWN: out - min        COUNTERUP: max - out
	//------------------------------------------------------------------------------------------------
	public int ReadCounterUses(bool bValueSemantics = false)
	{
		int iButton = this.iButton;

		int iMin = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMin"));
		int iMax = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_flMax"));
		int iOut = RoundFloat(GetEntPropFloat(iButton, Prop_Data, "m_OutValue"));

		this.hConfigButton.iMaxUses = iMax - iMin;

		bool bCountingDown = this.hConfigButton.iType == EW_BUTTON_TYPE_COUNTERDOWN;

		if (bValueSemantics)
			return bCountingDown ? iOut - iMin : iMax - iOut;

		return bCountingDown ? iMax - iOut : iOut - iMin;
	}
}
