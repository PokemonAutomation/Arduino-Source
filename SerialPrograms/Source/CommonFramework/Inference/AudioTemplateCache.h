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
    const AudioTemplate* get_nothrow(const QString& path, size_t sample_rate);
    const AudioTemplate& get_throw(const QString& path, size_t sample_rate);

    static AudioTemplateCache& instance();

private:
    ~AudioTemplateCache();
    AudioTemplateCache();

    const AudioTemplate* get_nothrow_internal(const QString& full_path, size_t sample_rate);


private:
    SpinLock m_lock;
    std::map<QString, AudioTemplate> m_cache;
};






}
#endif
