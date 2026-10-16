/*  Audio Pipeline - ChannelMixer Matrix Test Infra
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdint>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"

#ifndef PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_Matrix_TestInfra_H
#define PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_Matrix_TestInfra_H

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



//
//  Builds a channel mixer matrix from a pair of QAudioFormat channel configs and verifies
//  that it matches the expected matrix. A config of 0 means "no channel layout"
//  (channel count only, all positions unknown), which must fail with an empty matrix.
//
class MixerMatrixTest : public UnitTest{
public:
    MixerMatrixTest(
        const std::string& name,
        uint32_t in_config,
        uint32_t out_config,
        std::vector<float> expected
    );

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override;


private:
    uint32_t m_in_config;
    uint32_t m_out_config;
    std::vector<float> m_expected;
};



}
}
}
#endif
