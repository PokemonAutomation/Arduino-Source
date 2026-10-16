/*  Audio Pipeline Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioFormat>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AudioPipeline.h"
#include "Kernels_ChannelMixer_Matrix_TestInfra.h"
#include "Kernels_AudioPipeline_RoundTrip_TestInfra.h"
#include "Kernels_ChannelMixer_TestInfra.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{




void add_tests(UnitTestDatabase& database){
    //  Raw -> Working -> Raw round-trip.
    for (size_t channels = 1; channels <= 4; channels++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<RoundTripTest>(SampleFormat::UINT8, channels, samples);
            database.add<RoundTripTest>(SampleFormat::SINT16, channels, samples);
            database.add<RoundTripTest>(SampleFormat::SINT32, channels, samples);
            database.add<RoundTripTest>(SampleFormat::FLOAT32, channels, samples);
        }
    }

    //  ChannelMixer with an identity weight matrix.
    for (size_t channels = 1; channels <= 4; channels++){
        database.add<MixerIdentityTest>(channels, 1000);
    }

    //  1 -> 2 -> 1 averaging round-trip.
    database.add<MixerRoundTripTest>(1000);

    //  Channel mixer matrix builder (industry-standard coefficients).
    using CC = QAudioFormat::ChannelConfig;
    database.add<MixerMatrixTest>(
        "MixerMatrix(Mono->Stereo)", CC::ChannelConfigMono, CC::ChannelConfigStereo,
        std::vector<float>{1.0f, 1.0f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->Mono)", CC::ChannelConfigStereo, CC::ChannelConfigMono,
        std::vector<float>{0.5f, 0.5f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(3.0->Stereo)", CC::ChannelConfig3Dot0, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.7f,
            0.0f, 1.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->3.0)", CC::ChannelConfigStereo, CC::ChannelConfig3Dot0,
        std::vector<float>{
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.35f, 0.35f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->Stereo)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.7f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->5.1)", CC::ChannelConfigStereo, CC::ChannelConfigSurround5Dot1,
        std::vector<float>{
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.35f, 0.35f,
            0.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->Mono)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigMono,
        std::vector<float>{0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(2.1->Stereo)", CC::ChannelConfig2Dot1, CC::ChannelConfigStereo,
        std::vector<float>{
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(7.1->5.1)", CC::ChannelConfigSurround7Dot1, CC::ChannelConfigSurround5Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->3.0)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfig3Dot0,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->2.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfig2Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.7f, 0.0f, 0.7f, 0.0f,
            0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.7f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f
        }
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->7.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigSurround7Dot1,
        std::vector<float>{
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f
        }
    );
    std::vector<float> identity_5_1(36, 0.0f);
    for (size_t c = 0; c < 6; c++){
        identity_5_1[c + 6 * c] = 1.0f;
    }
    database.add<MixerMatrixTest>(
        "MixerMatrix(5.1->5.1)", CC::ChannelConfigSurround5Dot1, CC::ChannelConfigSurround5Dot1,
        identity_5_1
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(UnknownIn->Stereo)", 0, CC::ChannelConfigStereo,
        std::vector<float>{}
    );
    database.add<MixerMatrixTest>(
        "MixerMatrix(Stereo->UnknownOut)", CC::ChannelConfigStereo, 0,
        std::vector<float>{}
    );
}




}
}
}
