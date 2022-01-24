(
s.boot;
MIDIClient.init;
MIDIClient.destinations;
MIDIIn.connectAll;
MIDIIn.connect(0, MIDIClient.sources.at(2));

NetAddr.langPort;
b = NetAddr.new("localhost", 12345);

OSCdef.new(
	\mode,
	{
		arg msg;
		msg[1].postln;
		b.sendMsg("/mode", msg[1]);
	},
	'/mode'
);

// Rerouting piano notes to OpenFrameworks code.
~noteon = MIDIFunc.noteOn({ |vel, num, chan, src|
	num.postln;
	b.sendMsg("/noteon", num, vel);
});

~noteoff = MIDIFunc.noteOff({ |vel, num, chan, src|
	b.sendMsg("/noteoff", num, vel);
});

~ccevent = MIDIFunc.cc({ |vel, num, chan, src|
	b.sendMsg("/cc", num, vel);
});

)

NetAddr.langPort;