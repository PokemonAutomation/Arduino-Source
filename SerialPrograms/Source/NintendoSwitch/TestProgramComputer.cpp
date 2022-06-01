/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <mutex>
#include <condition_variable>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/AlignedVector.h"
#include "Common/Cpp/CircularBuffer.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "TestProgramComputer.h"
#include "ClientSource/Libraries/Logging.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x4_Default.h"
#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "Common/Cpp/AlignedVector.tpp"
#include "CommonFramework/AudioPipeline/TimeSampleWriter.h"
#include "CommonFramework/AudioPipeline/TimeSampleBuffer.h"
#include "CommonFramework/AudioPipeline/TimeSampleBufferReader.h"
#include "CommonFramework/AudioPipeline/AudioNormalization.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "Common/Cpp/PeriodicScheduler.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "Kernels/Kernels_Alignment.h"
//#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/Kernels_x64_AVX2.h"
//#include "Kernels/Kernels_x64_AVX512.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.h"
#include "Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h"
#include "Kernels/SpikeConvolution/Kernels_SpikeConvolution.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "Kernels/AudioStreamConversion/AudioStreamConversion.h"
#include "Common/Cpp/StreamConverters.h"




#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;


TestProgramComputer_Descriptor::TestProgramComputer_Descriptor()
    : RunnableComputerProgramDescriptor(
        "Computer:TestProgram",
        "Computer", "Test Program (Computer)",
        "",
        "Test Program"
    )
{}
TestProgramComputer::TestProgramComputer(const TestProgramComputer_Descriptor& descriptor)
    : RunnableComputerProgramInstance(descriptor)
{
}

WallClock REFERENCE = current_time();


template <typename Type>
void print(const Type* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << ptr[c];
    }
    cout << "}" << endl;
}
void print_u8(const uint8_t* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << (unsigned)ptr[c];
    }
    cout << "}" << endl;
}
void print(uint64_t m){
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            cout << (m & 1);
            m >>= 1;
        }
        cout << endl;
    }
    cout << endl;
}
void print_8x8(uint64_t m){
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            cout << (m & 1);
            m >>= 1;
        }
        cout << " ";
    }
    cout << endl;
}
void print_8x64(__m512i m){
    for (int i = 0; i < 8; i++){
        print_8x8(((const uint64_t*)&m)[i]);
    }
    cout << endl;
}





using namespace Kernels;


enum class AudioStreamFormat{
    UINT8,
    SINT16,
    SINT32,
    FLOAT32,
};

