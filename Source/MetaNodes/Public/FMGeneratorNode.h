// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundParamHelper.h"

namespace Metasound {
    // Appease compiler.
    namespace StandardNodes
    {
        const FName Namespace = "UE";
        const FName AudioVariant = "Audio";
    }
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_MetaSoundFMGenerator"

    // Vertex Names - define your node's inputs and outputs here
    namespace FMGenerator
    {
        METASOUND_PARAM(InParamFrequency, "Frequency", "Tone frequency.");
        METASOUND_PARAM(InParamMRatio, "Mod osc amount.", "Modulation Ratio.");
        METASOUND_PARAM(InParamCRatio, "Carrier osc amount.", "Carrier Ratio.");
        METASOUND_PARAM(InParamModIndex, "Modulation index (modAmp/modFreq).", "Modulation Index.");
        METASOUND_PARAM(InParamModEnv, "Envelope applied to modulation osc.", "Modulation Envelope.");
        METASOUND_PARAM(InParamAmpEnv, "Envelope applied to entire output.", "Amplitude Envelope.");

        METASOUND_PARAM(OutParamAudio, "Out", "Audio output.")
    }

#undef LOCTEXT_NAMESPACE

    // Operator Declaration.
    class FFMGeneratorOperator : public TExecutableOperator<FFMGeneratorOperator>
    {
    public:

        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FFMGeneratorOperator(const FOperatorSettings& InSettings,
            const FFloatReadRef& InFrequency,
            const FInt32ReadRef& InMRatio,
            const FInt32ReadRef& InCRatio,
            const FInt32ReadRef& InModIndex,
            const FFloatReadRef& InModEnv,
            const FAudioBufferReadRef& InAmpEnv);

        virtual FDataReferenceCollection GetInputs() const override;
        virtual FDataReferenceCollection GetOutputs() const override;
        void Execute();

    private:
        
        void IncrementPhase(float& phase, float increment);

        FAudioBufferWriteRef AudioOutput;
        FFloatReadRef Frequency;
        float currentFreq;
        float SampleRate = 48000.0f;
        
        // Carrier and Modulator phases.
        float carrPhase = 0.0f;
        float carrPhaseInc;
        float modPhase = 0.0f;
        float modPhaseInc;
        
        // FM Params.
        FInt32ReadRef CRatio;
        FInt32ReadRef MRatio;
        FInt32ReadRef ModIndex;
        FFloatReadRef ModEnv;
        
        // Amp Env.
        FAudioBufferReadRef AmpEnv;
        
    };

    // Facade Declaration.
    class FFMGeneratorNode : public FNodeFacade
    {
    public:
        // Constructor used by the Metasound Frontend.
        FFMGeneratorNode(const FNodeInitData& InitData);
    };
}
