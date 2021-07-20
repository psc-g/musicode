(
s.boot;
s.latency = 0.05;

NetAddr.langPort;
b = NetAddr.new("localhost", 12345);

MIDIClient.init;
MIDIClient.destinations;
MIDIIn.connectAll;
MIDIIn.connect(0, MIDIClient.sources.at(2));


~out = MIDIOut.newByName("YAMAHA USB Device", "Port1").latency_(0.0);

~kickBuf = nil;
~snareBuf = nil;
~hhBuf = nil;

~kick = SynthDef(\kick, {
    |out = 0, pan = 0, amp = 0.3|
    var body, bodyFreq, bodyAmp;
    var pop, popFreq, popAmp;
    var click, clickAmp;
    var snd;

    // body starts midrange, quickly drops down to low freqs, and trails off
    bodyFreq = EnvGen.ar(Env([261, 120, 51], [0.035, 0.08], curve: \exp));
    bodyAmp = EnvGen.ar(Env.linen(0.005, 0.1, 0.3), doneAction: 2);
    body = SinOsc.ar(bodyFreq) * bodyAmp;
    // pop sweeps over the midrange
    popFreq = XLine.kr(750, 261, 0.02);
    popAmp = EnvGen.ar(Env.linen(0.001, 0.02, 0.001)) * 0.15;
    pop = SinOsc.ar(popFreq) * popAmp;
    // click is spectrally rich, covering the high-freq range
    // you can use Formant, FM, noise, whatever
    clickAmp = EnvGen.ar(Env.perc(0.001, 0.01)) * 0.15;
    click = LPF.ar(Formant.ar(910, 4760, 2110), 3140) * clickAmp;

    snd = body + pop + click;
    snd = snd.tanh;

    Out.ar(out, Pan2.ar(snd, pan, amp));
}).add;

SynthDef(\snare, {
    |out = 0, pan = 0, amp = 0.3|
    var pop, popAmp, popFreq;
    var noise, noiseAmp;
    var snd;

    // pop makes a click coming from very high frequencies
    // slowing down a little and stopping in mid-to-low
    popFreq = EnvGen.ar(Env([3261, 410, 160], [0.005, 0.01], curve: \exp));
    popAmp = EnvGen.ar(Env.perc(0.001, 0.11)) * 0.7;
    pop = SinOsc.ar(popFreq) * popAmp;
    // bandpass-filtered white noise
    noiseAmp = EnvGen.ar(Env.perc(0.001, 0.15), doneAction: 2);
    noise = BPF.ar(WhiteNoise.ar, 810, 1.6) * noiseAmp;

    snd = (pop + noise) * 1.3;

    Out.ar(out, Pan2.ar(snd, pan, amp));
}).add;

SynthDef(\snare2, {
    |out = 0, pan = 0, amp = 0.3|
    var pop, popAmp, popFreq;
    var noise, noiseAmp;
    var snd;

    // pop makes a click coming from very high frequencies
    // slowing down a little and stopping in mid-to-low
    popFreq = EnvGen.ar(Env([3261, 410, 160], [0.005, 0.5], curve: \exp));
    popAmp = EnvGen.ar(Env.perc(0.001, 0.11)) * 0.7;
    pop = SinOsc.ar(popFreq) * popAmp;
    // bandpass-filtered white noise
    noiseAmp = EnvGen.ar(Env.perc(0.001, 0.15), doneAction: 2);
    noise = BPF.ar(WhiteNoise.ar, 810, 1.6) * noiseAmp;

    snd = (pop + noise) * 1.3;

    Out.ar(out, Pan2.ar(snd, pan, amp));
}).add;


SynthDef(\hihat, {
    |out = 0, pan = 0, amp = 0.3|
    var click, clickAmp;
    var noise, noiseAmp;
    var snd;

    // noise -> resonance -> expodec envelope
    noiseAmp = EnvGen.ar(Env.perc(0.001, 0.3, curve: -8), doneAction: 2);
    noise = Mix(BPF.ar(ClipNoise.ar, [4010, 4151], [0.15, 0.56], [1.0, 0.6])) * 0.7 * noiseAmp;

    snd = noise;

    Out.ar(out, Pan2.ar(snd, pan, amp));
}).add;

~listeningToPedal = 1;

~recording = 'drumBeat';
~loopCounts = 0;
~numLoops = 6;

~bassLoop = Array.new(300);
~firstRecord = 1;
~firstBeat = -1;
~lastBeat = 0;
~activeNotes = Array.newClear(128);
~velocities = Array.newClear(128);
~deduper = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.

~drumLoop = Array.new(300);
~firstRecordLoopOne = 1;
~activeNotesLoopOne = Array.newClear(128);
~velocitiesLoopOne = Array.newClear(128);
~deduperLoopOne = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.

~noteon = MIDIFunc.noteOn({ |vel, num, chan, src|
	b.sendMsg("/noteon", num, vel);
	if(~recording == 'bassline', {
		if(~velocities[num].isNil) {
			~activeNotes[num] = TempoClock.default.beats;
			~velocities[num] = vel;
			~deduper[num] = nil;
			b.sendMsg("/increaseBassline", 0, 0);
		}
	}, {
		if (~recording == 'endGroove' && ~listeningToPedal == 1) {
			if (~velocities[num].isNil) {
				if (num < 53) {
					if(~firstBeat < 0) {
						~firstBeat = TempoClock.default.beats;
					};
					~activeNotes[num] = TempoClock.default.beats;
					~velocities[num] = vel;
					~deduper[num] = nil;
					b.sendMsg("/increaseEndGroove", 0, 0);
				};
			};
		};
	});
});

~noteoff = MIDIFunc.noteOff({ |vel, num, chan, src|
	b.sendMsg("/noteoff", num, vel);
	if(~recording == 'bassline', {
		if(~velocities[num].notNil && ~deduper[num].isNil, {
			~bassLoop.add([num, ~velocities[num], ~activeNotes[num] - ~firstBeat, TempoClock.default.beats - ~firstBeat]);
			~velocities[num] = nil;
			~deduper[num] = 1;
		});
	}, {
		if (~recording == 'endGroove' && ~listeningToPedal == 1) {
			if(~velocities[num].notNil && ~deduper[num].isNil) {
				if (num < 53) {
					~bassLoop.add([num, ~velocities[num], ~activeNotes[num] - ~firstBeat, TempoClock.default.beats - ~firstBeat]);
					~velocities[num] = nil;
					~deduper[num] = 1;
				};
			};
		};
	});
});

~ccevent = MIDIFunc.cc({ |vel, num, chan, src|
	if(vel > 0 && ~listeningToPedal == 1 && num == 67, {
		~listeningToPedal = 0;
		if(~recording == 'bassline', {
			r = Routine({{
				~loopCounts = ~loopCounts + 1;
				~bassLoop.do{
					arg nn;
					TempoClock.default.sched(nn[2], {
						~out.noteOn(0, nn[0], nn[1]);
						b.sendMsg("/bassOn", num, vel);
					});
					TempoClock.default.sched(nn[3], {
						~out.noteOff(0, nn[0]);
						b.sendMsg("/bassOff", num, vel);
					});
				};
				~drumBeatDuration.wait;
			}.loop; }).play;
			~firstRecord = 0;
			~recording = 'waiting';
		}, {
			if(~recording == 'waiting', {
				~recording = 'endGroove';
				TempoClock.clear;
				~velocities = Array.newClear(128);
				~deduper = Array.newClear(128);  // Ridiculous, but SC doesn't always make sense to me.
				~bassLoop = Array.new(300);
				~firstBeat = -1;
				~firstRecord = 1;
				~lastBeat = 0;
				~drumLoop = Array.new(300);
				~firstRecordLoopOne = 1;
				~kickBuf = nil;
				~snareBuf = nil;
				~hhBuf = nil;
				~loopCounts = 0;
			}, {
				if (~recording == 'endGroove', {
					~lastBeat = TempoClock.default.beats - ~firstBeat;
					~basslineDuration = ~lastBeat;
					r = Routine({{
						~loopCounts = ~loopCounts + 1;
						b.sendMsg("/newNewGroove", 0, 0);
						if (~loopCounts < 4) {
							~bassLoop.do{
								arg nn;
								TempoClock.default.sched(nn[2], {
									~out.noteOn(0, nn[0], nn[1]);
									b.sendMsg("/increaseEndGroove", num, vel);
								});
								TempoClock.default.sched(nn[3], {
									~out.noteOff(0, nn[0]);
								});
							};
						};
						~basslineDuration.wait;
						if (~loopCounts > 3) {
							r.stop;
						};
					}.loop; }).play;
					~firstRecord = 0;
					~recording = 'waiting';
				}, {
					r.stop;
					e.stop;
					f.stop;
					~bassLoop = Array.new(300);
					~firstBeat = -1;
					~firstRecord = 1;
					~lastBeat = 0;
					~listeningToPedal = 1;
					~recording = 'none';
					b.sendMsg("/resetCube", 0, 0);
				});
			});
		});
	}, {
		if(vel == 0 && num == 67, {
			~listeningToPedal = 1;
		}, {  });
	});
});

~oscnoteon = OSCFunc( { | msg, time, addr, port |
	if(~recording == 'drumBeat') {
		if(~velocitiesLoopOne[msg[1]].isNil) {
			if(~firstBeat < 0) {
				~firstBeat = TempoClock.default.beats;
			};
			~activeNotesLoopOne[msg[1]] = TempoClock.default.beats;
			~velocitiesLoopOne[msg[1]] = msg[2];
			~deduperLoopOne[msg[1]] = nil;
			if (msg[1] == 21, {
				~kickBuf = nil;
				~kickBuf = Synth(\kick);
			}, {
				if (msg[1] == 107, {
					~snareBuf = nil;
					~snareBuf = Synth(\snare);
				}, {
					~hhBuf = nil;
					~hhBuf = Synth(\hihat);
				});
			});
		}
	};
}, '/playnote' );

~stopnote = OSCFunc( { | msg, time, addr, port |
	if(~recording == 'drumBeat') {
		if(~velocitiesLoopOne[msg[1]].notNil && ~deduperLoopOne[msg[1]].isNil) {
			~drumLoop.add([msg[1], ~velocitiesLoopOne[msg[1]], ~activeNotesLoopOne[msg[1]] - ~firstBeat, TempoClock.default.beats - ~firstBeat]);
			~velocitiesLoopOne[msg[1]] = nil;
			~deduperLoopOne[msg[1]] = 1;
			if(TempoClock.default.beats > ~lastBeat && ~firstRecordLoopOne > 0) {
				~lastBeat = TempoClock.default.beats - ~firstBeat;
			};
		};
	};
}, '/stopnote' );

~playdrumBeat = OSCFunc( { | msg, time, addr, port |
	if (~recording == 'drumBeat') {
		~recording = 'bassline';
		~lastBeat = TempoClock.default.beats - ~firstBeat;
		~drumBeatDuration = ~lastBeat;
		e = Routine({{
			~drumLoop.do{
				arg nn;
				if (nn[0] ==21, {
					TempoClock.default.sched(nn[2], {
						b.sendMsg("/kickOn", 0, 0);
						~kickBuf = nil;
						~kickBuf = Synth(\kick);
					});
					TempoClock.default.sched(nn[3], {
						b.sendMsg("/kickOff", 0, 0);
					});
				}, {
					if (nn[0] == 107, {
						TempoClock.default.sched(nn[2], {
							b.sendMsg("/snareOn", 0, 0);
							~snareBuf = nil;
							~snareBuf = Synth(\snare);
						});
						TempoClock.default.sched(nn[3], {
							b.sendMsg("/snareOff", 0, 0);
						});
					}, {
						if (nn[0] == 108, {
							TempoClock.default.sched(nn[2], {
								~hhBuf = nil;
								~hhBuf = Synth(\hihat);
							});
						}, {
							if (nn[0] == 106) {
								TempoClock.default.sched(nn[2], {
									b.sendMsg("/snare2On", 0, 0);
									~snareBuf = nil;
									~snareBuf = Synth(\snare2);
								});
								TempoClock.default.sched(nn[3], {
									b.sendMsg("/snare2Off", 0, 0);
								});
							};
						});
					});
				});
			};
			if (~loopCounts > (~numLoops - 1)) {
				r.stop;
				e.stop;
			};
			~lastBeat.wait;
			~firstBeat = TempoClock.default.beats;
		}.loop; }).play;
	};
}, '/playdrumBeat');

~reset = OSCFunc( { | msg, time, addr, port |
	r.stop;
	e.stop;
	~bassLoop = Array.new(300);
	~firstBeat = -1;
	~firstRecord = 1;
	~lastBeat = 0;
	~drumLoop = Array.new(300);
	~firstRecordLoopOne = 1;
	~kickBuf = nil;
	~snareBuf = nil;
	~hhBuf = nil;
	~listeningToPedal = 1;
	~recording = 'drumBeat';
	~loopCounts = 0;
}, '/reset');
)

(
r.stop;
e.stop;

~bassLoop = Array.new(300);
~firstBeat = -1;
~firstRecord = 1;
~lastBeat = 0;

~drumLoop = Array.new(300);
~firstRecordLoopOne = 1;

~kickBuf = nil;
~snareBuf = nil;
~hhBuf = nil;


~listeningToPedal = 1;
~recording = 'drumBeat';
)

~recording;
~recording = 'bassline';
~bassLoop = Array.new(300);
~bassLoop;
~recording = 'endGroove';
~loopCounts;

~bassLoop = Array.new(300);
NetAddr.langPort;
