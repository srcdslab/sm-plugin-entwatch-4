#if defined entWatch_class_item_included
	#endinput
#endif

#define entWatch_class_item_included

methodmap CItem < StringMap
{
	public CItem(CConfig hConfig = null, int iCreateID = -1)
	{
		StringMap stringMap = new StringMap();

		stringMap.SetValue("hConfig",           hConfig);

		stringMap.SetValue("iCreateID",         iCreateID);
		stringMap.SetValue("iClient",           INVALID_ENT_REFERENCE);
		stringMap.SetValue("iWeapon",           INVALID_ENT_REFERENCE);
		stringMap.SetValue("iState",            EW_ENTITY_STATE_INITIAL);

		stringMap.SetValue("flReadyTime",       0.0);

		stringMap.SetValue("hButtons",          new ArrayList());
		stringMap.SetValue("hTriggers",         new ArrayList());

		return view_as<CItem>(stringMap);
	}

	property CConfig hConfig
	{
		public get()
		{
			CConfig hValue;
			if (this.GetValue("hConfig", hValue))
				return hValue;

			return null;
		}
		public set(CConfig hValue)
		{
			this.SetValue("hConfig", hValue);
		}
	}

	property int iCreateID
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iCreateID", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iCreateID", iValue);
		}
	}

	property int iClient
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iClient", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iClient", iValue);
		}
	}

	property int iWeapon
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iWeapon", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iWeapon", iValue);
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

	property ArrayList hButtons
	{
		public get()
		{
			ArrayList hValue;
			if (this.GetValue("hButtons", hValue))
				return hValue;

			return null;
		}
		public set(ArrayList hValue)
		{
			this.SetValue("hButtons", hValue);
		}
	}

	property ArrayList hTriggers
	{
		public get()
		{
			ArrayList hValue;
			if (this.GetValue("hTriggers", hValue))
				return hValue;

			return null;
		}
		public set(ArrayList hValue)
		{
			this.SetValue("hTriggers", hValue);
		}
	}

	// Full memory teardown: frees the contained CItemButton/CItemTrigger entries
	// and the hButtons/hTriggers containers allocated by the constructor.
	// SourcePawn has no destructors, so the caller still frees the CItem itself:
	//     hItem.Delete();
	//     delete hItem;
	// (A bare `delete` on a CItem only frees the backing StringMap.)
	// Entries are typed as StringMap because this include is processed before
	// CItemButton.inc / CItemTrigger.inc.
	public void Delete()
	{
		ArrayList hButtons = this.hButtons;
		if (hButtons != null)
		{
			for (int i = 0; i < hButtons.Length; i++)
			{
				StringMap hEntry = hButtons.Get(i);
				delete hEntry;
			}

			delete hButtons;
		}

		ArrayList hTriggers = this.hTriggers;
		if (hTriggers != null)
		{
			for (int i = 0; i < hTriggers.Length; i++)
			{
				StringMap hEntry = hTriggers.Get(i);
				delete hEntry;
			}

			delete hTriggers;
		}
	}
}
