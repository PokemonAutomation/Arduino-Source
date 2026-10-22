/*  Audio Pipeline - Audio File Loader Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  WAV-native file input: WavFile header parsing (PCM 8/16/32-bit and
 *  IEEE-float), probe_wav_native_format(), and AudioFileLoader's batch
 *  API (loadFullAudio still converts to a requested format; the sample
 *  rate must match that request). The test WAVs are generated on the fly
 *  in the system temp directory; no audio I/O is involved.
 *
 */

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>
#include <QAudioFormat>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "CommonFramework/AudioPipeline/IO/AudioFileLoader.h"
#include "CommonFramework/AudioPipeline/IO/AudioInputFile.h"
#include "AudioFileLoader_Tests.h"

namespace PokemonAutomation{

namespace{



//
//  Writes a minimal canonical WAV file (RIFF + fmt + data, little-endian)
//  with the given format tag, channels, rate, and bits per sample.
//
void write_wav_file(
    const std::string& path,
    uint16_t format_tag, uint16_t channels, uint32_t rate, uint16_t bits_per_sample,
    const std::vector<uint8_t>& data
){
    auto append16 = [&](std::vector<uint8_t>& out, uint16_t v){
        out.push_back((uint8_t)(v & 0xff));
        out.push_back((uint8_t)(v >> 8));
    };
    auto append32 = [&](std::vector<uint8_t>& out, uint32_t v){
        out.push_back((uint8_t)(v & 0xff));
        out.push_back((uint8_t)((v >> 8) & 0xff));
        out.push_back((uint8_t)((v >> 16) & 0xff));
        out.push_back((uint8_t)((v >> 24) & 0xff));
    };

    const uint32_t data_size = (uint32_t)data.size();
    const uint32_t byte_rate = rate * channels * (bits_per_sample / 8);
    const uint16_t block_align = (uint16_t)(channels * (bits_per_sample / 8));

    std::vector<uint8_t> out;
    out.insert(out.end(), {'R', 'I', 'F', 'F'});
    append32(out, 36 + data_size);
    out.insert(out.end(), {'W', 'A', 'V', 'E'});
    out.insert(out.end(), {'f', 'm', 't', ' '});
    append32(out, 16);
    append16(out, format_tag);
    append16(out, channels);
    append32(out, rate);
    append32(out, byte_rate);
    append16(out, block_align);
    append16(out, bits_per_sample);
    out.insert(out.end(), {'d', 'a', 't', 'a'});
    append32(out, data_size);
    out.insert(out.end(), data.begin(), data.end());

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write((const char*)out.data(), (std::streamsize)out.size());
}

std::string temp_wav_path(const std::string& name){
    return (std::filesystem::temp_directory_path() / ("pa_wav_test_" + name + ".wav")).string();
}



template <typename Type>
std::vector<uint8_t> to_bytes(const std::vector<Type>& samples){
    std::vector<uint8_t> out(samples.size() * sizeof(Type));
    std::memcpy(out.data(), samples.data(), out.size());
    return out;
}



//
//  PCM variants: the probe must report the file's actual rate, channel
//  count, and sample type.
//
class WavParsePcmFormatsTest : public UnitTest{
public:
    WavParsePcmFormatsTest()
        : UnitTest("AudioFile - WavParse_PcmFormats")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        {
            //  S16 44100 stereo.
            const std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
            const std::string path = temp_wav_path("s16_44100_2ch");
            write_wav_file(path, 1, 2, 44100, 16, data);
            QAudioFormat format;
            if (!probe_wav_native_format(path, format)){
                return "probe failed for the S16 44100 stereo wav.";
            }
            if (format.sampleRate() != 44100 || format.channelCount() != 2
                || format.sampleFormat() != QAudioFormat::SampleFormat::Int16){
                return "S16 wav parsed as: " + dump_audio_format(format) + ".";
            }
        }
        {
            //  S32 32000 mono.
            const std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
            const std::string path = temp_wav_path("s32_32000_1ch");
            write_wav_file(path, 1, 1, 32000, 32, data);
            QAudioFormat format;
            if (!probe_wav_native_format(path, format)){
                return "probe failed for the S32 32000 mono wav.";
            }
            if (format.sampleRate() != 32000 || format.channelCount() != 1
                || format.sampleFormat() != QAudioFormat::SampleFormat::Int32){
                return "S32 wav parsed as: " + dump_audio_format(format) + ".";
            }
        }
        {
            //  U8 8000 stereo.
            const std::vector<uint8_t> data = {0x00, 0x7f, 0x80, 0xff};
            const std::string path = temp_wav_path("u8_8000_2ch");
            write_wav_file(path, 1, 2, 8000, 8, data);
            QAudioFormat format;
            if (!probe_wav_native_format(path, format)){
                return "probe failed for the U8 8000 stereo wav.";
            }
            if (format.sampleRate() != 8000 || format.channelCount() != 2
                || format.sampleFormat() != QAudioFormat::SampleFormat::UInt8){
                return "U8 wav parsed as: " + dump_audio_format(format) + ".";
            }
        }
        return true;
    }
};



//
//  IEEE-float (format tag 3) WAVs: the probe must report a float format.
//
class WavParseIeeeFloatTest : public UnitTest{
public:
    WavParseIeeeFloatTest()
        : UnitTest("AudioFile - WavParse_IeeeFloat")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::vector<float> samples = {0.5f, -0.5f, 0.0f, 1.0f};
        const std::string path = temp_wav_path("f32_96000_1ch");
        write_wav_file(path, 3, 1, 96000, 32, to_bytes(samples));