size_t sample_size(AudioStreamFormat format){
    switch (format){
    case AudioStreamFormat::UINT8:
        return sizeof(uint8_t);
    case AudioStreamFormat::SINT16:
        return sizeof(int16_t);
    case AudioStreamFormat::SINT32:
        return sizeof(int32_t);
    case AudioStreamFormat::FLOAT32:
        return sizeof(float);
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioStreamFormat: " + std::to_string((size_t)format));
}


#if 0
class AudioStreamReader{
public:
    AudioStreamReader(size_t channels, AudioStreamFormat format);
    size_t frames_available() const;
    void push_bytes(const void* data, size_t bytes);
    void read_frames(float* data, uint64_t frame_offset, size_t frames);
    void pop_to(uint64_t frame_offset);

private:
    AudioStreamFormat m_format;
    size_t m_channels;
    size_t m_sample_size;
    size_t m_frame_size;
    StreamReader m_reader;
};

AudioStreamReader::AudioStreamReader(size_t channels, AudioStreamFormat format)
    : m_format(format)
    , m_channels(channels)
    , m_sample_size(sample_size(format))
    , m_frame_size(m_sample_size * channels)
    , m_reader(m_frame_size * 16384)
{}
size_t AudioStreamReader::frames_available() const{
    return m_reader.bytes_stored() / m_frame_size;
}
void AudioStreamReader::push_bytes(const void* data, size_t bytes){
    m_reader.push_back(data, bytes);
}
void AudioStreamReader::read_frames(float* data, uint64_t frame_offset, size_t frames){
    m_reader.read(data, frame_offset * m_frame_size, frames * m_frame_size);
}
void AudioStreamReader::pop_to(uint64_t frame_offset){
    m_reader.pop_to(frame_offset * m_frame_size);
}



class AudioBuffer{
public:

    size_t frames_available() const;
    size_t read_frames(float* data, size_t frames);
    size_t pop_frames(size_t frames);

private:
    size_t m_channels;
    size_t m_frame_size;
    StreamReader m_buffer;

};


class AudioStreamReader2 : public ConvertedStreamReader{
public:
    AudioStreamReader2(size_t channels, AudioStreamFormat format);


private:
    virtual void convert(void* object, const void* raw, size_t count) override;

private:
    AudioStreamFormat m_format;
    size_t m_channels;
    size_t m_sample_size;
    size_t m_frame_size;
};
AudioStreamReader2::AudioStreamReader2(size_t channels, AudioStreamFormat format)
    : ConvertedStreamReader(sample_size(format), sizeof(float), 16384)
    , m_format(format)
    , m_sample_size(sample_size(format))
    , m_frame_size(m_sample_size * channels)
{}
void AudioStreamReader2::convert(void* object, const void* raw, size_t count){
    switch (m_format){
    case AudioStreamFormat::UINT8:
        Kernels::AudioStreamConversion::convert_audio_uint8_to_float((float*)object, (const uint8_t*)raw, count * m_channels);
        return;
    case AudioStreamFormat::SINT16:
        Kernels::AudioStreamConversion::convert_audio_sint16_to_float((float*)object, (const int16_t*)raw, count * m_channels);
        return;
    case AudioStreamFormat::SINT32:
        Kernels::AudioStreamConversion::convert_audio_sint32_to_float((float*)object, (const int32_t*)raw, count * m_channels);
        return;
    case AudioStreamFormat::FLOAT32:
        memcpy(object, raw, count * m_frame_size);
        return;
    }
}
#endif



class AudioStreamReader : public MisalignedStreamConverter{
public:
    AudioStreamReader(size_t channels, AudioStreamFormat format);

private:
    virtual void convert(void* out, const void* in, size_t count) override;

private:
    AudioStreamFormat m_format;
    size_t m_channels;
    size_t m_sample_size;
    size_t m_frame_size;
};
AudioStreamReader::AudioStreamReader(size_t channels, AudioStreamFormat format)
    : MisalignedStreamConverter(sample_size(format), sizeof(float), 16384)
    , m_format(format)
    , m_channels(channels)
    , m_sample_size(sample_size(format))
    , m_frame_size(m_sample_size * channels)
{}
void AudioStreamReader::convert(void* out, const void* in, size_t count){
    switch (m_format){
    case AudioStreamFormat::UINT8:
        Kernels::AudioStreamConversion::convert_audio_uint8_to_float((float*)out, (const uint8_t*)in, count * m_channels);
        return;
    case AudioStreamFormat::SINT16:
        Kernels::AudioStreamConversion::convert_audio_sint16_to_float((float*)out, (const int16_t*)in, count * m_channels);
        return;
    case AudioStreamFormat::SINT32:
        Kernels::AudioStreamConversion::convert_audio_sint32_to_float((float*)out, (const int32_t*)in, count * m_channels);
        return;
    case AudioStreamFormat::FLOAT32:
        memcpy(out, in, count * m_frame_size);
        return;
    }
}





class AudioListener : public StreamListener{
public:
    AudioListener()
        : StreamListener(sizeof(float))
    {}
    virtual void on_objects(const void* data, size_t objects) override{
        const float* samples = (const float*)data;
        print(samples, objects);
    }
};








void TestProgramComputer::program(ProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonLA;
    using namespace Pokemon;

    int16_t in[4] = {1, 1, 2, -2};
    print_u8((uint8_t*)in, 8);



    AudioStreamReader reader(2, AudioStreamFormat::SINT16);
    AudioListener listener;
    reader += listener;


//    reader.push_bytes(in, 8);
    reader.push_bytes((char*)in + 0, 3);
    reader.push_bytes((char*)in + 3, 5);




//    AudioStreamReader2 reader(2, AudioStreamFormat::SINT16);



#if 0
    char buffer[17] = {};
    for (size_t c = 0; c < 16; c++){
        buffer[c] = '=';
    }

    StreamReader reader(8);
    cout << reader.dump() << endl;

    reader.push_back("asdf", 4);
    reader.pop_to(2);
    reader.push_back("qwerzx", 6);
    cout << reader.dump() << endl;

    reader.push_back("sdfg", 4);
    cout << reader.dump() << endl;

//    reader.read(buffer, 4, 7);
//    cout << buffer << endl;
//    cout << reader.dump() << endl;
#endif

#if 0
    float f[10];
    uint8_t i[10] = {};
    i[0] = 1;
    i[1] = 255;
    i[2] = 0;
    i[3] = 123;
    i[9] = 123;

    Kernels::AudioStreamConversion::convert_audio_uint8_to_float(f, i, 10);
    print(f, 10);
    memset(i, 0, sizeof(i));
    Kernels::AudioStreamConversion::convert_audio_float_to_uint8(i, f, 10);
    print_u8(i, 10);
#endif


#if 0
    union{
        float f32;
        int32_t i32;
    };
    f32 = 12582912.;
    cout << i32 << endl;
#endif


//    cout << _mm_cvt_ss2si() << endl;

#if 0
    CircularBuffer buffer(8);

    char data[21] = {};

    buffer.push_back("asdfqwer", 8);

    cout << buffer.pop_front(data, 4) << endl;
    cout << data << endl;

//    cout << buffer.dump() << endl;

    buffer.push_back("zxcvsdfg", 8);

    cout << buffer.pop_front(data, 20) << endl;
    cout << data << endl;
#endif


//    __m256 k0 = _mm256_set1_ps(-4.);
//    __m256 k1 = _mm256_set1_ps(-3.);
//    __m256 k2 = _mm256_set1_ps(-2.);
//    __m256 k3 = _mm256_set1_ps(-1.);






}





inline std::string dump8(uint8_t x){
    std::string str;
    for (size_t c = 0; c < 8; c++){
        str += ((x >> c) & 1) ? "1" : "0";
    }
    return str;
}


void print(const uint64_t* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << ptr[c];
    }
    cout << "}" << endl;
}







