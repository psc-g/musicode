(
s.boot;
MIDIClient.init;
MIDIClient.destinations;
MIDIIn.connectAll;
MIDIIn.connect(0, MIDIClient.sources.at(2));
~midiLoop = Array.new(300);
~midiLoopt = Array.new(300);
~midiLoopChords = Array.new(300);

~out = MIDIOut.newByName("YAMAHA USB Device", "Port1").latency_(0.0);

~delayMultiplier = 1;

~mode = 1;


~listeningToPedal = 1;

~recording = 'none';

~playBass1 = true;
~playBass2 = true;
~playChords = true;
~doScribbles = false;
~pitchShift = 0;

b = NetAddr.new("127.0.0.1", 12345);

~firstRecord = 1;
~firstBeat = -1;
~lastBeat = 0;
~activeNotes = Array.newClear(128);
~velocities = Array.newClear(128);
~deduper = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.

~firstRecordt = 1;
~firstBeatt = -1;
~lastBeatt = 0;
~activeNotest = Array.newClear(128);
~velocitiest = Array.newClear(128);
~dedupert = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.

~firstChords = 1;
~firstChordBeat = -1;
~lastChords = 0;
~activeChords = Array.newClear(128);
~chordVelocities = Array.newClear(128);
~dedupeChords = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.

~noteon = MIDIFunc.noteOn({ |vel, num, chan, src|
	if(~recording == 'line1') {
		if(~velocities[num].isNil) {
			if(~firstBeat < 0) {
				~firstBeat = TempoClock.default.beats;
			};
			~activeNotes[num] = TempoClock.default.beats;
			~velocities[num] = vel;
			~deduper[num] = nil;
			b.sendMsg("/jellyleg", num, vel);
		}
	};
	if(~recording == 'line2') {
		if(~velocitiest[num].isNil) {
			if(~firstBeatt < 0) {
				~firstBeatt = TempoClock.default.beats;
			};
			~activeNotest[num] = TempoClock.default.beats;
			~velocitiest[num] = vel;
			~dedupert[num] = nil;
			b.sendMsg("/jellyleg", num, vel);			};
	};
	if(~recording == 'chords') {
		if(~chordVelocities[num].isNil) {
				if(~firstChordBeat < 0) {
					~firstChordBeat = TempoClock.default.beats;
				};
			~activeChords[num] = TempoClock.default.beats;
				~chordVelocities[num] = vel;
				~dedupeChords[num] = nil;
			};

	};
	if(~doScribbles) {
		b.sendMsg("/scribble", num, vel);

	};
	if(~recording == 'none') {
		b.sendMsg("/redShift", 0, 0, 0, 0);
	};
});

~noteoff = MIDIFunc.noteOff({ |vel, num, chan, src|
	if(~recording == 'line1') {
		if(~velocities[num].notNil && ~deduper[num].isNil) {
			~midiLoop.add([num, ~velocities[num], ~activeNotes[num] - ~firstBeat, TempoClock.default.beats - ~firstBeat]);
			~velocities[num] = nil;
			~deduper[num] = 1;
			if(TempoClock.default.beats > ~lastBeat && ~firstRecord > 0) {
				~lastBeat = TempoClock.default.beats - ~firstBeat;
			};
		};
	};
	if(~recording == 'line2') {
		if(~velocitiest[num].notNil && ~dedupert[num].isNil) {
			~midiLoopt.add([num, ~velocitiest[num], ~activeNotest[num] - ~firstBeatt, TempoClock.default.beats - ~firstBeatt]);
			~velocitiest[num] = nil;
			~dedupert[num] = 1;
			if(TempoClock.default.beats > ~lastBeatt && ~firstRecordt > 0) {
				~lastBeatt = TempoClock.default.beats - ~firstBeatt;
			};
		};
	};
	if(~recording == 'chords') {
		if(~chordVelocities[num].notNil && ~dedupeChords[num].isNil) {
			~midiLoopChords.add([num, ~chordVelocities[num], ~activeChords[num] - ~firstChordBeat, TempoClock.default.beats - ~firstChordBeat]);
			~chordVelocities[num] = nil;
			~dedupeChords[num] = 1;
			if(TempoClock.default.beats > ~lastChordBeat && ~firstChords > 0) {
				~lastChordBeat = TempoClock.default.beats - ~firstChordBeat;
			};
		};
	};
});

~ccevent = MIDIFunc.cc({ |vel, num, chan, src|
	if(vel > 0 && ~listeningToPedal == 1 && num == 67, {
		if(~recording == 'line1') {
			~lastBeat = TempoClock.default.beats - ~firstBeat;
			r = Routine({{
				~checkBeats = TempoClock.default.beats;
				~checkSeconds = TempoClock.default.seconds;
				~midiLoop.do{
					arg nn;
					if(~playBass1) {
						TempoClock.default.sched(nn[2], {
							~out.noteOn(0, nn[0] + ~pitchShift, nn[1]);
							b.sendMsg("/jellycycle1", 0, 0, 0, 0);
						});
						TempoClock.default.sched(nn[3], {
							~out.noteOff(0, nn[0] + ~pitchShift)
						});
					};
				};
				~lastBeat.wait;
				~firstBeat = TempoClock.default.beats;
			}.loop; }).play;
			b.sendMsg("/newjelly", 0, 0, 0, 0);
			~firstRecord = 0;
			~recording = 'none';
		};
		if(~recording == 'line2') {
			~lastBeatt = TempoClock.default.beats - ~firstBeatt;
			e = Routine({{
				~midiLoopt.do{
					arg nn;
					if(~playBass2) {
						TempoClock.default.sched(nn[2], {
							~out.noteOn(0, nn[0] + ~pitchShift, nn[1]);
							b.sendMsg("/jellycycle2", 0, 0, 0, 0);
							});
						TempoClock.default.sched(nn[3], {
							~out.noteOff(0, nn[0] + ~pitchShift);
						});
					};
				};
				~lastBeatt.wait;
				~firstBeatt = TempoClock.default.beats;
			}.loop; }).play;
			~firstRecordt = 0;
			~recording = 'none';
		};
		if(~recording == 'chords') {
			~lastChordBeat = TempoClock.default.beats - ~firstChordBeat;
			f = Routine({{
				~midiLoopChords.do{
					arg nn;
					if(~playChords) {
						TempoClock.default.sched(nn[2], {
							~out.noteOn(0, nn[0] + ~pitchShift, nn[1]);
							b.sendMsg("/chordcycle", 0, 0, 0, 0);
						});
						TempoClock.default.sched(nn[3], {
							~out.noteOff(0, nn[0] + ~pitchShift);
						});
					};
				};
				~lastChordBeat.wait;
				~firstChordBeat = TempoClock.default.beats;
			}.loop; }).play;
			~firstChords = 0;
			~recording = 'none';
		};
		if (~doScribbles) {
			b.sendMsg("/newscribble", 0, 0, 0, 0);
		};
		~listeningToPedal = 0;
	}, {
		if(vel == 0 && num == 67, {
			~listeningToPedal = 1;
		}, {  });
	});
});
)


(
r.stop;
e.stop;
f.stop;

~midiLoop = Array.new(300);
~firstBeat = -1;
~firstRecord = 1;
~lastBeat = 0;


~midiLoopt = Array.new(300);
~firstRecordt = 1;
~firstBeatt = -1;
~lastBeatt = 0;
~activeNotest = Array.newClear(128);
~velocitiest = Array.newClear(128);
~dedupert = Array.newClear(128);

~midiLoopChords = Array.new(300);
~firstChords = 1;
~firstChordBeat = -1;
~lastChords = 0;
~activeChords = Array.newClear(128);
~chordVelocities = Array.newClear(128);
~dedupeChords = Array.newClear(128);

~doScribbles = false;
~mode = 1;
~listeningToPedal = 1;
~recording = 'none';
~pitchShift = 0;
~playBass1 = true;
~playBass2 = true;
~playChords = true;
)


// LIVE CODING STARTS HERE!

~recording = 'none';
~doScribbles = false;
~playBass1 = false;
~playBass2 = false;
~playChords = false;