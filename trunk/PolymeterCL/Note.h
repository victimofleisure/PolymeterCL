// NON-FUNCTIONAL STUB

#pragma once

enum {
	NOTES = 12,
};

class CNote {
public:
	CNote() {}
	CNote(int Note) {}
	operator int() const { return 0; }
	CNote	operator-=(CNote Note) { return 0; }
	bool	Parse(LPCTSTR Name) { return false; }
	bool	ParseMidi(LPCTSTR Name) { return false; }
};
