// NON-FUNCTIONAL STUB

#include "resource.h"

class CMainFrame;

class CPolymeterApp : public CWinApp {
public:
	CPolymeterApp();
	CMainFrame	*GetMainFrame() const;
	CMainFrame	*m_pMainFrame;	// needed for CMappingArray::MapMidiEvent
};

extern CPolymeterApp theApp;

inline CMainFrame* CPolymeterApp::GetMainFrame() const
{
	return m_pMainFrame;
}
