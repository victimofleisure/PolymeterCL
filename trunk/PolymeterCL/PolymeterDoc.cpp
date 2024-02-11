// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23mar18	initial version
		01		18nov18	add goto next dub method
		02		20nov18	bump file version for recursive modulation
		03		20nov18	in FillTrack, fix non-rectangular selection case
		04		28nov18	in part overlap check, add option to resolve conflicts
		05		02dec18	add recording of MIDI input
		06		07dec18	in track export, don't export modulator tracks
		07		08dec18	bump file version for negative dub times
		08		10dec18	add song time shift to handle negative times
		09		13dec18	add import/export steps commands
		10		14dec18	bump file version for note range, position modulation
		11		15dec18	add import/export modulations commands
		12		17dec18	move MIDI file types into class scope
		13		18dec18	add import/export tracks
		14		19dec18	move track property names into base class
		15		03jan19	in Play, add support for MIDI output bar
		16		07jan19	in Play, add support for piano bar
		17		14jan19	bump file version for recursive position modulation
		18		16jan19	refactor insert tracks to standardize usage
		19		04feb19	add track offset command
		20		10feb19	add tools song export of MIDI data to CSV file
		21		14feb19	refactor export to avoid track mode special cases
		22		20feb19	add note overlap prevention
		23		22mar19	add track invert command
		24		09sep19	bump file version for tempo track type and modulation
		25		15nov19	add option for signed velocity scaling
		26		12dec19	add GetPeriod
		27		26dec19	in TimeToRepeat, report fractional beats
		28		24feb20	use new INI file implementation
		29		29feb20	add support for recording live events
		30		16mar20	bump file version for new modulation types
		31		18mar20	cache song position in document
		32		20mar20	add mapping
		33		29mar20	add learn multiple mappings
		34		03apr20	refactor go to position dialog for variable format
		35		04apr20	bump file version for chord modulation
		36		07apr20	add move steps; fix cut steps undo code
		37		14apr20	add send MIDI clock option
		38		17apr20	add track color; bump file version
		39		19apr20	optimize undo/redo menu item prefixing
		40		30apr20	add velocity only flag to set step methods
		41		19may20	refactor record dub methods to include conditional
		42		23may20	bump file version for negative voicing modulation
		43		03jun20	in record undo, restore recorded MIDI events 
		44		13jun20	add find convergence
		45		18jun20	add track modulation command
		46		05jul20	pass document pointer to UpdateSongPosition
		47		09jul20	move view type handling from document to child frame
		48		17jul20	in read properties, set cached song position
		49		28sep20	add part sort
        50      07oct20	in TimeToRepeat, standardize unique period method
		51		07oct20	in stretch tracks, make interpolation optional
		52		06nov20	refactor velocity transforms and add replace
		53		16nov20	on time division change, update tick dependencies
		54		19nov20	add set channel property methods
		55		04dec20	in goto next dub, pass target track, return dub track
		56		16dec20	add looping of playback
		57		23jan21	make fill dialog's step range one-origin
		58		24jan21	add prime factors command
		59		10feb21	use set track property overload for selected tracks
		60		07jun21	rename rounding functions
		61		08jun21	fix local name reuse warning
		62		08jun21	fix warning for CString as variadic argument
		63		20jun21	move focus edit handling to child frame
		64		19jul21	enable stretch for track selection only
		65		25oct21 in sort mappings, add optional sort direction
		66		30oct21	in time shift calc, handle positive case also
		67		30oct21	song duration method must account for start position
		68		31oct21	suppress view type notification when opening document
		69		15nov21	add tempo map to export song as CSV
		70		23nov21	add method to export tempo map
		71		21jan22	add note overlap method and bump file version
		72		05feb22	bump file version for tie mapping
		73		15feb22	add check modulations command
		74		19feb22	use INI file class directly instead of via profile
		75		19may22	add loop ruler selection attribute
		76		05jul22	use wrapper class to save and restore focus
		77		25oct22	add command to select all unmuted tracks
		78		16dec22	add quant string conversions that support fractions
		79		16feb23	add special handling for non-ASCII characters
		80		17feb23	add replace range to velocity transform
		81		20sep23	in track fill, fix divide by zero errors
		82		27nov23	specify key signature in OnFileExport
		83		19dec23	bump file version for internal track type
		84		24jan24	use sequencer's warning error attribute
		85		29jan24	add class to save and restore track selection

