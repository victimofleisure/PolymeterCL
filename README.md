# PolymeterCL

This is a command line fork of the Polymeter MIDI Sequencer, with all user interface code removed. This Visual Studio 2012 project builds a Windows command line application which opens and plays a Polymeter (.plm) document. The application can also export the document to a standard MIDI file. Removing the UI greatly reduces the number of lines of code, which could facilitate porting the sequencer to other platforms. Be advised that this project still depends on MFC, and on the Windows API, and particularly on the MIDI streams API. The MFC dependencies include CArray, CMap, CString, CFile, and CStdioFile, and while these could in theory be replaced with standard library classes, this is a task for a standard library expert, which I'm surely not.




