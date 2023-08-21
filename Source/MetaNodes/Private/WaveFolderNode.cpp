#include "WaveFolderNode.h"
#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
// #include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"                         // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
#include <algorithm>

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_MetaSoundWaveFolderNode"

namespace Metasound
{

    // Implementation - Operator.
    FWaveFolderOperator::FWaveFolderOperator(
        const FOperatorSettings& InSettings,
        const FAudioBufferReadRef& InAudioInput,
        const FFloatReadRef& InDepth,
        const FFloatReadRef& InFreq,
        const FFloatReadRef& InFbDrive)
        : AudioInput(InAudioInput)
        , Depth(InDepth)
        , Freq(InFreq)
        , FbDrive(InFbDrive)
        , SampleRate((float) InSettings.GetSampleRate())
        , AudioOutput(FAudioBufferWriteRef::CreateNew(InSettings))
    {
    }

    // Helper function for constructing vertex interface
    const FVertexInterface& FWaveFolderOperator::GetVertexInterface()
    {
        using namespace WaveFolder;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamAudioInput)),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamDepth), 0.5f),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamFreq), 0.5f),
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
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamDepth), FFloatReadRef(Depth));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamFreq), FFloatReadRef(Freq));
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
        FFloatReadRef Depth = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamDepth), InParams.OperatorSettings);
        FFloatReadRef Freq = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamFreq), InParams.OperatorSettings);
        FFloatReadRef FbDrive = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamFbDrive), InParams.OperatorSettings);

        return MakeUnique<FWaveFolderOperator>(InParams.OperatorSettings, AudioIn, Depth, Freq, FbDrive);
    }

    // Primary node functionality
    void FWaveFolderOperator::Execute()
    {
        const float* InputAudio = AudioInput->GetData();
        float* OutputAudio = AudioOutput->GetData();
        const int NumFrames = AudioInput->Num();
        const int OverSampFrames = NumFrames * 4;

        Audio::FAlignedFloatBuffer oversampInput;
        Audio::FAlignedFloatBuffer oversampOutput;
        oversampInput.AddZeroed(OverSampFrames);
        oversampOutput.SetNumUninitialized(OverSampFrames);

        // Copy input buffer into oversampled buffer and pad with zeroes.
        int overSampIndex = 0;
        for (int i = 0; i < NumFrames; ++i) {
            oversampInput[overSampIndex] = InputAudio[i]; // 0
            overSampIndex++;
            oversampInput[overSampIndex] = 0.0f;
            overSampIndex++;
            oversampInput[overSampIndex] = 0.0f;
            overSampIndex++;
            oversampInput[overSampIndex] = 0.0f;
            overSampIndex++;
        }

        // Lowpass to average out zero padding.
        for (int i = 0; i < OverSampFrames; ++i) {

        }

        // Apply wavefolding and saturation.
        for (int i = 0; i < OverSampFrames; ++i) {
            float fb = Audio::FastTanh(outputMinusOne);
            float satFactor = Audio::FastTanh(oversampInput[i]) + *FbDrive * fb;
            float output = satFactor - *Depth * FMath::Sin(UE_TWO_PI * oversampInput[i] * (std::max(*Freq, 0.00001f) * (SampleRate / 2)) / SampleRate);

            oversampOutput[i] = output / (1.0f + fb);
            outputMinusOne = output;
        }

        // Filter aliasing from oversampled buffer.
        for (int i = 0; i < OverSampFrames; ++i) {

        }

        // Downsample.
        // Write every 4th sample from oversampled buffer to output.
        overSampIndex = 0;
        for (int i = 0; i < NumFrames; ++i) {
            OutputAudio[i] = oversampOutput[overSampIndex];
            overSampIndex += 4;
        }
        
//        // -- Original --
//        // Apply wavefolding and saturation.
//        for (int i = 0; i < NumFrames; ++i) {
//            float fb = Audio::FastTanh(outputMinusOne);
//            float satFactor = Audio::FastTanh(InputAudio[i]) + *FbDrive * fb;
//            float output = satFactor - *Depth * FMath::Sin(UE_TWO_PI * InputAudio[i] * (std::max(*Freq, 0.00001f) * (SampleRate / 2)) / SampleRate);
//
//            OutputAudio[i] = output / (1.0f + fb);
//            outputMinusOne = output;
//        }
        
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