#if 0
struct Uint8Scaler_x1_Default{
    static constexpr size_t VECTOR_LENGTH = 1;
    using vtype = uint8_t;

    uint16_t vscale;

    Uint8Scaler_x1_Default(double scale)
        : vscale(scale * 256 + 0.5)
    {}
    PA_FORCE_INLINE uint8_t apply(uint8_t x) const{
        return (uint8_t)(x * vscale >> 8);
    }

    PA_FORCE_INLINE void mulset(uint8_t* dst, const uint8_t* src) const{
        dst[0] = apply(src[0]);
    }
    PA_FORCE_INLINE void muladd(uint8_t* dst, const uint8_t* src) const{
        dst[0] = std::min<uint16_t>(dst[0] + apply(src[0]), 255);
    }
    static PA_FORCE_INLINE void mulset2(
        const uint8_t* src,
        uint8_t* dst0, const Uint8Scaler_x1_Default& lower,
        uint8_t* dst1, const Uint8Scaler_x1_Default& upper
    ){
        uint8_t x = src[0];
        dst0[0] = std::min<uint16_t>(dst0[0] + lower.apply(x), 255);
        dst1[0] = upper.apply(x);
    }
};
struct Uint8Scaler_x16_SSE41{
    static constexpr size_t VECTOR_LENGTH = 16;
    using vtype = __m128i;

