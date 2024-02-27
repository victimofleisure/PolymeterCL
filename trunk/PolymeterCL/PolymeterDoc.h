// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23mar18	initial version
		01		18nov18	add center song position hint
		02		28nov18	add undo handlers for modify parts and refactor names
		03		02dec18	add recording of MIDI input
		04		10dec18	add song time shift to handle negative times
		05		18dec18	add import/export tracks
		06		19dec18	move track property names into base class
		07		16jan19	refactor insert tracks to standardize usage
		08		04feb19	add track offset command
		09		14feb19	refactor export to avoid track mode special cases
		10		22mar19	add track invert command
		11		15nov19	add option for signed velocity scaling
		12		29feb20	add support for recording live events
		13		18mar20	cache song position in document
		14		20mar20	add mapping
		15		29mar20	add learn multiple mappings
		16		07apr20	add move steps
		17		19apr20	optimize undo/redo menu item prefixing
		18		30apr20	add velocity only flag to set step methods
		19		03jun20	in record undo, restore recorded MIDI events 
		20		13jun20	add find convergence
		21		18jun20	add track modulation command
		22		09jul20	move view type handling from document to child frame
		23		28sep20	add part sort
		24		07oct20	in stretch tracks, make interpolation optional
		25		06nov20	refactor velocity transforms and add replace
		26		16nov20	add tick dependencies
		27		19nov20	add set channel property methods
		28		04dec20	in goto next dub, pass target track, return dub track
		29		16dec20	add looping of playback
		30		24jan21	add prime factors command
		31		07jun21	rename rounding functions
		32		25oct21 in sort mappings, add optional sort direction
		33		30oct21	move song duration method from sequencer to here
		34		23nov21	add method to export tempo map
		35		15feb22	add check modulations command
		36		19feb22	use INI file class directly instead of via profile
		37		19may22	add loop ruler selection attribute
		38		25oct22	add command to select all unmuted tracks
		39		16dec22	add quant string conversions that support fractions
		40		29jan24	add class to save and restore track selection
		41		16feb24	move track color message handlers here
		42		25feb24	single handler for next and prev convergence

*/

// PolymeterDoc.h : interface of the CPolymeterDoc class
//

#pragma once

#include "Sequencer.h"
#include "MasterProps.h"
#include "Channel.h"
#include "Preset.h"
#include "Mapping.h"

class COptions;
class CIniFile;

class CPolymeterDoc : public CMasterProps, public CTrackBase
{
public:
// Construction
	CPolymeterDoc();
	~CPolymeterDoc();

// Constants
	enum {
		INIT_TRACKS = 32,		// initial track count
		MAX_TRACKS = USHRT_MAX,	// limiting factor is 16-bit track indices in undo implementation
	};
	enum {	// view types
		#define VIEWTYPEDEF(name) VIEW_##name,
		#include "ViewTypeDef.h"	// generate view type enum
		VIEW_TYPES,
		DEFAULT_VIEW_TYPE = VIEW_Track
	};

// Types
	class CMySequencer : public CSequencer {
	public:
		virtual	void	OnMidiError(MMRESULT nResult);
	};

// Public data
	int		m_nFileVersion;		// file version number
	CMySequencer	m_Seq;			// sequencer
	CChannelArray	m_arrChannel;	// array of channels
	CPresetArray	m_arrPreset;	// array of presets
	CTrackGroupArray	m_arrPart;	// array of parts
	double	m_fStepZoom;		// step view zoom
	double	m_fSongZoom;		// song view zoom
	int		m_nViewType;		// see view type enum; child frames may have different view types
	LONGLONG	m_nSongPos;		// cached song position

// Attributes
	int		GetTrackCount() const;

// Operations
public:
	void	ReadProperties(LPCTSTR szPath);
	void	WriteProperties(LPCTSTR szPath) const;
	void	InitChannelArray();
	void	UpdateChannelEvents();

// Overrides
public:

// Implementation

protected:
// Types

// Constants
	static const LPCTSTR	m_arrViewTypeName[];	// array of view type names

// Data members

// Overrides

// Helpers
	void	ReadTrackModulations(CIniFile& fIni, CString sTrkID, CTrack& trk);
	void	WriteTrackModulations(CIniFile& fIni, CString sTrkID, const CTrack& trk) const;
	void	ConvertLegacyFileFormat();
	void	ReadEnum(CIniFile& fIni, LPCTSTR pszSection, LPCTSTR pszKey, int& Value, const CProperties::OPTION_INFO *pOption, int nOptions);
	void	WriteEnum(CIniFile& fIni, LPCTSTR pszSection, LPCTSTR pszKey, const int& Value, const CProperties::OPTION_INFO *pOption, int nOptions) const;
	template<class T> void ReadEnum(CIniFile&, LPCTSTR, LPCTSTR, T&, const CProperties::OPTION_INFO*, int);
	template<class T> void WriteEnum(CIniFile&, LPCTSTR, LPCTSTR, const T&, const CProperties::OPTION_INFO*, int) const;
};

inline int CPolymeterDoc::GetTrackCount() const
{
	return m_Seq.GetTrackCount();
}
