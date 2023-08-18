#include "FMGeneratorNode.h"
#include "MetasoundExecutableOperator.h"     // TExecutableOperator class
#include "MetasoundPrimitives.h"             // ReadRef and WriteRef descriptions for bool, int32, float, and string
#include "MetasoundNodeRegistrationMacro.h"  // METASOUND_LOCTEXT and METASOUND_REGISTER_NODE macros
// #include "MetasoundStandardNodesNames.h"     // StandardNodes namespace
#include "MetasoundFacade.h"                         // FNodeFacade class, eliminates the need for a fair amount of boilerplate code
#include "MetasoundParamHelper.h"            // METASOUND_PARAM and METASOUND_GET_PARAM family of macros
#include <math.h>

// Required for ensuring the node is supported by all languages in engine. Must be unique per MetaSound.
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_MetaSoundFMGenerator"

namespace Metasound
{

    // Implementation - Operator.
    FFMGeneratorOperator::FFMGeneratorOperator(
        const FOperatorSettings& InSettings,
        const FFloatReadRef& InFrequency,
        const FInt32ReadRef& InMRatio,
        const FInt32ReadRef& InCRatio,
        const FInt32ReadRef& InModIndex,
        const FFloatReadRef& InModEnv,
        const FFloatReadRef& InAmpEnv)
        : AudioOutput(FAudioBufferWriteRef::CreateNew(InSettings))
        , Frequency(InFrequency)
        , SampleRate((float) InSettings.GetSampleRate())
        , CRatio(InCRatio)
        , MRatio(InMRatio)
        , ModIndex(InModIndex)
        , ModEnv(InModEnv)
        , AmpEnv(InAmpEnv)
    {
    }

    // Helper function for constructing vertex interface
    const FVertexInterface& FFMGeneratorOperator::GetVertexInterface()
    {
        using namespace FMGenerator;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamFrequency), 440.0f),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamMRatio), 1),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamCRatio), 1),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamModIndex), 1),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamModEnv), 1.0f),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamAmpEnv), 1.0f)
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamAudio))
            )
        );

        return Interface;
    }

    // Retrieves necessary metadata about your node
    const FNodeClassMetadata& FFMGeneratorOperator::GetNodeInfo()
    {
        auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
        {
            FVertexInterface NodeInterface = GetVertexInterface();

            FNodeClassMetadata Metadata
            {
                FNodeClassName { StandardNodes::Namespace, "FM Generator Node", StandardNodes::AudioVariant },
                1, // Major Version
                0, // Minor Version
                METASOUND_LOCTEXT("FMGeneratorNodeDisplayName", "FM Node"),
                METASOUND_LOCTEXT("FMGeneratorNodeDesc", "A test gain node."),
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
    FDataReferenceCollection FFMGeneratorOperator::GetInputs() const
    {
        using namespace FMGenerator;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamFrequency), FFloatReadRef(Frequency));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamMRatio), FInt32ReadRef(MRatio));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamCRatio), FInt32ReadRef(CRatio));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamModIndex), FInt32ReadRef(ModIndex));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamModEnv), FFloatReadRef(ModEnv));
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamAmpEnv), FFloatReadRef(AmpEnv));

        return InputDataReferences;
    }

    // Allows MetaSound graph to interact with your node's outputs
    FDataReferenceCollection FFMGeneratorOperator::GetOutputs() const
    {
        using namespace FMGenerator;

        FDataReferenceCollection OutputDataReferences;

        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamAudio), FAudioBufferReadRef(AudioOutput));

        return OutputDataReferences;
    }

    // Used to instantiate a new runtime instance of your node
    TUniquePtr<IOperator> FFMGeneratorOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace FMGenerator;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FFloatReadRef Frequency = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamFrequency), InParams.OperatorSettings);
        FInt32ReadRef MRatio = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamMRatio), InParams.OperatorSettings);
        FInt32ReadRef CRatio = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamCRatio), InParams.OperatorSettings);
        FInt32ReadRef ModIndex = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamModIndex), InParams.OperatorSettings);
        FFloatReadRef ModEnv = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamModEnv), InParams.OperatorSettings);
        FFloatReadRef AmpEnv = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamAmpEnv), InParams.OperatorSettings);

        return MakeUnique<FFMGeneratorOperator>(InParams.OperatorSettings, Frequency, MRatio, CRatio, ModIndex, ModEnv, AmpEnv);
    }

    void FFMGeneratorOperator::Execute()
    {
        float* OutputAudio = AudioOutput->GetData();
        const int NumFrames = AudioOutput->Num();
        
        for (int i = 0; i < NumFrames; ++i) {
            // Short ciruit and save cycles if Amplitude Envelope at 0.
            if ( *AmpEnv == 0 ) {
                OutputAudio[i] = 0;
            }
            
            // Calculate modulator frequency and amount.
            float modAmp = *Frequency * *MRatio * *ModIndex * *ModEnv;
            float modFreq = modAmp * FMath::Sin(modPhase);
            
            // Write out to buffer.
            OutputAudio[i] = *AmpEnv * FMath::Sin(carrPhase);
            
            float carrierFreq = *Frequency * *CRatio + modFreq;
            
            // Update phase increments.
            carrPhaseInc = UE_TWO_PI * (carrierFreq / SampleRate);
            modPhaseInc = UE_TWO_PI * ((*Frequency * *MRatio) / SampleRate);
            
            // Increment modulator phase.
            IncrementPhase(modPhase, modPhaseInc);
            // Increment carrier phase.
            IncrementPhase(carrPhase, carrPhaseInc);
        }
    }

    void FFMGeneratorOperator::IncrementPhase(float& phase, float increment)
    {
        float nextPhase = phase + increment;
        if (nextPhase > UE_TWO_PI) {
            phase = nextPhase - UE_TWO_PI;
        } else {
            phase = nextPhase;
        }
    }

    // Implementation - Facade.
    FFMGeneratorNode::FFMGeneratorNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FFMGeneratorOperator>())
    {
    }

    // Register node
    METASOUND_REGISTER_NODE(FFMGeneratorNode);
}

#undef LOCTEXT_NAMESPACE
