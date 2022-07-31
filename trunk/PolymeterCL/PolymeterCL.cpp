// PolymeterCL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PolymeterCL.h"
#include "Polymeter.h"
#include "PolymeterDoc.h"
#include "MainFrm.h"
#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

using namespace std;

CPolymeterApp	theApp;

CMainFrame	gMainFrame;

CPolymeterApp::CPolymeterApp()
{ 
	m_pMainFrame = &gMainFrame;	// needed for CMappingArray::MapMidiEvent
}

bool Play(CPolymeterDoc& doc, int iOutputDevice)
{
	doc.UpdateChannelEvents();
	if (doc.m_Seq.HasDubs())
		doc.m_Seq.SetSongMode(true);
	doc.m_Seq.SetOutputDevice(iOutputDevice);
	if (!doc.m_Seq.Play(true))
		return false;
	printf("press any key to stop playing\n");
	CString	sSongPos, sSongTime;
	while (!_kbhit()) {
		LONGLONG	nSongPos;
		if (doc.m_Seq.GetPosition(nSongPos)) {
			doc.m_Seq.ConvertPositionToString(nSongPos, sSongPos);
			doc.m_Seq.ConvertPositionToTimeString(nSongPos, sSongTime);
			_tprintf(_T("%10s  %s\r"), sSongPos, sSongTime);
		}
		Sleep(100);
	}
	return true;
}

bool Export(CPolymeterDoc& doc, LPCTSTR szPath)
{
	doc.UpdateChannelEvents();
	return doc.m_Seq.Export(szPath, doc.m_nSongLength, doc.m_Seq.HasDubs(), doc.m_nStartPos);
}

class CMyCommandLineInfo : public CCommandLineInfo {
public:
	CMyCommandLineInfo();
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
	enum {
		F_OUT_DEVICE,
		F_EXPORT,
		FLAGS
	};
	CString	m_sExportPath;
	int		m_iOutputDevice;

protected:
	static const LPCTSTR pszFlags[FLAGS];
	int		m_iFlag;
};

CMyCommandLineInfo::CMyCommandLineInfo()
{
	m_iFlag = -1;
	m_iOutputDevice = -1;
}

const LPCTSTR CMyCommandLineInfo::pszFlags[FLAGS] = {
	_T("D"),
	_T("E"),
};

void CMyCommandLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag) {
		CString	sFlag(pszParam);
		sFlag.MakeUpper();
		int	iFlag = ArrayFind(pszFlags, FLAGS, sFlag);
		if (iFlag >= 0)
			m_iFlag = iFlag;
		else
			m_iFlag = -1;
	} else {
		switch (m_iFlag) {
		case F_OUT_DEVICE:
			_stscanf_s(pszParam, _T("%d"), &m_iOutputDevice);
			break;
		case F_EXPORT:
			m_sExportPath = pszParam;
			break;
		}
	}
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			TRY {
				if (argc > 1) {
					CMyCommandLineInfo	infoCmdLine;
					theApp.ParseCommandLine(infoCmdLine);
					LPCTSTR	pszDocPath = argv[1];
					CPolymeterDoc	doc;
					gMainFrame.m_pActiveDoc = &doc;
					doc.ReadProperties(pszDocPath);
					if (infoCmdLine.m_sExportPath.IsEmpty()) {
						if (!Play(doc, infoCmdLine.m_iOutputDevice)) {
							_tprintf(_T("Error playing document '%s'.\n"), pszDocPath);
							nRetCode = 1;
						}
					} else {
						if (!Export(doc, infoCmdLine.m_sExportPath)) {
							_tprintf(_T("Error exporting document '%s'.\n"), pszDocPath);
							nRetCode = 1;
						}
					}
				} else {
					_tprintf(
						_T("PolymeterCL plm_path [/D out_device] [/E midi_path]\n\n")
						_T("The Polymeter document specified by plm_path is read and parsed.\n")
						_T("The document is then played, unless this behavior is overridden\n")
						_T("by flags such as /E. The default MIDI output device is used for\n")
						_T("playback, unless otherwise specified via the /D flag.\n")
						_T("\n")
						_T("/D out_device\tOutput device to use for playback, as a zero-based index.\n")
						_T("/E midi_path\tExport the document as a MIDI file, instead of playing it.\n")
					);
				}
			}
			CATCH(CException, e) {
				TCHAR	szErrorMsg[MAX_PATH];
				e->GetErrorMessage(szErrorMsg, _countof(szErrorMsg));
				_putts(szErrorMsg);
				nRetCode = 1;
			}
			END_CATCH
		}
	}
	else
	{
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
