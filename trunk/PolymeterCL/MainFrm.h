// NON-FUNCTIONAL STUB

#pragma once

class CPolymeterDoc;

class CMainFrame {
public:
	CMainFrame() { m_pActiveDoc = NULL; }
	void	PostMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0) {}
	CPolymeterDoc	*GetActiveMDIDoc();
	CPolymeterDoc	*m_pActiveDoc;	// needed for CMappingArray::MapMidiEvent
};

inline CPolymeterDoc *CMainFrame::GetActiveMDIDoc()
{
	return m_pActiveDoc;
}
