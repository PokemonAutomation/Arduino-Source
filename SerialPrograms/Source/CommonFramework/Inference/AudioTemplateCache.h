/*  Audio Template Cache
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioTemplateCache_H
#define PokemonAutomation_CommonFramework_AudioTemplateCache_H

#include <map>
#include "Common/Cpp/SpinLock.h"

class QString;

namespace PokemonAutomation{

class AudioTemplate;


class AudioTemplateCache{
public:
    const AudioTemplate* get(const QString& path, size_t sample_rate);

private:
    ~AudioTemplateCache();
    AudioTemplateCache();

private:
    SpinLock m_lock;
    std::map<QString, AudioTemplate> m_cache;
};



class AudioSpectrumCache{
public:

private:

};




}
#endif
