/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/AlignedVector.h"
#include "Common/Cpp/CpuId.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "TestProgramComputer.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_Default.h"
#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_x64_SSE42.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "Common/Cpp/AlignedVector.tpp"
#include "CommonFramework/AudioPipeline/TimeSampleWriter.h"
#include "CommonFramework/AudioPipeline/TimeSampleBuffer.h"
#include "CommonFramework/AudioPipeline/TimeSampleBufferReader.h"
#include "CommonFramework/AudioPipeline/AudioNormalization.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"


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



void print(const float* ptr, size_t len){
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

std::chrono::system_clock::time_point REFERENCE = std::chrono::system_clock::now();


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





void TestProgramComputer::program(ProgramEnvironment& env){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonLA;
    using namespace Pokemon;





#if 0
    uint8_t x[32] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    PartialWordAccess_x64_SSE41 access(2);

    access.store_no_past_end(x + 14, _mm_setr_epi8(100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115));

    for (size_t c = 0; c < 32; c++){
        cout << (int)x[c] << " ";
    }
    cout << endl;
#endif

#if 0
    QImage src("20220315-055335301551.jpg");
    while (true){
//        src.scaled(src.width(), 500);
        QImage dst(src.width(), 500, QImage::Format_ARGB32);
        scale_vertical_shrink<Uint8Scaler_x32_AVX2>(
            src.width(),
            (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
            (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
        );
    }
#endif

#if 0
    QImage src("20220315-055335301551.jpg");
    src = src.scaled(96, 54);

    size_t height = 33;

//    QImage dst(src.width(), height, QImage::Format_ARGB32);
//    cout << dst.width() << " x " << dst.height() << endl;

    {
        auto start = std::chrono::system_clock::now();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink_Default(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = std::chrono::system_clock::now();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = std::chrono::system_clock::now();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst = src.scaled(src.width(), height);
        }
        auto end = std::chrono::system_clock::now();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = std::chrono::system_clock::now();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink<Uint8Scaler_x16_SSE41>(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = std::chrono::system_clock::now();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = std::chrono::system_clock::now();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink<Uint8Scaler_x32_AVX2>(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = std::chrono::system_clock::now();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }

//    cout << dst.save("test.png") << endl;
#endif




#if 0
    cout << (int)scale_Default(255, 256) << endl;
    cout << (int)scale_Default(255, 128) << endl;

    {
        __m128i x = _mm_set1_epi8(255);
        __m128i scale = _mm_set1_epi16(256);
        x = scale_SSE2(x, scale);
        print_u8(x);
    }
    {
        __m128i x = _mm_set1_epi8(255);
        __m128i scale = _mm_set1_epi16(128);
        x = scale_SSE2(x, scale);
        print_u8(x);
    }




    uint8_t src[15] = {4, 8, 9, 7, 6, 4, 8, 5, 0, 7, 9, 3, 6, 2, 4};
    double dst[7];
    memset(dst, 0, sizeof(dst));

    size_t index_src = 0;
    size_t index_dst = 0;
    size_t stop_src = 15;
    size_t stop_dst = 7;
    double ratio = (double)stop_dst / stop_src;
    for (; index_src < stop_src; index_src++){
        double src_s = ratio * (double)index_src;
        double src_e = ratio * (double)(index_src + 1);
        size_t index = (size_t)src_s;
        if (src_e <= index + 1){
            dst[index] += src[index_src] * ratio;
        }else{
            double lower = index + 1 - src_s;
            double upper = src_e - (index + 1);
            dst[index + 0] += src[index_src] * lower;
            dst[index + 1] += src[index_src] * upper;
        }
    }

    for (size_t c = 0; c < stop_dst; c++){
        cout << dst[c] << ", ";
    }
    cout << endl;
#endif



#if 0
    QImage image("DetectionImages/20220322-210628266511-MountDetection.png");

    MountDetector detector(MountDetectorLogging::LOG_ONLY);
    detector.detect(image);
#endif


#if 0
    cout << CPU_CAPABILITY_CURRENT.OK_08_Nehalem << endl;
    cout << CPU_CAPABILITY_CURRENT.OK_13_Haswell << endl;
    cout << CPU_CAPABILITY_CURRENT.OK_17_Skylake << endl;
    cout << CPU_CAPABILITY_CURRENT.OK_19_IceLake << endl;
#endif


//    using WallClock = std::chrono::system_clock::time_point;

//    cout << (WallClock::min() < WallClock::max()) << endl;



//    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "asdf");
//    throw UserSetupError(env.logger(), "asdf");


#if 0
    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.20, 0.95, 0.60, 0.03}, 20);

    black_screen1.process_frame(QImage("screenshot-20220221-232325966395.png"), std::chrono::system_clock::now());
#endif

#if 0
    float data[25];
    for (int c = 0; c < 25; c++){
        data[c] = c;
    }
    print(data, 25);

//    auto start = std::chrono::system_clock::now();

    TimeSampleBuffer<float> buffer(10, std::chrono::seconds(10));

    buffer.push_samples(data +  0, 5, REFERENCE + std::chrono::milliseconds( 500));
    buffer.push_samples(data +  5, 5, REFERENCE + std::chrono::milliseconds(1000));
    buffer.push_samples(data + 10, 5, REFERENCE + std::chrono::milliseconds(1500));
    buffer.push_samples(data + 15, 5, REFERENCE + std::chrono::milliseconds(2200));
    buffer.push_samples(data + 20, 5, REFERENCE + std::chrono::milliseconds(2500));

    cout << buffer.dump() << endl;



    TimeSampleBufferReader reader(buffer);
//    reader.set_to_timestamp(REFERENCE + std::chrono::milliseconds(2610));
//    cout << "block = " << reader.m_current_block - REFERENCE << endl;
//    cout << "index = " << reader.m_current_index << endl;

#if 1
    float read[25];
    for (int c = 0; c < 25; c++){
        read[c] = -1;
    }
    reader.read_samples(read, 25, REFERENCE + std::chrono::milliseconds(2500));

    print(read, 25);
#endif
#endif
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



std::set<std::string> read_name(
    LoggerQt& logger,
    Language language,
    const QImage& screen, const ImageFloatBox& box
){
    if (language == Language::None){
        return {};
    }

    QImage image = extract_box(screen, box);
    OCR::filter_smart(image);

    std::set<std::string> ret;

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(logger, language, image);
    if (result.results.empty()){
//        dump_image(
//            logger, ProgramInfo(),
//            QString::fromStdString("NameOCR-" + language_data(language).code),
//            screen
//        );
    }else{
        for (const auto& item : result.results){
            ret.insert(item.second.token);
        }
    }
    return ret;
}








}