    vtype vscale;

    Uint8Scaler_x16_SSE41(double scale)
        : vscale(_mm_set1_epi16((uint16_t)(scale * 256)))
    {}
    PA_FORCE_INLINE vtype apply(vtype x) const{
        vtype L = _mm_slli_epi16(x, 8);
        vtype H = _mm_and_si128(x, _mm_set1_epi16(0xff00));
        L = _mm_mulhi_epu16(L, vscale);
        H = _mm_mulhi_epu16(H, vscale);
        H = _mm_slli_epi16(H, 8);
        return _mm_or_si128(L, H);
    }

    PA_FORCE_INLINE void mulset(uint8_t* ptr, const uint8_t* src) const{
        vtype x = _mm_loadu_si128((const vtype*)src);
        x = apply(x);
        _mm_storeu_si128((vtype*)ptr, x);
    }
    PA_FORCE_INLINE void mulset(uint8_t* ptr, const uint8_t* src, size_t length) const{
        PartialWordAccess_x64_SSE41 partial_access(length);
        vtype x = partial_access.load(src);
        x = apply(x);
        partial_access.store_no_past_end(ptr, x);
    }
    PA_FORCE_INLINE void muladd(uint8_t* ptr, const uint8_t* src) const{
        vtype x = _mm_loadu_si128((const vtype*)src);
        x = apply(x);
        x = _mm_adds_epu8(_mm_loadu_si128((const vtype*)ptr), x);
        _mm_storeu_si128((vtype*)ptr, x);
    }
    PA_FORCE_INLINE void muladd(uint8_t* ptr, const uint8_t* src, size_t length) const{
        PartialWordAccess_x64_SSE41 partial_access(length);
        vtype x = partial_access.load(src);
        x = apply(x);
        x = _mm_adds_epu8(_mm_loadu_si128((const vtype*)ptr), x);
        partial_access.store_no_past_end(ptr, x);
    }
    static PA_FORCE_INLINE void mulset2(
        const uint8_t* src,
        uint8_t* dst0, const Uint8Scaler_x16_SSE41& lower,
        uint8_t* dst1, const Uint8Scaler_x16_SSE41& upper
    ){
        vtype x = _mm_loadu_si128((const __m128i*)src);
        vtype L = lower.apply(x);
        vtype H = upper.apply(x);
        L = _mm_adds_epu8(_mm_loadu_si128((const vtype*)dst0), L);
        _mm_storeu_si128((vtype*)dst0, L);
        _mm_storeu_si128((vtype*)dst1, H);
    }
    static PA_FORCE_INLINE void mulset2(
        const uint8_t* src, size_t length,
        uint8_t* dst0, const Uint8Scaler_x16_SSE41& lower,
        uint8_t* dst1, const Uint8Scaler_x16_SSE41& upper
    ){
        PartialWordAccess_x64_SSE41 partial_access(length);
        vtype x = partial_access.load(src);
        vtype L = lower.apply(x);
        vtype H = upper.apply(x);
        L = _mm_adds_epu8(_mm_loadu_si128((const vtype*)dst0), L);
        partial_access.store_no_past_end(dst0, L);
        partial_access.store_no_past_end(dst1, H);
    }
};
struct Uint8Scaler_x32_AVX2{
    static constexpr size_t VECTOR_LENGTH = 32;
    using vtype = __m256i;

    vtype vscale;

    Uint8Scaler_x32_AVX2(double scale)
        : vscale(_mm256_set1_epi16((uint16_t)(scale * 256)))
    {}
    PA_FORCE_INLINE vtype apply(vtype x) const{
        vtype L = _mm256_slli_epi16(x, 8);
        vtype H = _mm256_and_si256(x, _mm256_set1_epi16(0xff00));
        L = _mm256_mulhi_epu16(L, vscale);
        H = _mm256_mulhi_epu16(H, vscale);
        H = _mm256_slli_epi16(H, 8);
        return _mm256_or_si256(L, H);
    }

