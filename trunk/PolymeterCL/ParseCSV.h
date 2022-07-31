// NON-FUNCTIONAL STUB

#pragma once

class CParseCSV {
public:
	CParseCSV(const CString& sLine) {}
	bool	GetString(CString& sDest) { return false; }
	static	CString	ValToStr(const int& nVal) { return _T(""); }
	static	CString	ValToStr(const double& fVal) { return _T(""); }
	static	CString	ValToStr(const CString& sVal) { return _T(""); }
	static	bool	StrToVal(const CString& str, int& nVal) { return false; }
	static	bool	StrToVal(const CString& str, bool& bVal) { return false; }
	static	bool	StrToVal(const CString& str, double& fVal) { return false; }
	static	bool	StrToVal(const CString& str, CString& sVal) { return false; }
};
