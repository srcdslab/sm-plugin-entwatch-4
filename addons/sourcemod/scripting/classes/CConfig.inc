#if defined entWatch_class_config_included
	#endinput
#endif

#define entWatch_class_config_included

methodmap CConfig < StringMap
{
	public CConfig()
	{
		StringMap stringMap = new StringMap();

		stringMap.SetString("sName",            "");
		stringMap.SetString("sShort",           "");
		stringMap.SetString("sColor",           "");
		stringMap.SetString("sSpawner",         "");

		stringMap.SetValue("iConfigID",         0);
		stringMap.SetValue("iHammerID",         0);

		stringMap.SetValue("bShowMessages",     false);
		stringMap.SetValue("bShowInterface",    false);

		stringMap.SetValue("bAllowTransfer",    true);

		stringMap.SetValue("hButtons",          new ArrayList());
		stringMap.SetValue("hTriggers",         new ArrayList());

		return view_as<CConfig>(stringMap);
	}

	public bool GetName(char[] sBuffer, int iLength)
	{
		return this.GetString("sName", sBuffer, iLength);
	}

	public bool SetName(const char[] sBuffer)
	{
		return this.SetString("sName", sBuffer);
	}

	public bool GetShort(char[] sBuffer, int iLength)
	{
		return this.GetString("sShort", sBuffer, iLength);
	}

	public bool SetShort(const char[] sBuffer)
	{
		return this.SetString("sShort", sBuffer);
	}

	public bool GetColor(char[] sBuffer, int iLength)
	{
		return this.GetString("sColor", sBuffer, iLength);
	}

	public bool SetColor(const char[] sBuffer)
	{
		return this.SetString("sColor", sBuffer);
	}

	public bool GetSpawner(char[] sBuffer, int iLength)
	{
		return this.GetString("sSpawner", sBuffer, iLength);
	}

	public bool SetSpawner(const char[] sBuffer)
	{
		return this.SetString("sSpawner", sBuffer);
	}

	property int iConfigID
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iConfigID", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iConfigID", iValue);
		}
	}

	property int iHammerID
	{
		public get()
		{
			int iValue;
			if (this.GetValue("iHammerID", iValue))
				return iValue;

			return -1;
		}
		public set(int iValue)
		{
			this.SetValue("iHammerID", iValue);
		}
	}

	property bool bShowMessages
	{
		public get()
		{
			bool bValue;
			if (this.GetValue("bShowMessages", bValue))
				return bValue;

			return false;
		}
		public set(bool bValue)
		{
			this.SetValue("bShowMessages", bValue);
		}
	}

	property bool bShowInterface
	{
		public get()
		{
			bool bValue;
			if (this.GetValue("bShowInterface", bValue))
				return bValue;

			return false;
		}
		public set(bool bValue)
		{
			this.SetValue("bShowInterface", bValue);
		}
	}

	property bool bAllowTransfer
	{
		public get()
		{
			bool bValue;
			if (this.GetValue("bAllowTransfer", bValue))
				return bValue;

			return false;
		}
		public set(bool bValue)
		{
			this.SetValue("bAllowTransfer", bValue);
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

	// Full memory teardown: frees the contained CConfigButton/CConfigTrigger
	// entries and the hButtons/hTriggers containers allocated by the constructor.
	// SourcePawn has no destructors, so the caller still frees the CConfig itself:
	//     hConfig.Delete();
	//     delete hConfig;
	// Entries are typed as StringMap because this include is processed before
	// CConfigButton.inc / CConfigTrigger.inc.
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
