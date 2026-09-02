#if defined entWatch_class_item_trigger_included
	#endinput
#endif

#define entWatch_class_item_trigger_included

// ArrayList-backed: one slot per field, direct indexed cell reads instead of a
// string hash per access. See CConfig.inc for the rationale. No string fields,
// so the default single-cell block size is enough.
enum
{
	CItemTrigger_hConfigTrigger = 0,
	CItemTrigger_hItem,
	CItemTrigger_iTrigger,
	CItemTrigger_iState,
	CItemTrigger_SLOTS
};

methodmap CItemTrigger < ArrayList
{
	public CItemTrigger(CConfigTrigger hConfigTrigger = null, CItem hItem = null)
	{
		ArrayList hData = new ArrayList();
		hData.Resize(CItemTrigger_SLOTS);

		hData.Set(CItemTrigger_hConfigTrigger, hConfigTrigger);
		hData.Set(CItemTrigger_hItem,          hItem);

		hData.Set(CItemTrigger_iTrigger,       INVALID_ENT_REFERENCE);
		hData.Set(CItemTrigger_iState,         EW_ENTITY_STATE_INITIAL);

		return view_as<CItemTrigger>(hData);
	}

	property CConfigTrigger hConfigTrigger
	{
		public get()
		{
			return view_as<CConfigTrigger>(this.Get(CItemTrigger_hConfigTrigger));
		}
		public set(CConfigTrigger hValue)
		{
			this.Set(CItemTrigger_hConfigTrigger, hValue);
		}
	}

	property CItem hItem
	{
		public get()
		{
			return view_as<CItem>(this.Get(CItemTrigger_hItem));
		}
		public set(CItem hValue)
		{
			this.Set(CItemTrigger_hItem, hValue);
		}
	}

	property int iTrigger
	{
		public get()
		{
			return this.Get(CItemTrigger_iTrigger);
		}
		public set(int iValue)
		{
			this.Set(CItemTrigger_iTrigger, iValue);
		}
	}

	property int iState
	{
		public get()
		{
			return this.Get(CItemTrigger_iState);
		}
		public set(int iValue)
		{
			this.Set(CItemTrigger_iState, iValue);
		}
	}
}
