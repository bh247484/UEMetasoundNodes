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
        const FAudioBufferReadRef& InAudioInput,
        const FFloatReadRef& InGain,
        const FFloatReadRef& InFbDrive)
        : AudioInput(InAudioInput)
        , Gain(InGain)
        , FbDrive(InFbDrive)
        , SampleRate((float) InSettings.GetSampleRate())
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
                TInputDataVertexModel<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamAudioInput)),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamGain), -0.5f),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamFbDrive), 0.9f)
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
                METASOUND_LOCTEXT("WaveFolderNodeDesc", "A node to apply wavefolding and saturation to oscillators or other input sources."),
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
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamGain), FFloatReadRef(Gain));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamFbDrive), FFloatReadRef(FbDrive));

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
        FFloatReadRef Gain = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamGain), InParams.OperatorSettings);
        FFloatReadRef FbDrive = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamFbDrive), InParams.OperatorSettings);

        return MakeUnique<FWaveFolderOperator>(InParams.OperatorSettings, AudioIn, Gain, FbDrive);
    }

    // Primary node functionality
    void FWaveFolderOperator::Execute()
    {
        const float* InputAudio = AudioInput->GetData();
        float* OutputAudio = AudioOutput->GetData();
        const int NumFrames = AudioInput->Num();
        
        for (int i = 0; i < NumFrames; ++i) {
            float satFactor = Audio::FastTanh(InputAudio[i]) + *FbDrive * Audio::FastTanh(outputMinusOne);
            float output = satFactor + *Gain * FMath::Sin(UE_TWO_PI * InputAudio[i] * (SampleRate / 2) / SampleRate);
            
            OutputAudio[i] = output;
            outputMinusOne = output;
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
