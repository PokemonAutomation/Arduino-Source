/*  Audio Template Cache
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFileInfo>
#include <QString>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/Globals.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"
#include "AudioTemplateCache.h"


namespace PokemonAutomation{


AudioTemplateCache::~AudioTemplateCache(){}
AudioTemplateCache::AudioTemplateCache(){}

AudioTemplateCache& AudioTemplateCache::instance(){
    static AudioTemplateCache cache;
    return cache;
}


const AudioTemplate* AudioTemplateCache::get_nothrow_internal(const std::string& full_path_no_ext, size_t sample_rate){
    WriteSpinLock lg(m_lock);
    auto iter = m_cache.find(full_path_no_ext);
    if (iter != m_cache.end()){
        return &iter->second;
    }

    std::string full_path = full_path_no_ext + ".wav";
    if (!QFileInfo::exists(QString::fromStdString(full_path))){
        full_path = full_path_no_ext + ".mp3";
    }

    AudioTemplate audio_template = loadAudioTemplate(full_path, (int)sample_rate);
    if (audio_template.numFrequencies() == 0){
        return nullptr;
    }

    iter = m_cache.emplace(
        std::move(full_path_no_ext),
        std::move(audio_template)
    ).first;

    return &iter->second;
}



const AudioTemplate* AudioTemplateCache::get_nothrow(const std::string& path, size_t sample_rate){
    std::string full_path_no_ext = RESOURCE_PATH() + path + "-" + std::to_string(sample_rate);
    return get_nothrow_internal(full_path_no_ext, sample_rate);
}
const AudioTemplate& AudioTemplateCache::get_throw(const std::string& path, size_t sample_rate){
    std::string full_path_no_ext = RESOURCE_PATH() + path + "-" + std::to_string(sample_rate);
    const AudioTemplate* audio_template = get_nothrow_internal(full_path_no_ext, sample_rate);
    if (audio_template == nullptr){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Unable to open audio template file. (Sample Rate = " + std::to_string(sample_rate) + ")",
            full_path_no_ext + ".(wav or mp3)"
        );
    }
    return *audio_template;
}




}
