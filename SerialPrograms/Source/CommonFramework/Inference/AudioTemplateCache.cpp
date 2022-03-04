/*  Audio Template Cache
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "AudioTemplateCache.h"

namespace PokemonAutomation{


AudioTemplateCache::~AudioTemplateCache(){}
AudioTemplateCache::AudioTemplateCache(){}


const AudioTemplate* AudioTemplateCache::get(const QString& path, size_t sample_rate){
    QString full_path = path + "-" + QString::number(sample_rate) + ".wav";
    SpinLockGuard lg(m_lock);
    auto iter = m_cache.find(full_path);
    if (iter != m_cache.end()){
        return &iter->second;
    }

    AudioTemplate audio_template = loadAudioTemplate(full_path, (int)sample_rate);
    if (audio_template.numFrequencies() == 0){
        return nullptr;
    }

    iter = m_cache.emplace(
        std::move(full_path),
        std::move(audio_template)
    ).first;

    return &iter->second;
}




}
