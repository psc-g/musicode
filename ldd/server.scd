(
s.boot;

NetAddr.langPort;
b = NetAddr.new("localhost", 12345);

MIDIClient.init;
MIDIClient.destinations;
MIDIIn.connect(0, MIDIClient.sources.at(2));

SynthDef(\controlSines, { arg att = 0.1, rel = 2, lofreq = 1000, hifreq = 3000;
    var env, snd;
	var kFreqs, kAmp;
	kFreqs = Control.names(\freqs).kr([100, 200, 300, 400, 500]);
	kAmp = Control.names(\amp).kr(0.4);
    env = Env.perc(
		attackTime: att,
		releaseTime: rel,
		level: kAmp
	).kr(doneAction: 2);
    snd = SinOsc.ar(freq: kFreqs, mul: env);
	snd = LPF.ar(
		in: snd,
		freq: LFNoise2.kr(1).range(lofreq, hifreq)
	);
    snd = Splay.ar(snd);
    Out.ar(0, snd);
}).add;

~notes = Array.newClear(128);  // One slot per MIDI note, in global var notes.
~multiplier = 1.001;
~scale = Scale.chromatic(\et12);

~noteon = MIDIFunc.noteOn({ |veloc, num, chan, src|
	var freq;
	freq = ~scale.degreeToFreq(num - 21, 21.midicps, 0);
	if (num < 60, { ~notes[num] = Synth(\controlSines, [rel: 8]); }, { ~notes[num] = Synth(\controlSines); });
	~notes[num].setn(\freqs, [freq, freq * 2, freq * 3, freq * 4, freq * 5]);
	b.sendMsg("/noteon", num, veloc);
});

~noteoff = MIDIFunc.noteOff({ |veloc, num, chan, src|
	~notes[num].free;
	b.sendMsg("/noteoff", num, veloc);
});

~cc = MIDIFunc.cc({ | veloc, num, chan, src|
	b.sendMsg("/ccEvent", num, veloc, chan);
});

~setDetune = OSCFunc( { | msg, time, addr, port |
	var freq, detune, amp;
	freq = ~scale.degreeToFreq(msg[1] - 21, 21.midicps, 0);
	detune = 1.0 + msg[2];
	amp = 2.0 - detune;
	if (~notes[msg[1]].isNil, {}, {
		~notes[msg[1]].setn(\freqs, [freq, freq * 2 * detune, freq * 3 * detune, freq * 4 * detune, freq * 5 * detune]);
		~notes[msg[1]].setn(\amp, amp);
	});
}, '/detune');

~setDetuneChord = OSCFunc( { | msg, time, addr, port |
	var freq1, freq2, freq3, detune;
	freq1 = ~scale.degreeToFreq(msg[2] - 21, 21.midicps, 0);
	freq2 = ~scale.degreeToFreq(msg[3] - 21, 21.midicps, 0);
	freq3 = ~scale.degreeToFreq(msg[4] - 21, 21.midicps, 0);
	detune = 1.0 + msg[1];
	if (~notes[msg[2]].isNil, {}, {
		if (msg[2] >= 0) {
			~notes[msg[2]].setn(\freqs, [freq1, freq1 * 2 * detune, freq1 * 3 * detune, freq1 * 4 * detune, freq1 * 5 * detune]);
		};
	});
	if (~notes[msg[3]].isNil, {}, {
		if (msg[3] >= 0) {
			~notes[msg[3]].setn(\freqs, [freq2, freq2 * 2 * detune, freq2 * 3 * detune, freq2 * 4 * detune, freq2 * 5 * detune]);
		};
	});
	if (~notes[msg[4]].isNil, {}, {
		if (msg[4] >= 0) {
			~notes[msg[4]].setn(\freqs, [freq3, freq3 * 2 * detune, freq3 * 3 * detune, freq3 * 4 * detune, freq3 * 5 * detune]);
		};
	});
}, '/detuneChord');
)


~multipliier = 1.001;

Tuning.directory;
~scale = Scale.chromatic(\et12);
~scale = Scale.chromatic(\just);
~scale = Scale.chromatic(\pythagorean);
~scale = Scale.chromatic(\lu);
~scale = Scale.chromatic(\wcHarm);
~scale = Scale.chromatic(\reinhard);
~scale = Scale.chromatic(\perret);
~scale = Scale.chromatic(\catler);

~scale.degreeToFreq(48, 21.midicps, 0);


(
var seq, str;
seq = Pseq(Array.interpolation(100, 1.0, 1.2), 1);
str = seq.asStream;
Routine({ 100.do({ ~detune = str.next; 0.2.wait; }) }).repeat(1).play;
)

s.freeAll
s.stopAll