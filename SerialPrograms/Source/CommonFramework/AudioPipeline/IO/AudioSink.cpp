/*  Audio Output Writer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <QtGlobal>
#include <QAudioSink>
using NativeAudioSink = QAudioSink;

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSink.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



// Slider bar volume: [0, 100], in log scale
// Volume value passed to AudioDisplayWidget (and the audio thread it manages): [0.f, 1.f], linear scale
float convertAudioVolumeFromSlider(double volume){
    volume = std::max(volume, 0.0);
    volume = std::min(volume, 1.0);
    // The slider bar value is in the log scale because log scale matches human sound
    // perception.
    float linearVolume = QAudio::convertVolume(
        (float)volume,
        QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale
    );
    return linearVolume;
}


const char* audio_error_to_str(QAudio::Error error){
    switch (error){
    case QAudio::NoError:
        return "NoError";
    case QAudio::OpenError:
        return "OpenError (device cannot be opened with the requested format)";
    case QAudio::IOError:
        return "IOError";
    case QAudio::UnderrunError:
        return "UnderrunError";
    case QAudio::FatalError:
        return "FatalError";
    default:
        return "UnknownError";
    }
}



// Convert a stream of interleaved float frames from one channel count and sample rate
// to another. This is used by `AudioOutputDevice` when the output device (e.g. headphones)
// cannot play the capture card's format directly. For example, capture cards typically
// give 48000Hz stereo, while Bluetooth headphones may only accept 44100Hz stereo (A2DP
// profile) or 16000Hz mono (hands-free profile), and many USB headsets only support one
// fixed sample rate.
//
// 1. Channel mapping is done first, per frame:
//    - Mono output: average all input channels.
//    - Mono input: copy it into the first two output channels (left and right).
//    - Otherwise: copy each input channel to the same output channel.
//    Any remaining output channels (e.g. surround channels) are filled with silence.
// 2. Sample rate conversion uses streaming linear interpolation between consecutive
//    frames. The fractional read position and the previous frame are kept across calls
//    so there are no clicks at buffer boundaries. There is no anti-aliasing filter, so
//    large downsampling ratios lose some quality, but this is only for listening to the
//    game, not for audio inference (which uses the unconverted input stream).
class AudioFrameConverter{
public:
    AudioFrameConverter(
        size_t input_channels, size_t input_rate,
        size_t output_channels, size_t output_rate
    )
        : m_input_channels(input_channels)
        , m_output_channels(output_channels)
        , m_step((double)input_rate / (double)output_rate)
        , m_position(0)
        , m_mapped(output_channels)
        , m_previous(output_channels, 0.0f)
    {}

    // Convert `frames` input frames in `data` and return a pointer to the output
    // frames. The number of output frames is written to `output_frames`. The returned
    // buffer is owned by this class and is valid until the next call.
    const float* convert(const float* data, size_t frames, size_t& output_frames){
        m_output.clear();
        for (size_t f = 0; f < frames; f++){
            map_channels(data + f * m_input_channels);

            //  Emit all output frames that fall between the previous input frame and
            //  this one. `m_position` is the read position relative to the previous frame.
            while (m_position < 1.0){
                float t = (float)m_position;
                for (size_t c = 0; c < m_output_channels; c++){
                    m_output.push_back(m_previous[c] + (m_mapped[c] - m_previous[c]) * t);
                }
                m_position += m_step;
            }
            m_position -= 1.0;
            m_previous.swap(m_mapped);
        }
        output_frames = m_output.size() / m_output_channels;
        return m_output.data();
    }

private:
    void map_channels(const float* in){
        if (m_output_channels == 1){
            float sum = 0;
            for (size_t c = 0; c < m_input_channels; c++){
                sum += in[c];
            }
            m_mapped[0] = sum / (float)m_input_channels;
            return;
        }
        for (size_t c = 0; c < m_output_channels; c++){
            if (m_input_channels == 1){
                m_mapped[c] = c < 2 ? in[0] : 0.0f;
            }else{
                m_mapped[c] = c < m_input_channels ? in[c] : 0.0f;
            }
        }
    }

private:
    size_t m_input_channels;
    size_t m_output_channels;
    double m_step;
    double m_position;
    std::vector<float> m_mapped;
    std::vector<float> m_previous;
    std::vector<float> m_output;
};



// Receive float frames in the input (capture card) layout, convert them to the output
// device's channel count and sample rate if needed, convert to the device's sample
// format and write them to the Qt audio sink.
class AudioOutputDevice : public AudioFloatStreamListener, private ObjectStreamListener{
public:
    AudioOutputDevice(
        Logger& logger,
        const NativeAudioInfo& device, const QAudioFormat& format,
        AudioSampleFormat sample_format,
        size_t input_channels, size_t input_steps_per_frame,
        std::unique_ptr<AudioFrameConverter> converter,
        double volume
    )
        : AudioFloatStreamListener(input_channels * input_steps_per_frame)
        , ObjectStreamListener((size_t)format.channelCount() * sample_size(sample_format))
        , m_logger(logger)
        , m_input_steps_per_frame(input_steps_per_frame)
        , m_converter(std::move(converter))
        , m_to_stream(sample_format, (size_t)format.channelCount())
        , m_sink(device, format)
        , m_io_device(nullptr)
    {
        m_sink.connect(
            &m_sink, &NativeAudioSink::stateChanged,
            &m_sink, [this](QAudio::State state){
                if (state == QAudio::State::StoppedState){
                    m_io_device = nullptr;
                    QAudio::Error error = m_sink.error();
                    if (error == QAudio::NoError){
                        m_logger.log("AudioOutputDevice has stopped.", COLOR_ORANGE);
                    }else{
                        m_logger.log(
                            std::string("AudioOutputDevice has stopped with error: ") + audio_error_to_str(error),
                            COLOR_RED
                        );
                    }
                }
            }
        );

        m_io_device = m_sink.start();
        QAudio::Error error = m_sink.error();
        if (m_io_device == nullptr || error != QAudio::NoError){
            m_logger.log(
                std::string("Unable to start audio output device: ") + audio_error_to_str(error),
                COLOR_RED
            );
        }
        m_sink.setVolume(convertAudioVolumeFromSlider(volume));
        m_to_stream.add_listener(*this);
    }
    ~AudioOutputDevice(){
        m_to_stream.remove_listener(*this);
    }

    void set_volume(double volume){
        auto scope_check = m_sanitizer.check_scope();
        double absolute = convertAudioVolumeFromSlider(volume);
        m_logger.log("Volume set to: Slider = " + tostr_default(volume) + " -> Absolute = " + tostr_default(absolute));
        m_sink.setVolume(absolute);
    }

    virtual void on_samples(const float* data, size_t frames) override{
        auto scope_check = m_sanitizer.check_scope();
        //  The input frames are contiguous, so a frame that groups several time steps
        //  (MONO_96000) can be treated as that many single-step frames.
        size_t steps = frames * m_input_steps_per_frame;
        if (!m_converter){
            m_to_stream.on_samples(data, steps);
            return;
        }
        size_t output_frames;
        const float* output = m_converter->convert(data, steps, output_frames);
        m_to_stream.on_samples(output, output_frames);
    }

private:
    virtual void on_objects(const void* data, size_t objects) override{
        auto scope_check = m_sanitizer.check_scope();
        if (m_io_device != nullptr){
            m_io_device->write((const char*)data, objects * object_size);
        }
    }

private:
    Logger& m_logger;

    //  Number of time steps in each input frame. e.g. MONO_96000 groups
    //  2 mono samples into each frame.
    size_t m_input_steps_per_frame;

    //  Null if the device plays the input format directly.
    std::unique_ptr<AudioFrameConverter> m_converter;

    AudioFloatToStream m_to_stream;
    NativeAudioSink m_sink;
    QIODevice* m_io_device;
    LifetimeSanitizer m_sanitizer;
};


AudioSink::~AudioSink(){}

AudioSink::AudioSink(
    Logger& logger,
    const AudioDeviceInfo& device,
    AudioChannelFormat format,
    double volume
){
    switch (format){
    case AudioChannelFormat::MONO_48000:
        m_sample_rate = 48000;
        m_channels = 1;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::DUAL_44100:
        m_sample_rate = 44100;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::DUAL_48000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        break;
    case AudioChannelFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_sample_rate = 96000;
        m_channels = 1;
        m_multiplier = 2;
        break;
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Interleaved format not allowed for audio output."
        );
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid AudioFormat: " + std::to_string((size_t)format)
        );
    }

    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();
    logger.log("AudioOutputDevice(): Native: " + dump_audio_format(native_format));

    //  First try to play the input format as-is: the device's preferred sample format
    //  with the input's channel count and sample rate.
    QAudioFormat target_format = native_format;
    set_format(target_format, format);
    AudioSampleFormat sample_format = get_sample_format(target_format);
    if (sample_format == AudioSampleFormat::INVALID){
        sample_format = AudioSampleFormat::FLOAT32;
        set_sample_format_to_float(target_format);
    }
    logger.log("AudioOutputDevice(): Target: " + dump_audio_format(target_format));

    std::unique_ptr<AudioFrameConverter> converter;
    if (!native_info.isFormatSupported(target_format)){
        //  The device cannot play the input format. This is common with headphones,
        //  e.g. Bluetooth headphones that only accept 44100Hz, or 16000Hz mono when in
        //  hands-free mode. Fall back to the device's preferred format and convert the
        //  channel count and sample rate ourselves.
        target_format = native_format;
        sample_format = get_sample_format(target_format);
        if (sample_format == AudioSampleFormat::INVALID){
            sample_format = AudioSampleFormat::FLOAT32;
            set_sample_format_to_float(target_format);
        }
        if (target_format.channelCount() <= 0 || target_format.sampleRate() <= 0 ||
            !native_info.isFormatSupported(target_format)
        ){
            logger.log(
                "Audio output device does not support the requested audio format or its own preferred format. "
                "Audio output is disabled. Target: " + dump_audio_format(target_format),
                COLOR_RED
            );
            return;
        }
        logger.log(
            "AudioOutputDevice(): Device does not support the input format. Converting from " +
            std::to_string(m_channels) + " channel(s) at " + std::to_string(m_sample_rate) + "Hz to " +
            std::to_string(target_format.channelCount()) + " channel(s) at " +
            std::to_string(target_format.sampleRate()) + "Hz.",
            COLOR_ORANGE
        );
        converter = std::make_unique<AudioFrameConverter>(
            m_channels, m_sample_rate,
            target_format.channelCount(), target_format.sampleRate()
        );
    }

    m_writer = std::make_unique<AudioOutputDevice>(
        logger,
        native_info, target_format,
        sample_format,
        m_channels, m_multiplier,
        std::move(converter),
        volume
    );
}

AudioFloatStreamListener* AudioSink::float_stream_listener(){
    auto scope_check = m_sanitizer.check_scope();
    return m_writer.get();
}
void AudioSink::set_volume(double volume){
    auto scope_check = m_sanitizer.check_scope();
    if (!m_writer){
        return;
    }
    m_writer->set_volume(volume);
}




}
