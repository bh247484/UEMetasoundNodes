#include "WaveFolderNode.h"
#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
// #include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"                         // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_MetaSoundWaveFolderNode"

namespace Metasound
{

    // Implementation - Operator.
    FWaveFolderOperator::FWaveFolderOperator(
        const FOperatorSettings& InSettings,
        const FAudioBufferReadRef& InAudioInput)
        : AudioInput(InAudioInput)
        , AudioOutput(FAudioBufferWriteRef::CreateNew(InSettings))
    {
        // Init logic if I need it.
    }

    // Helper function for constructing vertex interface
    const FVertexInterface& FWaveFolderOperator::GetVertexInterface()
    {
        using namespace WaveFolder;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamAudioInput))
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamAudio))
            )
        );

        return Interface;
    }

    // Retrieves necessary metadata about your node
    const FNodeClassMetadata& FWaveFolderOperator::GetNodeInfo()
    {
        auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
        {
            FVertexInterface NodeInterface = GetVertexInterface();

            FNodeClassMetadata Metadata
            {
                FNodeClassName { StandardNodes::Namespace, "Wave Folder Node", StandardNodes::AudioVariant },
                1, // Major Version
                0, // Minor Version
                METASOUND_LOCTEXT("WaveFolderNodeDisplayName", "Wave Folder Node"),
                METASOUND_LOCTEXT("WaveFolderNodeDesc", "A test gain node."),
                PluginAuthor,
                PluginNodeMissingPrompt,
                NodeInterface,
                { }, // Category Hierarchy
                { }, // Keywords for searching
                FNodeDisplayStyle{}
            };

            return Metadata;
        };

        static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
        return Metadata;
    }

    // Allows MetaSound graph to interact with your node's inputs
    FDataReferenceCollection FWaveFolderOperator::GetInputs() const
    {
        using namespace WaveFolder;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamAudioInput), FAudioBufferReadRef(AudioInput));

        return InputDataReferences;
    }

    // Allows MetaSound graph to interact with your node's outputs
    FDataReferenceCollection FWaveFolderOperator::GetOutputs() const
    {
        using namespace WaveFolder;

        FDataReferenceCollection OutputDataReferences;

        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamAudio), FAudioBufferReadRef(AudioOutput));

        return OutputDataReferences;
    }

    // Used to instantiate a new runtime instance of your node
    TUniquePtr<IOperator> FWaveFolderOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace WaveFolder;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FAudioBufferReadRef AudioIn = InputCollection.GetDataReadReferenceOrConstruct<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InParamAudioInput), InParams.OperatorSettings);
//        FFloatReadRef PitchShift = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamPitchShift), InParams.OperatorSettings);

        return MakeUnique<FWaveFolderOperator>(InParams.OperatorSettings, AudioIn);
    }

    // Primary node functionality
    void FWaveFolderOperator::Execute()
    {
        const float* InputAudio = AudioInput->GetData();
        float* OutputAudio = AudioOutput->GetData();
        const int NumFrames = AudioInput->Num();
        
        for (int i = 0; i < NumFrames; ++i) {
            OutputAudio[i] = InputAudio[i] * 0.25f;
        }
    }

    // Implementation - Facade.
    FWaveFolderNode::FWaveFolderNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FWaveFolderOperator>())
    {
    }

    // Register node
    METASOUND_REGISTER_NODE(FWaveFolderNode);
}

#undef LOCTEXT_NAMESPACE
