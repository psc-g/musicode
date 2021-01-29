s.boot;
NetAddr.langPort;
MIDIClient.init;
(

MIDIClient.init;
b = NetAddr.new("localhost", 12345);

MIDIIn.connect(0, MIDIClient.sources.at(2));
~out = MIDIOut.newByName("YAMAHA USB Device", "Port1").latency_(0.0);
//~out = MIDIOut.newByName("Arturia MiniLab mkII", "Arturia MiniLab mkII").latency_(0.0);

~midinoteon = MIDIFunc.noteOn({ |veloc, num, chan, src|
	b.sendMsg("/noteon", num, veloc);
});

~midinoteoff = MIDIFunc.noteOff({ |veloc, num, chan, src|
	b.sendMsg("/noteoff", num, veloc);
});

~oscnoteon = OSCFunc( { | msg, time, addr, port |
	~out.noteOn(16, msg[1], msg[2]);
}, '/playnote' );

~stopnote = OSCFunc( { | msg, time, addr, port |
	~out.noteOff(16, msg[1], msg[2]);
}, '/stopnote' );

~cc = MIDIFunc.cc({ |num, chan, src, args|
	b.sendMsg("/ccevent", num, chan, src, args);
});

)
