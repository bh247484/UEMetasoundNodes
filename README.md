# What is it?
Custom FM Synth and Wavefolder dsp implemented in an Unreal Engine Metasound node, all packaged in a Unreal Engine plugin. There's also a Metasound asset (the 3 voice fm synth described later in the README) in the `Content/` folder which implements the custom Metasound nodes.

# DSP
I created and implemented two different Metasound nodes, an FM Synth Tone Generator and a Nonlinear Wavefolder/Saturator. The `TestNode` is a simple hard coded gain node I used as a template for the other nodes.

Header declarations for the custom nodes are located in `Source/MetaNodes/Private/` and the cpp implementations are in `Source/MetaNodes/Public/`.

Look for the `Execute()` method implementation in each class to see the dsp algorithims applied to incoming audio buffers.

### FM Synthesis Tone Generator

The FM Synth node has a single carrier and single modulator oscillator. At moderate param values the tones stay harmonic and diatonically tuned but some enharmonicity is possible at extreme values. In other words there are sensible parameter constraints but classic wacky bell/metallic fm sounds are possible. Apply short envelope times and modulate params for some great enharmonic percussion.

**Params**
- Frequency
- Modulator Ratio
- Carrier Ratio
- Modulation Index (modAmp/modFreq)
- Modulation Envelope
- Amplitude Envelope

Special thanks for Eli Fieldsteel for his [lucid explanation](https://www.youtube.com/watch?v=UoXMUQIqFk4) of fm synth principles/parameters.

### Nonlinear Wavefolder with Saturation

The Wavefolder node adds harmonics to incoming audio by folding waveforms over themselves around floating point audio bounds (-1.0, 1.0). Particularly nice on bass sounds. There's also a `tan` derived saturation factor with a feedback component, for extra drive.

I expected the algorithm to produce some aliasing. That's typical of nonlinear dsp. To my surprise there was no audible aliasing, even at extreme frequencies. I [started implementing oversampling](https://github.com/bh247484/UEMetasoundNodes/commit/6f00e4312ec20b0bd94a8e50282031096d0504cb) but ultimately decided the extra cpu cycles weren't justifiable in this case.

**Params**
- Depth
- Frequency
- Feedback Drive

Special thanks to Jatin Chowdhury. His [CCRMA publication](https://ccrma.stanford.edu/~jatin/ComplexNonlinearities/Wavefolder.html) and [Medium Article](https://jatinchowdhury18.medium.com/complex-nonlinearities-episode-6-wavefolding-9529b5fe4102) pointed me in the right direction(s) here.

# UE Integration

With those Metasound nodes and their custom DSP complete I created a small demo project in Unreal to test them out.

I started by building a three voice synth with the FM Tone Generator. Each voice has an amplitude envelope, mod envelope, and the custom FM tone generator itself. The envelopes are controlled by a single AD env node upstream.
![synth-voices](https://github.com/bh247484/UEMetasoundNodes/assets/57693937/3f92775e-7b0e-45e0-bc88-996b0bc0b43f)

Then I applied my custom Wavefolder node to the bass voice.
![wavefolder](https://github.com/bh247484/UEMetasoundNodes/assets/57693937/d1108584-31f8-4951-8c82-7ffd7533b3a2)

And fed each voice a series of randomized midi notes.
![randomized-notes](https://github.com/bh247484/UEMetasoundNodes/assets/57693937/1d7adf6e-c17b-4896-8c37-71d3a23a4ccd)

Next I implemented some blueprint logic (combo of Anim Notifies and Event Dispatches) to trigger the FM Synth with each player character mesh's footsteps and placed a capsule asset over the center of the level, this suspended chrome marshmellow looking fellow.
![chrome-marshmellow](https://github.com/bh247484/UEMetasoundNodes/assets/57693937/5feab0e7-91f1-4c81-a305-ae34e89ff7f3)

I fed the distance between the player character and the chrome marshmellow to a Low Pass Filter that muffles the middle and upper synth voices. The nearer the player character the brighter the sound. All the player character blueprint logic looks like this.
![bp-third-person-logic](https://github.com/bh247484/UEMetasoundNodes/assets/57693937/505e5c28-b508-4e89-9e7e-2fdcf28c50f9)

I recorded a short piano improvisation to harmonically contextualize the synth sounds and break up the monotony of the footstep rhythm. The piano wav is triggered on `BeginPlay` in the level blueprint.

The final result looks and sounds something like this.

https://github.com/bh247484/UEMetasoundNodes/assets/57693937/fd559502-ec1f-4366-9dd6-f76a8906030e

Attn: Be sure to unmute the above video. Github mutes embeds by default.

The player can activate time dilation (slowmo) with a middle mouse click to vary their step frequency.

# Optimization Opportunities
There's an opportunity for optimization [here](https://github.com/bh247484/UEMetasoundNodes/blob/main/Source/MetaNodes/Private/FMGeneratorNode.cpp#L158) in the `modPhaseInc` calculation. If the contributing variables (parts of the mod osc frequency component) are not updated between buffer loops there's no reason to recalculate `modPhaseInc` every iteration. Those variable values could be cached and checked against incoming update param values. I'll implement that optimization in the future. For this first version I just wanted to standup something functional.

The carrier and modulator oscillators share much of the same logic. It makes sense to abstract out an Osc class. That'd simplify the FMGenerator class and separate concerns more cleanly. It'd be easier to expand the FM logic too if I want to introduce more carriers/modulators.
