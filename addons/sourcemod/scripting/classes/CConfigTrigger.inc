#if defined entWatch_class_config_trigger_included
	#endinput
#endif

#define entWatch_class_config_trigger_included

// ArrayList-backed: one slot per field, direct indexed cell reads instead of a
// string hash per access. See CConfig.inc for the rationale. No string fields,
// so the default single-cell block size is enough.
enum
{
	CConfigTrigger_hConfig = 0,
	CConfigTrigger_iConfigID,
	CConfigTrigger_iHammerID,
	CConfigTrigger_iType,
	CConfigTrigger_SLOTS
};

methodmap CConfigTrigger < ArrayList
{
	public CConfigTrigger(CConfig hConfig = null)
	{
		ArrayList hData = new ArrayList();
		hData.Resize(CConfigTrigger_SLOTS);

		hData.Set(CConfigTrigger_hConfig,   hConfig);

		hData.Set(CConfigTrigger_iConfigID, 0);
		hData.Set(CConfigTrigger_iHammerID, 0);
		hData.Set(CConfigTrigger_iType,     0);

		return view_as<CConfigTrigger>(hData);
	}

	property CConfig hConfig
	{
		public get() { return view_as<CConfig>(this.Get(CConfigTrigger_hConfig)); }
		public set(CConfig hValue) { this.Set(CConfigTrigger_hConfig, hValue); }
	}

	property int iConfigID
	{
		public get() { return this.Get(CConfigTrigger_iConfigID); }
		public set(int iValue) { this.Set(CConfigTrigger_iConfigID, iValue); }
	}

	property int iHammerID
	{
		public get() { return this.Get(CConfigTrigger_iHammerID); }
		public set(int iValue) { this.Set(CConfigTrigger_iHammerID, iValue); }
	}

	property int iType
	{
		public get() { return this.Get(CConfigTrigger_iType); }
		public set(int iValue) { this.Set(CConfigTrigger_iType, iValue); }
	}
}