*/

// PolymeterDoc.cpp : implementation of the CPolymeterDoc class
//

#include "stdafx.h"
#include "Polymeter.h"
#include "PolymeterDoc.h"
#include "MainFrm.h"
#include "IniFile.h"
#include "Properties.h"

// CPolymeterDoc

// file versioning
#define FILE_ID				_T("Polymeter")
#define	FILE_VERSION		21

// file format keys
#define RK_FILE_ID			_T("FileID")
#define RK_FILE_VERSION		_T("FileVersion")

// track member keys
#define RK_TRACK_COUNT		_T("Tracks")
#define RK_TRACK_IDX		_T("Track%d")
#define RK_TRACK_LENGTH		_T("Length")
#define RK_TRACK_COLOR		_T("Color")
#define RK_TRACK_STEP		_T("Step")
#define RK_TRACK_DUB_ARRAY	_T("Dub")
#define RK_TRACK_DUB_COUNT	_T("Dubs")
#define RK_TRACK_MOD_LIST	_T("Mods")

// other document member keys
#define RK_MASTER			_T("Master")
#define RK_STEP_ZOOM		_T("Zoom")
#define RK_SONG_ZOOM		_T("Zoom")
#define RK_VIEW_TYPE		_T("ViewType")
#define RK_PART_SECTION		_T("Part")
#define RK_RECORD_EVENTS	_T("RecordEvents")

#define RK_SONG_VIEW		_T("SongView")
#define RK_STEP_VIEW		_T("StepView")

const LPCTSTR CPolymeterDoc::m_arrViewTypeName[VIEW_TYPES] = {
	#define VIEWTYPEDEF(name) _T(#name),
	#include "ViewTypeDef.h"	// generate view type name array
};

// CPolymeterDoc construction/destruction

CPolymeterDoc::CPolymeterDoc() 
{
	m_nFileVersion = FILE_VERSION;
	InitChannelArray();
	m_fStepZoom = 1;
	m_fSongZoom = 1;
	m_nViewType = DEFAULT_VIEW_TYPE;
	m_nSongPos = 0;
}

CPolymeterDoc::~CPolymeterDoc()
{
}

void CPolymeterDoc::InitChannelArray()
{
	CChannel	chanDefault(true);	// init to default values
	for (int iChan = 0; iChan < MIDI_CHANNELS; iChan++)	// for each channel
		m_arrChannel[iChan] = chanDefault;	// set to default values
}

