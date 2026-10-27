/*  Audio Source (File)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_File_H
#define PokemonAutomation_AudioPipeline_AudioSource_File_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/AudioPipeline/AudioSourceDescriptor.h"
#include "CommonFramework/AudioPipeline/IO/AudioSource.h"
#include "CommonFramework/AudioPipeline/IO/AudioInputFile.h"

namespace PokemonAutomation{


class AudioSourceDescriptor_File : public AudioSourceDescriptor{
public:
    AudioSourceDescriptor_File()
        : AudioSourceDescriptor(AudioSourceType::File)
    {}
    AudioSourceDescriptor_File(std::string path)
        : AudioSourceDescriptor(AudioSourceType::File)
        , m_path(std::move(path))
    {}

public:
    // get the image source path
    std::string path() const;
    // set the image source path
    void set_path(std::string path);

    virtual bool should_reload() const override{ return true; }
    virtual bool operator==(const AudioSourceDescriptor& x) const override;
    virtual std::string display_name() const override{
        return "Play Audio File";
    }

    virtual void run_post_select() override;

    virtual JsonValue to_json() const override;
    virtual void load_json(const JsonValue& json) override;

    virtual std::unique_ptr<AudioSource> make_AudioSource(
        Logger& logger,
        const AudioInputChoice& format
    ) const override;


private:
    mutable SpinLock m_lock;
    std::string m_path;
};




class AudioSource_File : public AudioSource{
public:
    AudioSource_File(
        Logger& logger,
        const std::string& file
    );

    virtual void add_raw_listener(RawAudioStreamListener& listener) override;
    virtual void remove_raw_listener(RawAudioStreamListener& listener) override;


public:
    virtual AudioSampleFormat raw_sample_format() const override{
        return m_stream_format;
    }
    virtual size_t raw_sample_rate() const override;
    virtual size_t raw_channels() const override;

    virtual float volume_multiplier() const override;


private:
    AudioSampleFormat m_stream_format;
    std::unique_ptr<AudioInputFile> m_input_file;
};







}
#endif
