/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Holds a history of the most recent FFT spectrums as colors.
 *  This is used to render the spectrograph.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_Spectrograph_H
#define PokemonAutomation_AudioPipeline_Spectrograph_H

#include <stdint.h>
#include "Common/Cpp/Containers/AlignedVector.h"

namespace PokemonAutomation{

class ImageRGB32;


class Spectrograph{
public:
    Spectrograph(size_t buckets_per_spectrum, size_t frames);
    ~Spectrograph();

    void clear();

    void push_spectrum(const uint32_t* spectrum);

    ImageRGB32 to_image() const;


private:
    size_t m_buckets;
    size_t m_frames;
    size_t m_current_index;
    AlignedVector<uint32_t> m_buffer;
};


}
#endif