    PA_FORCE_INLINE void mulset(uint8_t* ptr, const uint8_t* src) const{
        vtype x = _mm256_loadu_si256((const vtype*)src);
        x = apply(x);
        _mm256_storeu_si256((vtype*)ptr, x);
    }
    PA_FORCE_INLINE void mulset(uint8_t* ptr, const uint8_t* src, size_t length) const{
        PartialWordAccess32_x64_AVX2 partial_access(length / 4);
        vtype x = partial_access.load(src);
        x = apply(x);
        partial_access.store(ptr, x);
    }
    PA_FORCE_INLINE void muladd(uint8_t* ptr, const uint8_t* src) const{
        vtype x = _mm256_loadu_si256((const vtype*)src);
        x = apply(x);
        x = _mm256_adds_epu8(_mm256_loadu_si256((const vtype*)ptr), x);
        _mm256_storeu_si256((vtype*)ptr, x);
    }
    PA_FORCE_INLINE void muladd(uint8_t* ptr, const uint8_t* src, size_t length) const{
        PartialWordAccess32_x64_AVX2 partial_access(length / 4);
        vtype x = partial_access.load(src);
        x = apply(x);
        x = _mm256_adds_epu8(_mm256_loadu_si256((const vtype*)ptr), x);
        partial_access.store(ptr, x);
    }
    static PA_FORCE_INLINE void mulset2(
        const uint8_t* src,
        uint8_t* dst0, const Uint8Scaler_x32_AVX2& lower,
        uint8_t* dst1, const Uint8Scaler_x32_AVX2& upper
    ){
        vtype x = _mm256_loadu_si256((const vtype*)src);
        vtype L = lower.apply(x);
        vtype H = upper.apply(x);
        L = _mm256_adds_epu8(_mm256_loadu_si256((const vtype*)dst0), L);
        _mm256_storeu_si256((vtype*)dst0, L);
        _mm256_storeu_si256((vtype*)dst1, H);
    }
    static PA_FORCE_INLINE void mulset2(
        const uint8_t* src, size_t length,
        uint8_t* dst0, const Uint8Scaler_x32_AVX2& lower,
        uint8_t* dst1, const Uint8Scaler_x32_AVX2& upper
    ){
        PartialWordAccess32_x64_AVX2 partial_access(length / 4);
        vtype x = partial_access.load(src);
        vtype L = lower.apply(x);
        vtype H = upper.apply(x);
        L = _mm256_adds_epu8(_mm256_loadu_si256((const vtype*)dst0), L);
        partial_access.store(dst0, L);
        partial_access.store(dst1, H);
    }
};




void scale_vertical_shrink_Default(
    size_t width,
    const uint32_t* src, size_t src_bytes_per_row, size_t src_height,
    uint32_t* dst, size_t dst_bytes_per_row, size_t dst_height
){
    width *= 4;
    double ratio = (double)dst_height / src_height;
    Uint8Scaler_x1_Default scaler(ratio);

    const uint8_t* src_ptr = (uint8_t*)src;
    uint8_t* dst_ptr = (uint8_t*)dst;

    size_t uninitialized = 0;
    for (size_t src_index = 0; src_index < src_height; src_index++){
        double src_s = ratio * (double)src_index;
        double src_e = ratio * (double)(src_index + 1);

        size_t index0 = (size_t)src_s;
        size_t index1 = index0 + 1;

        if (index0 >= dst_height){
            break;
        }

        const uint8_t* src0 = src_ptr + src_index * src_bytes_per_row;
        uint8_t* dst0 = dst_ptr + index0 * dst_bytes_per_row;

        if (src_e <= (double)index1 || index1 >= dst_height){
            //  Source fits entirely in the current row.
            if (uninitialized <= index0){
                for (size_t c = 0; c < width; c++){
                    scaler.mulset(dst0 + c, src0 + c);
                }
                uninitialized = index1;
            }else{
                for (size_t c = 0; c < width; c++){
                    scaler.muladd(dst0 + c, src0 + c);
                }
            }
            continue;
        }else{
            //  Source is split across the current row and the next.
            uint8_t* dst1 = dst0 + dst_bytes_per_row;
            Uint8Scaler_x1_Default lower(index1 - src_s);
            Uint8Scaler_x1_Default upper(src_e - index1);
            for (size_t c = 0; c < width; c++){
                Uint8Scaler_x1_Default::mulset2(
                    src0 + c,
                    dst0 + c, lower,
                    dst1 + c, upper
                );
            }
            uninitialized = index1 + 1;
        }
    }
}

