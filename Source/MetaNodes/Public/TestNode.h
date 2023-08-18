// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundParamHelper.h"

namespace Metasound {
    // Appease compiler.
//    namespace StandardNodes
//    {
//        const FName Namespace = "UE";
//        const FName AudioVariant = "Audio";
//    }
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_MetaSoundTestNode"

    // Vertex Names - define your node's inputs and outputs here
    namespace TestNode
    {
        METASOUND_PARAM(InParamAudioInput, "In", "Audio input.");

        METASOUND_PARAM(OutParamAudio, "Out", "Audio output.")
    }

#undef LOCTEXT_NAMESPACE

    // Operator Declaration.
    class FTestNodeOperator : public TExecutableOperator<FTestNodeOperator>
    {
    public:

        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FTestNodeOperator(const FOperatorSettings& InSettings,
            const FAudioBufferReadRef& InAudioInput);

        virtual FDataReferenceCollection GetInputs() const override;
        virtual FDataReferenceCollection GetOutputs() const override;
        void Execute();

    private:

        // Inputs
        FAudioBufferReadRef AudioInput;

        // Outputs
        FAudioBufferWriteRef AudioOutput;
    };

    // Facade Declaration.
    class FTestNode : public FNodeFacade
    {
    public:
        // Constructor used by the Metasound Frontend.
        FTestNode(const FNodeInitData& InitData);
    };
}
