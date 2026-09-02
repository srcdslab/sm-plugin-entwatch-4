#if defined entWatch_class_config_button_included
	#endinput
#endif

#define entWatch_class_config_button_included

// ArrayList-backed: one slot per field, direct indexed cell reads instead of a
// string hash per access. See CConfig.inc for the rationale. Slots are 8 cells
// (32 bytes) wide to also hold the char[32] sName / sOutput fields.
enum
{
	CConfigButton_sName = 0,
	CConfigButton_sOutput,
	CConfigButton_hConfig,
	CConfigButton_iConfigID,
	CConfigButton_iHammerID,
	CConfigButton_iType,
	CConfigButton_iMode,
	CConfigButton_iMaxUses,
	CConfigButton_flButtonCooldown,
	CConfigButton_flItemCooldown,
	CConfigButton_bShowActivate,
	CConfigButton_bShowCooldown,
	CConfigButton_SLOTS
};

methodmap CConfigButton < ArrayList
{
	public CConfigButton(CConfig hConfig = null)
	{
		ArrayList hData = new ArrayList(8);
		hData.Resize(CConfigButton_SLOTS);

		hData.SetString(CConfigButton_sName,   "");
		hData.SetString(CConfigButton_sOutput, "");

		hData.Set(CConfigButton_hConfig,          hConfig);

		hData.Set(CConfigButton_iConfigID,        0);
		hData.Set(CConfigButton_iHammerID,        0);
		hData.Set(CConfigButton_iType,            0);
		hData.Set(CConfigButton_iMode,            0);
		hData.Set(CConfigButton_iMaxUses,         0);

		hData.Set(CConfigButton_flButtonCooldown, 0.0);
		hData.Set(CConfigButton_flItemCooldown,   0.0);

		hData.Set(CConfigButton_bShowActivate,    false);
		hData.Set(CConfigButton_bShowCooldown,    false);

		return view_as<CConfigButton>(hData);
	}

	public bool GetName(char[] sBuffer, int iLength)
	{
		this.GetString(CConfigButton_sName, sBuffer, iLength);
		return true;
	}

	public bool SetName(const char[] sBuffer)
	{
		this.SetString(CConfigButton_sName, sBuffer);
		return true;
	}

	public bool GetOutput(char[] sBuffer, int iLength)
	{
		this.GetString(CConfigButton_sOutput, sBuffer, iLength);
		return true;
	}

	public bool SetOutput(const char[] sBuffer)
	{
		this.SetString(CConfigButton_sOutput, sBuffer);
		return true;
	}

	property CConfig hConfig
	{
		public get() { return view_as<CConfig>(this.Get(CConfigButton_hConfig)); }
		public set(CConfig hValue) { this.Set(CConfigButton_hConfig, hValue); }
	}

	property int iConfigID
	{
		public get() { return this.Get(CConfigButton_iConfigID); }
		public set(int iValue) { this.Set(CConfigButton_iConfigID, iValue); }
	}

	property int iHammerID
	{
		public get() { return this.Get(CConfigButton_iHammerID); }
		public set(int iValue) { this.Set(CConfigButton_iHammerID, iValue); }
	}

	property int iType
	{
		public get() { return this.Get(CConfigButton_iType); }
		public set(int iValue) { this.Set(CConfigButton_iType, iValue); }
	}

	property int iMode
	{
		public get() { return this.Get(CConfigButton_iMode); }
		public set(int iValue) { this.Set(CConfigButton_iMode, iValue); }
	}

	property int iMaxUses
	{
		public get() { return this.Get(CConfigButton_iMaxUses); }
		public set(int iValue) { this.Set(CConfigButton_iMaxUses, iValue); }
	}

	property float flButtonCooldown
	{
		public get() { return view_as<float>(this.Get(CConfigButton_flButtonCooldown)); }
		public set(float flValue) { this.Set(CConfigButton_flButtonCooldown, flValue); }
	}

	property float flItemCooldown
	{
		public get() { return view_as<float>(this.Get(CConfigButton_flItemCooldown)); }
		public set(float flValue) { this.Set(CConfigButton_flItemCooldown, flValue); }
	}

	property bool bShowActivate
	{
		public get() { return view_as<bool>(this.Get(CConfigButton_bShowActivate)); }
		public set(bool bValue) { this.Set(CConfigButton_bShowActivate, bValue); }
	}

	property bool bShowCooldown
	{
		public get() { return view_as<bool>(this.Get(CConfigButton_bShowCooldown)); }
		public set(bool bValue) { this.Set(CConfigButton_bShowCooldown, bValue); }
	}
}
