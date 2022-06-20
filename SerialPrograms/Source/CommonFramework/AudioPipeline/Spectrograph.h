/*  Audio Device Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_Spectrograph_H
#define PokemonAutomation_AudioPipeline_Spectrograph_H

#include <stdint.h>
#include "Common/Cpp/AlignedVector.h"

class QImage;

namespace PokemonAutomation{


class Spectrograph{
public:
    Spectrograph(size_t buckets, size_t frames);
    ~Spectrograph();

    void push_spectrum(const uint32_t* spectrum);

    QImage to_image() const;


private:
    size_t m_buckets;
    size_t m_frames;
    size_t m_current_index;
    AlignedVector<uint32_t> m_buffer;
};


}
#endif
