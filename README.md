# What is it?
Custom FM Synth and Wavefolder dsp implemented in an Unreal Engine Metasound node, all packaged in a Unreal Engine plugin. There's also a Metasound blueprint asset (the 3 voice fm synth described later in the README) in the `/Content` folder which implements the custom Metasound nodes.

# DSP
- Header delcarations in `/Source/MetaNodes/Private/`
- Cpp Implementations in `/Source/MetaNodes/Public/`

I created and implemented two different Metasound nodes, an FM Synth Tone Generator and a Nonlinear Wavefolder/Saturator.  The `TestNode` is a simple hard coded gain node I used as a template for the other nodes.

Look for `Execute()` method in each class to see the dsp algorithims applied to incoming audio buffers.

### FM Synthesis Tone Generator

The FM Synth node has a single carrier and single modulator oscillator. At moderate param values the tones stay harmonic and diatonically tuned but some enharmonicity is possible at extreme values. In other words there are sensible parameter constraints but classic wacky bell/metallic fm sounds are possible. Apply short envelope times and modulate params for some great enharmonic percussion.

There's an opportunity for optimization in the `modPhaseInc` calculation. If the contributing variables (parts of the mod osc frequency component) are not updated between buffer loops there's no reason recalculate the `modPhaseInc`. Those variable values could be cached and checked against incoming update param values. I'll implement that optimization in the future. For this first version I just wanted to standup something functional.

Params
- Frequency
- Modulator Ratio
- Carrier Ratio
- Modulation Index (modAmp/modFreq)
- Modulation Envelope
- Amplitude Envelope

Special thanks for Eli Fieldsteel for his [lucid explanation](https://www.youtube.com/watch?v=UoXMUQIqFk4) of fm synth principles/parameters.

### Nonlinear Wavefolder with Saturation

The Wavefolder node adds harmonics to incoming audio by folding waveforms over themselves around floating point audio limits (-1.0, 1.0). Particularly nice on bass sounds. There's also a Tan derived saturation factor with a feedback component, for some extra drive.

I expected the algorithm to result in some aliasing. That's typical of nonlinear dsp. To my surprise there was no audible aliasing, even at extreme frequencies. I [started implementing oversampling]() to mitigate aliasing but decided the extra cpu cycles weren't justifiable in this case.

Params
- Depth
- Frequency
- Feedback Drive

Special thanks to Jatin Chowdhury. His [CCRMA publication](https://ccrma.stanford.edu/~jatin/ComplexNonlinearities/Wavefolder.html) and [Medium Article](https://jatinchowdhury18.medium.com/complex-nonlinearities-episode-6-wavefolding-9529b5fe4102) pointed me in the right direction(s) here.

# UE Integration

With those Metasound nodes and their custom DSP complete I created a small demo project in Unreal to test them out.

I started by building a three voice synth with the FM Tone Generator. Each voice has an amplitude envelope, mod envelope, and the custom FM tone generator itself. The envelopes are controlled by a single AD node upstream.
- insert photo

Then I applied my custom Wavefolder node to the bass voice.
- insert photo

And fed each voice a series of randomized midi notes.
- insert photo

Next I implemented some blueprint logic (combo of Anim Notifies and Event Dispatches) to trigger the FM Synth with each player character mesh's footsteps.
- insert photo

Finally I placed a capsule asset over the center of the level, this suspended chrome marshmellow looking fellow.
- insert photo

I fed the distance between the player character and the chrome marshmellow to a Low Pass Filter that muffles the middle and upper synth voices. The nearer the player character the brighter the sound.

The final result looks and sounds something like this.
- insert photo

There's time dilation (slomo) in there so the player can vary the step frequency.
