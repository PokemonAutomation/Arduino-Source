/*  Audio Source Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSourceDescriptor_H
#define PokemonAutomation_AudioPipeline_AudioSourceDescriptor_H

#include <memory>
#include <string>
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"

namespace PokemonAutomation{


//
//  Describes which audio input to use (a file, or a device with an input
//  choice). The descriptor is plain state: no Qt objects, no threads, so it
//  can be passed across threads (shared_ptr) and stored in options. The
//  actual audio source (an AudioSource subclass: AudioInputFile /
//  AudioInputDevice) is built from the descriptor on the pair's event
//  thread, where the probing and the Qt objects must live.
//
//  A null shared_ptr<AudioSourceDescriptor> means "no audio input".
//
class AudioSourceDescriptor{
public:
    virtual ~AudioSourceDescriptor() = default;

    virtual bool operator==(const AudioSourceDescriptor& x) const = 0;
    virtual std::string display_name() const = 0;

    //  State accessors, used by the source factory. For any given
    //  descriptor exactly one of file_path() / input_device() is non-null;
    //  input_choice() is non-null only for device descriptors.
    virtual const std::string* file_path() const{ return nullptr; }
    virtual const AudioDeviceInfo* input_device() const{ return nullptr; }
    virtual const AudioInputChoice* input_choice() const{ return nullptr; }
};


//  Audio input from a file (.wav, .mp3, ...).
class AudioSourceDescriptor_File : public AudioSourceDescriptor{
public:
    AudioSourceDescriptor_File() = default;
    AudioSourceDescriptor_File(std::string path)
        : m_path(std::move(path))
    {}

    virtual bool operator==(const AudioSourceDescriptor& x) const override{
        const AudioSourceDescriptor_File* y = dynamic_cast<const AudioSourceDescriptor_File*>(&x);
        return y && m_path == y->m_path;
    }
    virtual std::string display_name() const override{
        return m_path;
    }

    virtual const std::string* file_path() const override{ return &m_path; }

private:
    std::string m_path;
};


//  Audio input from a capture device, in the given input choice.
class AudioSourceDescriptor_Device : public AudioSourceDescriptor{
public:
    AudioSourceDescriptor_Device() = default;
    AudioSourceDescriptor_Device(AudioDeviceInfo device, AudioInputChoice choice)
        : m_device(std::move(device))
        , m_choice(std::move(choice))
    {}

    virtual bool operator==(const AudioSourceDescriptor& x) const override{
        const AudioSourceDescriptor_Device* y = dynamic_cast<const AudioSourceDescriptor_Device*>(&x);
        return y && m_device == y->m_device && m_choice == y->m_choice;
    }
    virtual std::string display_name() const override{
        return m_device.display_name();
    }

    virtual const AudioDeviceInfo* input_device() const override{ return &m_device; }
    virtual const AudioInputChoice* input_choice() const override{ return &m_choice; }

private:
    AudioDeviceInfo m_device;
    AudioInputChoice m_choice;
};



}
#endif