        QAudioFormat format;
        if (!probe_wav_native_format(path, format)){
            return "probe failed for the IEEE-float 96000 mono wav.";
        }
        if (format.sampleRate() != 96000 || format.channelCount() != 1
            || format.sampleFormat() != QAudioFormat::SampleFormat::Float){
            return "float wav parsed as: " + dump_audio_format(format) + ".";
        }
        return true;
    }
};



//
//  Unsupported / invalid files: the probe must fail cleanly.
//
class WavParseRejectionsTest : public UnitTest{
public:
    WavParseRejectionsTest()
        : UnitTest("AudioFile - WavParse_Rejections")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        {
            //  24-bit PCM is not supported.
            const std::vector<uint8_t> data = {0x01, 0x02, 0x03};
            const std::string path = temp_wav_path("s24_48000_1ch");
            write_wav_file(path, 1, 1, 48000, 24, data);
            QAudioFormat format;
            if (probe_wav_native_format(path, format)){
                return "24-bit wav should be rejected but parsed as: " + dump_audio_format(format) + ".";
            }
        }
        {
            //  Not a wav at all.
            const std::string path = temp_wav_path("not_a_wav");
            {
                std::ofstream file(path, std::ios::binary | std::ios::trunc);
                file.write("hello world, definitely not a wav", 32);
            }
            QAudioFormat format;
            if (probe_wav_native_format(path, format)){
                return "non-wav bytes should be rejected.";
            }
        }
        {
            //  Missing file.
            QAudioFormat format;
            if (probe_wav_native_format("/nonexistent/pa_wav_test_missing.wav", format)){
                return "missing file should be rejected.";
            }
        }
        return true;
    }
};



//
//  loadFullAudio() with a matching target: the file's native bytes come
//  back unconverted.
//
class LoadFullNativePassthroughTest : public UnitTest{
public:
    LoadFullNativePassthroughTest()
        : UnitTest("AudioFile - LoadFull_NativePassthrough")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::vector<float> samples = {0.25f, -0.25f, 0.125f, -0.125f};
        const std::vector<uint8_t> data = to_bytes(samples);
        const std::string path = temp_wav_path("f32_48000_2ch_passthrough");
        write_wav_file(path, 3, 2, 48000, 32, data);

        QAudioFormat target;
        target.setSampleFormat(QAudioFormat::SampleFormat::Float);
        target.setSampleRate(48000);
        target.setChannelCount(2);

        AudioFileLoader loader(nullptr, path, target);
        const auto ret = loader.loadFullAudio();
        if (std::get<0>(ret) == nullptr){
            return "loadFullAudio() returned null for a matching-format float wav.";
        }
        if (std::get<1>(ret) != data.size()){
            return "Length mismatch: expected " + std::to_string(data.size())
                + " bytes, got " + std::to_string(std::get<1>(ret)) + ".";
        }
        if (std::memcmp(std::get<0>(ret), data.data(), data.size()) != 0){
            return "loadFullAudio() modified the native float data.";
        }
        return true;
    }
};



//
//  loadFullAudio() converts the sample type to the requested format
//  (float, normalized by the type's max value, as the pipeline defines it).
//
class LoadFullS16ToFloatTest : public UnitTest{
public:
    LoadFullS16ToFloatTest()
        : UnitTest("AudioFile - LoadFull_S16ToFloat")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::vector<int16_t> samples = {0, 32767, -32768, 16384, -16384, 8192, 24576, 1};
        const std::string path = temp_wav_path("s16_48000_2ch");
        write_wav_file(path, 1, 2, 48000, 16, to_bytes(samples));

        QAudioFormat target;
        target.setSampleFormat(QAudioFormat::SampleFormat::Float);
        target.setSampleRate(48000);
        target.setChannelCount(2);

        AudioFileLoader loader(nullptr, path, target);
        const auto ret = loader.loadFullAudio();
        if (std::get<0>(ret) == nullptr){
            return "loadFullAudio() returned null for the S16 wav.";
        }
        if (std::get<1>(ret) != samples.size() * sizeof(float)){
            return "Length mismatch: expected " + std::to_string(samples.size() * sizeof(float))
                + " bytes, got " + std::to_string(std::get<1>(ret)) + ".";
        }
        const float* out = (const float*)std::get<0>(ret);
        const float rcp = 1.0f / 32767.0f;
        for (size_t c = 0; c < samples.size(); c++){
            const float expected = (float)samples[c] * rcp;
            if (out[c] != expected){
                return "Sample " + std::to_string(c) + " = " + std::to_string(out[c])
                    + ", expected " + std::to_string(expected) + ".";
            }
        }
        return true;
    }
};



//
//  loadFullAudio() does not resample: a rate mismatch is rejected.
//
class LoadFullRateMismatchTest : public UnitTest{
public:
    LoadFullRateMismatchTest()
        : UnitTest("AudioFile - LoadFull_RateMismatch")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::vector<int16_t> samples = {0, 1, 0, 1};
        const std::string path = temp_wav_path("s16_44100_2ch");
        write_wav_file(path, 1, 2, 44100, 16, to_bytes(samples));

        QAudioFormat target;
        target.setSampleFormat(QAudioFormat::SampleFormat::Float);
        target.setSampleRate(48000);
        target.setChannelCount(2);

        AudioFileLoader loader(nullptr, path, target);
        const auto ret = loader.loadFullAudio();
        if (std::get<0>(ret) != nullptr){
            return "loadFullAudio() should reject a rate mismatch (44100 vs 48000).";
        }
        return true;
    }
};



}



void add_tests_AudioFileLoader(UnitTestDatabase& database){
    database.add<WavParsePcmFormatsTest>();
    database.add<WavParseIeeeFloatTest>();
    database.add<WavParseRejectionsTest>();
    database.add<LoadFullNativePassthroughTest>();
    database.add<LoadFullS16ToFloatTest>();
    database.add<LoadFullRateMismatchTest>();
}



}
