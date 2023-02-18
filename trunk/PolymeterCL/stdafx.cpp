// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		23mar18	initial version
		01		01apr20	add generic context menu method
		02		06apr20	in FormatNumberCommas, emulate GetNumberFormatEx
		03		08jun21	add cast to fix narrowing conversion warning
		04		29oct22	FormatMessage call must specify ignore inserts
		05		17feb23	fix error in FormatNumberCommas when MBCS enabled
		06		17feb23	add stream file with UTF-8 support

*/

// stdafx.cpp : source file that includes just the standard includes
// PolymeterCL.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

CStdioFileEx::CStdioFileEx(LPCTSTR lpszFileName, UINT nOpenFlags, bool bUTF8)
{
	if (bUTF8) {	// if UTF-8 character encoding is enabled
		static const LPCTSTR arrModeStr[] = {
			_T("rt,ccs=UTF-8"),	// read text mode
			_T("wt,ccs=UTF-8"),	// write text mode
		};
		FILE	*fStream;
		errno_t	nErr =_tfopen_s(&fStream, lpszFileName, arrModeStr[(nOpenFlags & modeWrite) != 0]);
		if (nErr != 0) {	// if open error
			AfxThrowFileException(CFileException::OsErrorToException(nErr), nErr, lpszFileName);
		}
		CommonBaseInit(fStream, NULL);
		m_bCloseOnDelete = true;
	} else {	// default behavior
		CommonInit(lpszFileName, nOpenFlags, NULL);
	}
}
