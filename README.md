# PolymeterCL

This is a command line fork of the Polymeter MIDI Sequencer, with all user interface code removed. This project builds a Windows command line application which opens and plays a Polymeter (.plm) document. This greatly reduces the number of lines of code, which could facilitate porting the sequencer to other platforms. Be advised that this project still depends on MFC, and on the Windows API, and particularly on the MIDI streams API. The MFC dependencies include CArray, CMap, CString, CFile, and CStdioFile, and while these could in theory be replaced with standard library classes, this is a task for a standard library expert, which I'm surely not.




