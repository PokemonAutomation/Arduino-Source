/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_Spectrograph_H
#define PokemonAutomation_AudioPipeline_Spectrograph_H

#include <stdint.h>
#include "Common/Cpp/AlignedVector.h"

namespace PokemonAutomation{

class ImageRGB32;


class Spectrograph{
public:
    Spectrograph(size_t buckets, size_t frames);
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