template <typename Uint8Scalar>
void scale_vertical_shrink(
    size_t width,
    const uint32_t* src, size_t src_bytes_per_row, size_t src_height,
    uint32_t* dst, size_t dst_bytes_per_row, size_t dst_height
){
    width *= 4;
    if (width < Uint8Scalar::VECTOR_LENGTH){
        scale_vertical_shrink_Default(
            width,
            src, src_bytes_per_row, src_height,
            dst, dst_bytes_per_row, dst_height
        );
        return;
    }
    double ratio = (double)dst_height / src_height;
    Uint8Scalar scaler(ratio);

    const uint8_t* src_ptr = (uint8_t*)src;
    uint8_t* dst_ptr = (uint8_t*)dst;

    size_t uninitialized = 0;
    for (size_t src_index = 0; src_index < src_height; src_index++){
        double src_s = ratio * (double)src_index;
        double src_e = ratio * (double)(src_index + 1);

        size_t index0 = (size_t)src_s;
        size_t index1 = index0 + 1;

        if (index0 >= dst_height){
            break;
        }

        const uint8_t* src0 = src_ptr + src_index * src_bytes_per_row;
        uint8_t* dst0 = dst_ptr + index0 * dst_bytes_per_row;

        size_t lc = width / Uint8Scalar::VECTOR_LENGTH;
        if (src_e <= (double)index1 || index1 >= dst_height){
            //  Source fits entirely in the current row.
            if (uninitialized <= index0){
                do{
                    scaler.mulset(dst0, src0);
                    src0 += Uint8Scalar::VECTOR_LENGTH;
                    dst0 += Uint8Scalar::VECTOR_LENGTH;
                }while (--lc);
                lc = width % Uint8Scalar::VECTOR_LENGTH;
                if (lc){
                    scaler.mulset(dst0, src0, lc);
                }
                uninitialized = index1;
            }else{
                do{
                    scaler.muladd(dst0, src0);
                    src0 += Uint8Scalar::VECTOR_LENGTH;
                    dst0 += Uint8Scalar::VECTOR_LENGTH;
                }while (--lc);
                lc = width % Uint8Scalar::VECTOR_LENGTH;
                if (lc){
                    scaler.muladd(dst0, src0, lc);
                }
            }
            continue;
        }else{
            //  Source is split across the current row and the next.
            uint8_t* dst1 = dst0 + dst_bytes_per_row;
            Uint8Scalar lower(index1 - src_s);
            Uint8Scalar upper(src_e - index1);
            do{
                Uint8Scalar::mulset2(
                    src0,
                    dst0, lower,
                    dst1, upper
                );
                src0 += Uint8Scalar::VECTOR_LENGTH;
                dst0 += Uint8Scalar::VECTOR_LENGTH;
                dst1 += Uint8Scalar::VECTOR_LENGTH;
            }while (--lc);
            lc = width % Uint8Scalar::VECTOR_LENGTH;
            if (lc){
                Uint8Scalar::mulset2(
                    src0, lc,
                    dst0, lower,
                    dst1, upper
                );
            }
            uninitialized = index1 + 1;
        }

    }
}
#endif









}