void CPolymeterDoc::ReadProperties(LPCTSTR pszPath)
{
	CIniFile	fIni(pszPath, false);	// reading
	fIni.Read();	// read INI file
	CString	sFileID;
	fIni.Get(RK_FILE_ID, sFileID);
	if (sFileID != FILE_ID) {	// if unexpected file ID
		CString	msg;
		AfxFormatString1(msg, IDS_DOC_BAD_FORMAT, pszPath);
		AfxMessageBox(msg, MB_OK, IDS_DOC_BAD_FORMAT);
		AfxThrowUserException();	// fatal error
	}
	fIni.Get(RK_FILE_VERSION, m_nFileVersion);
	if (m_nFileVersion > FILE_VERSION) {	// if file is from a newer version
		CString	msg;
		AfxFormatString1(msg, IDS_DOC_NEWER_VERSION, pszPath);
		AfxMessageBox(msg, MB_OK, IDS_DOC_NEWER_VERSION);
	}
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		if (PT_##proptype == PT_ENUM) \
			ReadEnum(fIni, RK_MASTER, _T(#name), m_##name, itemname, items); \
		else \
			fIni.Get(RK_MASTER, _T(#name), m_##name);
	#include "MasterPropsDef.h"	// generate code to read master properties
	m_Seq.SetTempo(m_fTempo);
	m_Seq.SetTimeDivision(GetTimeDivisionTicks());
	m_Seq.SetNoteOverlapMode(m_iNoteOverlap != CMasterProps::NOTE_OVERLAP_Allow);
	m_Seq.SetMeter(m_nMeter);
	m_Seq.SetPosition(m_nStartPos);
	m_Seq.SetLoopRange(CLoopRange(m_nLoopFrom, m_nLoopTo));
	m_nSongPos = m_nStartPos;	// also set our cached song position
	int	nTracks = 0;
	fIni.Get(RK_TRACK_COUNT, nTracks);
	ASSERT(!GetTrackCount());	// track array should be empty for proper initialization
	m_Seq.SetTrackCount(nTracks);
	LPCTSTR	pszStepKey;
	if (!m_nFileVersion)	// if legacy format
		pszStepKey = _T("Event");	// use legacy step key
	else	// current format
		pszStepKey = RK_TRACK_STEP;
	for (int iTrack = 0; iTrack < nTracks; iTrack++) {	// for each track
		CTrack	trk(true);	// initialize to defaults
		CString	sTrkID;
		sTrkID.Format(RK_TRACK_IDX, iTrack);
		fIni.GetUnicodeString(sTrkID, _T("Name"), trk.m_sName);
		#define TRACKDEF(proptype, type, prefix, name, defval, minval, maxval, itemopt, items) \
			if (PT_##proptype == PT_ENUM) \
				ReadEnum(fIni, sTrkID, _T(#name), trk.m_##prefix##name, itemopt, items); \
			else \
				fIni.Get(sTrkID, _T(#name), trk.m_##prefix##name);
		#define TRACKDEF_EXCLUDE_LENGTH	// exclude track length
		#define TRACKDEF_EXCLUDE_NAME	// exclude track name
		#include "TrackDef.h"		// generate code to read track properties
		trk.m_clrCustom = fIni.GetInt(sTrkID, RK_TRACK_COLOR, -1);
		int	nLength = fIni.GetInt(sTrkID, RK_TRACK_LENGTH, INIT_STEPS);
		trk.m_arrStep.SetSize(nLength);
		UINT	nReadSize = nLength;
		fIni.GetBinary(sTrkID, pszStepKey, trk.m_arrStep.GetData(), nReadSize);
		int	nDubs = fIni.GetInt(sTrkID, RK_TRACK_DUB_COUNT, 0);
		if (nDubs) {	// if track has dubs
			trk.m_arrDub.SetSize(nDubs);
			nReadSize = nDubs * sizeof(CDub);
			fIni.GetBinary(sTrkID, RK_TRACK_DUB_ARRAY, trk.m_arrDub.GetData(), nReadSize);
			if (!trk.m_arrDub[0].m_nTime) {	// if first dub time is zero
				ASSERT(m_nFileVersion < 10);	// should only occur in older versions
				trk.m_arrDub[0].m_nTime = MIN_DUB_TIME;	// update to allow negative dub times
			}
		}
		ReadTrackModulations(fIni, sTrkID, trk);	// read modulations if any
		m_Seq.SetTrack(iTrack, trk);
	}
	if (m_nFileVersion < FILE_VERSION)	// if older format
		ConvertLegacyFileFormat();
	m_arrChannel.Read(fIni);	// read channels
	m_arrPreset.Read(fIni, GetTrackCount());	// read presets
	m_arrPart.Read(fIni, RK_PART_SECTION);		// read parts
	fIni.Get(RK_STEP_VIEW, RK_STEP_ZOOM, m_fStepZoom);
	fIni.Get(RK_SONG_VIEW, RK_SONG_ZOOM, m_fSongZoom);
	CString	sViewTypeName;
	fIni.Get(RK_VIEW_TYPE, sViewTypeName);
	LPCTSTR	pszViewTypeName = sViewTypeName;
	int	nViewType = ARRAY_FIND(m_arrViewTypeName, pszViewTypeName);
	if (nViewType >= 0)
		m_nViewType = nViewType;
	CMidiEventArray	arrRecordEvent;
	fIni.GetArray(arrRecordEvent, REG_SETTINGS, RK_RECORD_EVENTS);	// read recorded events if any
	m_Seq.AttachRecordedEvents(arrRecordEvent);	// load recorded events into sequencer
	CMappingArray	arrMapping;
	arrMapping.Read(fIni);
	m_Seq.m_mapping.Attach(arrMapping);
}

void CPolymeterDoc::WriteProperties(LPCTSTR pszPath) const
{
	CIniFile	fIni(pszPath, true);	// writing
	fIni.Put(RK_FILE_ID, CString(FILE_ID));
	fIni.Put(RK_FILE_VERSION, FILE_VERSION);
	#define PROPDEF(group, subgroup, proptype, type, name, initval, minval, maxval, itemname, items) \
		if (PT_##proptype == PT_ENUM) \
			WriteEnum(fIni, RK_MASTER, _T(#name), m_##name, itemname, items); \
		else \
			fIni.Put(RK_MASTER, _T(#name), m_##name);
	#include "MasterPropsDef.h"	// generate code to write master properties
	int	nTracks = GetTrackCount();
	fIni.Put(RK_TRACK_COUNT, nTracks);
	for (int iTrack = 0; iTrack < nTracks; iTrack++) {	// for each track
		const CTrack&	trk = m_Seq.GetTrack(iTrack);
		CString	sTrkID;
		sTrkID.Format(RK_TRACK_IDX, iTrack);
		fIni.WriteUnicodeString(sTrkID, _T("Name"), trk.m_sName);
		#define TRACKDEF(proptype, type, prefix, name, defval, minval, maxval, itemopt, items) \
			if (PT_##proptype == PT_ENUM) \
				WriteEnum(fIni, sTrkID, _T(#name), trk.m_##prefix##name, itemopt, items); \
			else \
				fIni.Put(sTrkID, _T(#name), trk.m_##prefix##name);
		#define TRACKDEF_EXCLUDE_LENGTH	// exclude track length
		#define TRACKDEF_EXCLUDE_NAME	// exclude track name
		#include "TrackDef.h"		// generate code to write track properties
		if (static_cast<int>(trk.m_clrCustom) >= 0)	// if track color specified
			fIni.WriteInt(sTrkID, RK_TRACK_COLOR, trk.m_clrCustom);
		fIni.WriteInt(sTrkID, RK_TRACK_LENGTH, trk.GetLength());
		fIni.WriteBinary(sTrkID, RK_TRACK_STEP, trk.m_arrStep.GetData(), trk.GetUsedStepCount());
		DWORD	nDubs = trk.m_arrDub.GetSize();
		if (nDubs) {	// if track has dubs
			fIni.WriteInt(sTrkID, RK_TRACK_DUB_COUNT, nDubs);
			fIni.WriteBinary(sTrkID, RK_TRACK_DUB_ARRAY, trk.m_arrDub.GetData(), nDubs * sizeof(CDub));
		}
		if (trk.IsModulated())	// if track has modulations
			WriteTrackModulations(fIni, sTrkID, trk);	// write modulations
	}
	m_arrChannel.Write(fIni);	// write channels
	m_arrPreset.Write(fIni);	// write presets
	m_arrPart.Write(fIni, RK_PART_SECTION);	// write parts
	fIni.Put(RK_STEP_VIEW, RK_STEP_ZOOM, m_fStepZoom);
	fIni.Put(RK_SONG_VIEW, RK_SONG_ZOOM, m_fSongZoom);
	fIni.Put(RK_VIEW_TYPE, CString(m_arrViewTypeName[m_nViewType]));
	if (m_Seq.GetRecordedEventCount())	// if recorded events to write
		fIni.WriteArray(m_Seq.GetRecordedEvents(), REG_SETTINGS, RK_RECORD_EVENTS);
	if (m_Seq.m_mapping.GetArray().GetSize())	// if mappings to write
		m_Seq.m_mapping.GetArray().Write(fIni);
	fIni.Write();	// write INI file
}

__forceinline void CPolymeterDoc::ReadTrackModulations(CIniFile& fIni, CString sTrkID, CTrack& trk)
{
	CString	sModList(fIni.GetString(sTrkID, RK_TRACK_MOD_LIST));
	if (!sModList.IsEmpty()) {	// if any modulations
		CString	sMod;
		int	iToken = 0;
		while (!(sMod = sModList.Tokenize(_T(","), iToken)).IsEmpty()) {	// for each modulation token
			int	iDelim = sMod.Find(':');	// find operator
			if (iDelim >= 0) {	// if operator found
				int	iType = FindModulationTypeInternalName(sMod.Left(iDelim));
				if (iType >= 0) {	// if valid modulation type name found
					int	iModSource = _ttoi(sMod.Mid(iDelim + 1));	// get index of modulator track
					if (iModSource >= 0 && iModSource < m_Seq.GetTrackCount()) {	// if valid track index
						CModulation	mod(iType, iModSource);
						trk.m_arrModulator.Add(mod);	// add modulation to track
					}
				}
			}
		}
	}
}

__forceinline void CPolymeterDoc::WriteTrackModulations(CIniFile& fIni, CString sTrkID, const CTrack& trk) const
{
	CString	sModSource, sModList;
	int	nMods = trk.m_arrModulator.GetSize();
	for (int iMod = 0; iMod < nMods; iMod++) {	// for each modulation
		const CModulation&	mod = trk.m_arrModulator[iMod];
		int	iModSource = mod.m_iSource;
		if (iModSource >= 0) {	// if modulation type applies
			if (!sModList.IsEmpty())	// if not first token
				sModList += ',';	// append token separator
			sModSource.Format(_T(":%d"), iModSource);
			sModList += GetModulationTypeInternalName(mod.m_iType) + sModSource;
		}
	}
	fIni.WriteString(sTrkID, RK_TRACK_MOD_LIST, sModList);
}

void CPolymeterDoc::ConvertLegacyFileFormat()
{
	const STEP	nLegacyDefaultVelocity = 64;
	if (!m_nFileVersion) {	// if file version 0
		// steps were boolean; for non-zero steps, substitute default velocity
		int	nTracks = GetTrackCount();
		for (int iTrack = 0; iTrack < nTracks; iTrack++) {	// for each track
			int	nSteps = m_Seq.GetLength(iTrack);
			for (int iStep = 0; iStep < nSteps; iStep++) {	// for each step
				if (m_Seq.GetStep(iTrack, iStep))
					m_Seq.SetStep(iTrack, iStep, nLegacyDefaultVelocity);
			}
		}
	}
}

inline void CPolymeterDoc::ReadEnum(CIniFile& fIni, LPCTSTR pszSection, LPCTSTR pszKey, int& Value, const CProperties::OPTION_INFO *pOption, int nOptions)
{
	ASSERT(pOption != NULL);
	Value = CProperties::FindOption(fIni.GetString(pszSection, pszKey), pOption, nOptions);
	if (Value < 0)	// if string not found
		Value = 0;	// default to zero, not -1; avoids range errors downstream
	ASSERT(Value < nOptions);
}

inline void CPolymeterDoc::WriteEnum(CIniFile& fIni, LPCTSTR pszSection, LPCTSTR pszKey, const int& Value, const CProperties::OPTION_INFO *pOption, int nOptions) const
{
	UNREFERENCED_PARAMETER(nOptions);
	ASSERT(pOption != NULL);
	ASSERT(Value < nOptions);
	fIni.WriteString(pszSection, pszKey, Value >= 0 ? pOption[Value].pszName : _T(""));
}

template<class T>
inline void CPolymeterDoc::ReadEnum(CIniFile&, LPCTSTR, LPCTSTR, T&, const CProperties::OPTION_INFO*, int)
{
	ASSERT(0);	// should never be called
}

template<class T>
inline void CPolymeterDoc::WriteEnum(CIniFile&, LPCTSTR, LPCTSTR, const T&, const CProperties::OPTION_INFO*, int) const
{
	ASSERT(0);	// should never be called
}

void CPolymeterDoc::UpdateChannelEvents()
{
	CDWordArrayEx	arrMidiEvent;
	m_arrChannel.GetMidiEvents(arrMidiEvent);
	m_Seq.SetInitialMidiEvents(arrMidiEvent);
}

CString	GetMidiOutErrorString(MMRESULT mmrError)
{
	CString	s;
	LPTSTR	pBuf = s.GetBuffer(MAXERRORLENGTH);
	MMRESULT	mr = midiOutGetErrorText(mmrError, pBuf, MAXERRORLENGTH);
	s.ReleaseBuffer();
	if (MIDI_FAILED(mr))
		s.Format(_T("midiOutGetErrorText error %d"), mr);
	return(s);
}

void CPolymeterDoc::CMySequencer::OnMidiError(MMRESULT nResult)
{
	static const int nSeqErrorId[] = {
		#define SEQERRDEF(name) IDS_SEQERR_##name,
		#include "SequencerErrors.h"
	};
	CString	sError;
	if (nResult > CSequencer::SEQERR_FIRST && nResult < CSequencer::SEQERR_LAST) {
		int	iSeqErr = static_cast<int>(nResult) - (CSequencer::SEQERR_FIRST + 1);
		sError.LoadString(nSeqErrorId[iSeqErr]);
	} else {
		sError.Format(LDS(IDS_SEQ_MIDI_ERROR), nResult);
		sError += '\n' + GetMidiOutErrorString(nResult);
	}
	_putts(sError);
}
