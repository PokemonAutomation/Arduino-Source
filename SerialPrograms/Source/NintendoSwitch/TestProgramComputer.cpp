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
#include "Common/Cpp/CpuId.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
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
#include "CommonFramework/OCR/OCR_Filtering.h"
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

//#include "Kernels/Kernels_x64_AVX2.h"
//#include "Kernels/Kernels_x64_AVX512.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.h"

#include <QMessageBox>


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





#if 0
class ScheduledPrinter : public PeriodicRunner{
public:
    virtual ~ScheduledPrinter(){
        PeriodicRunner::cancel(nullptr);
        stop_thread();
    }
    bool add_event(std::function<void()>& event, std::chrono::milliseconds period){
        return PeriodicRunner::add_event(&event, period);
    }
    void remove_event(std::function<void()>& event){
        PeriodicRunner::remove_event(&event);
    }
    virtual void run(void* event) override{
        cout << current_time_to_str() << ": ";
        (*(std::function<void()>*)event)();
    }

protected:
    using PeriodicRunner::PeriodicRunner;
};
#endif



#if 0
template <typename Counter>
PA_FORCE_INLINE void count_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    Counter& counter0
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Counter::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        size_t lc = width / VECTOR_SIZE;
        do{
            counter0.process_full(in);
            in += VECTOR_SIZE;
        }while (--lc);
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            counter0.process_partial(in, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
    }while (--height);
}


class PixelCounter_RgbRange_Default{
public:
    static const size_t VECTOR_SIZE = 1;

public:
    PixelCounter_RgbRange_Default(uint32_t mins, uint32_t maxs)
        : m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
        , m_count(0)
    {}

    size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(const uint32_t* ptr){
        uint32_t pixel = ptr[0];
        uint64_t ret = 1;
        {
            uint32_t p = pixel & 0xff000000;
            ret &= p >= m_minA;
            ret &= p <= m_maxA;
        }
        {
            uint32_t p = pixel & 0x00ff0000;
            ret &= p >= m_minR;
            ret &= p <= m_maxR;
        }
        {
            uint32_t p = pixel & 0x0000ff00;
            ret &= p >= m_minG;
            ret &= p <= m_maxG;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            ret &= p >= m_minB;
            ret &= p <= m_maxB;
        }
        m_count += ret;
    }
    PA_FORCE_INLINE void process_partial(const uint32_t* ptr, size_t left){
        process_full(ptr);
    }


private:
    uint32_t m_minB;
    uint32_t m_maxB;
    uint32_t m_minG;
    uint32_t m_maxG;
    uint32_t m_minR;
    uint32_t m_maxR;
    uint32_t m_minA;
    uint32_t m_maxA;
    size_t m_count;
};
#endif


#if 0
size_t count_pixels_rgb_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t mins0, uint32_t maxs0
){

}



size_t count_pixels_rgb_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t mins0, uint32_t maxs0
){

}



size_t count_pixels_rgb_range(const ConstImageRef& image, uint32_t mins, uint32_t maxs){
    return count_pixels_rgb_range(image.data(), image.bytes_per_row(), image.width(), image.height(), mins, maxs);
}
#endif





void TestProgramComputer::program(ProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonLA;
    using namespace Pokemon;


#if 0
    QImage image("20220301-205136873076.jpg");
    QImage image0, image1, image2, image3;

    size_t count0, count1, count2, count3;
    filter4_rgb32_range(
        image,
        count0, image0, 0xff808080, 0xffffffff, Color(0xff000000), false,
        count1, image1, 0xff909090, 0xffffffff, Color(0xff000000), false,
        count2, image2, 0xffa0a0a0, 0xffffffff, Color(0xff000000), false,
        count3, image3, 0xffb0b0b0, 0xffffffff, Color(0xff000000), false
    );
    cout << count0 << endl;
    cout << count1 << endl;
    cout << count2 << endl;
    cout << count3 << endl;

    auto ret = filter_rgb32_range(
        image,
        {
            {0xff808080, 0xffffffff, Color(0xff000000), false},
            {0xff909090, 0xffffffff, Color(0xff000000), false},
            {0xffa0a0a0, 0xffffffff, Color(0xff000000), false},
            {0xffb0b0b0, 0xffffffff, Color(0xff000000), false},
        }
    );
    cout << ret[0].second << endl;
    cout << ret[1].second << endl;
    cout << ret[2].second << endl;
    cout << ret[3].second << endl;



    image.save("test.png");
#endif


//    throw InternalProgramError(nullptr, "asdf", "qwer");

//    cout << "asdfasdf" << endl;





#if 0
    std::function<void()> callback0 = []{ cout << "asdf" << endl; };
    std::function<void()> callback1 = []{ cout << "qwer" << endl; };

    CancellableHolder<ScheduledPrinter> runner(scope, env.inference_dispatcher());
    runner.add_event(callback0, std::chrono::seconds(2));
    runner.add_event(callback1, std::chrono::seconds(3));

    scope.wait_for(std::chrono::seconds(10));
#endif

#if 0
    PeriodicScheduler scheduler;
    scheduler.add_event(&callback0, std::chrono::seconds(2));
    scheduler.add_event(&callback1, std::chrono::seconds(3));
    while (true){
        scope.throw_if_cancelled();
        void* ptr = scheduler.request_next_event();
        if (ptr != nullptr){
            cout << current_time_to_str() << ": ";
            (*(std::function<void()>*)ptr)();
        }
    }
#endif



#if 0
    QImage image("20220328-043030682479.jpg");
    auto matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);
    auto session = Waterfill::make_WaterfillSession(matrix);
    auto iter = session->make_iterator(10);
    WaterfillObject object;

    std::multimap<size_t, WaterfillObject> map;
    while (iter->find_next(object, false)){
        map.emplace(object.area, object);
    }

    for (const auto& item : map){
        cout << item.first << " : " << item.second.center_x() << ", " << item.second.center_y() << endl;
    }
#endif



#if 0
    __m512i r0 = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i r1 = _mm512_setr_epi64(8, 9, 10, 11, 12, 13, 14, 15);
    __m512i r2 = _mm512_setr_epi64(16, 17, 18, 19, 20, 21, 22, 23);
    __m512i r3 = _mm512_setr_epi64(24, 25, 26, 27, 28, 29, 30, 31);

    __m256i y0, y1, y2, y3, y4, y5, y6, y7;


    transpose_64x8x4_forward(r0, r1, r2, r3, y0, y1, y2, y3, y4, y5, y6, y7);
    transpose_64x8x4_inverse(r0, r1, r2, r3, y0, y1, y2, y3, y4, y5, y6, y7);

    print_u64(r0);
    print_u64(r1);
    print_u64(r2);
    print_u64(r3);
#endif


#if 0
    __m256i m0 = _mm256_setr_epi64x(0, 1, 2, 3);
    __m256i m1 = _mm256_setr_epi64x(4, 5, 6, 7);

    __m256i f0 = _mm256_permute4x64_epi64(m0, 57);
    __m256i f1 = _mm256_permute4x64_epi64(m1, 57);
    __m256i i0 = _mm256_permute4x64_epi64(m0, 147);
    __m256i i1 = _mm256_permute4x64_epi64(m1, 147);

    print_u64(f0);
    print_u64(f1);

    print_u64(_mm256_blend_epi32(f0, f1, 0xc0));
    print_u64(_mm256_blend_epi32(_mm256_setzero_si256(), i0, 0xfc));
#endif


#if 0
    __m512i r0 = _mm512_setr_epi64(6421819613966474057, 1548476234046137462, 14049977668605968662, \
708058441056082392, 11428559378587094239, 12213545342139629759, \
17862567678864820755, 11987104709490241592);
    __m512i r1 = _mm512_setr_epi64(7012288692411820516, 6973886362582395245, 11364415477305886278, \
10322188229391563915, 3708453892541429721, 8855683905787960164, \
2605976889027609101, 403416096811372768);
    __m512i r2 = _mm512_setr_epi64(17665478609307226908, 16752009108812571050, 4454511097901708078, \
12609186068992939628, 12108615882447791236, 15410920961313526119, \
4023827012830538432, 7936743448756384845);
    __m512i r3 = _mm512_setr_epi64(3537199782762716869, 14555250848951445739, 15852154872900097320, \
10410088093334275802, 11385802811118213207, 17289061284642715804, \
7799622589958222536, 14466268115045276806);
    __m512i r4 = _mm512_setr_epi64(13769594536536065114, 12515743165021122815, 4161499395208112919, \
18168754753082368138, 4817283361637148180, 7505890873922854790, \
7303930153414137652, 1842561318276695663);
    __m512i r5 = _mm512_setr_epi64(15976838521025854814, 7681872885215311593, 900077199972924276, \
15389419866922739680, 13257707967426128688, 5781805437221625080, \
430817187407044403, 15537562097183591203);
    __m512i r6 = _mm512_setr_epi64(12804211154189096457, 8060644897201369778, 2802370319901243535, \
9320484187102063985, 1250153720052910342, 16023311416566244278, \
24977779547950565, 13830241077450685593);
    __m512i r7 = _mm512_setr_epi64(5190206910167014533, 10216993842832427235, 3528454829789924720, \
11269157269833931374, 17983684737911214650, 12805961291569691763, \
9605022433230136624, 5249211048379677373);


//    r0 = _mm512_set1_epi8(-1);
    print_8x64(r0);
//    print_8x64(r1);
//    print_8x64(r2);
//    print_8x64(r3);
    cout << "---------" << endl;




//    print_8x64(_mm512_ternarylogic_epi64(_mm512_set1_epi8(0), _mm512_set1_epi8(-1), _mm512_set1_epi8(-1), 0b11111000));

//    print_8x64(_mm512_alignr_epi64(r0, _mm512_setzero_si512(), 7));
//    print_8x64(_mm512_alignr_epi64(r1, r0, 1));

//    Intrinsics_x64_AVX512::transpose_1x64x32(r0, r1, r2, r3);
//    r0 = Intrinsics_x64_AVX512::bit_reverse(r0);
//    r1 = Intrinsics_x64_AVX512::bit_reverse(r1);
//    r2 = Intrinsics_x64_AVX512::bit_reverse(r2);
//    r3 = Intrinsics_x64_AVX512::bit_reverse(r3);
//    Intrinsics_x64_AVX512::transpose_1x64x32_bitreverse_in(r0, r1, r2, r3);

//    transpose_8x2x2x4(r0, r1);
//    r0 = transpose_1x8x8x8(r0);
//    r0 = bit_reverse(r0);
//    r0 = transpose_1x8x8x8_bitreverse_in(r0);
//    r1 = transpose_1x8x8x8(r1);

//    print_8x64(r0);
//    print_8x64(r1);

//    transpose_16x2x2x2(r0, r1, r2, r3);

//    print_8x64(r0);
//    print_8x64(r1);
//    print_8x64(r2);
//    print_8x64(r3);


//    r0 = transpose_1x8x8x8(r0);
//    r1 = transpose_1x8x8x8(r1);
//    transpose_8x2x2x4(r0, r1);


#if 0
    __m512i r = _mm512_shuffle_epi32(r0, 78);

    __m512i L = _mm512_slli_epi64(r, 1);
    __m512i H = _mm512_srli_epi64(r, 1);

    r0 = _mm512_and_si512(r0, _mm512_setr_epi64(0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa, 0x5555555555555555, 0xaaaaaaaaaaaaaaaa));
    L = _mm512_and_si512(L, _mm512_setr_epi64(0xaaaaaaaaaaaaaaaa, 0x0000000000000000, 0xaaaaaaaaaaaaaaaa, 0x0000000000000000, 0xaaaaaaaaaaaaaaaa, 0x0000000000000000, 0xaaaaaaaaaaaaaaaa, 0x0000000000000000));
    H = _mm512_and_si512(H, _mm512_setr_epi64(0x0000000000000000, 0x5555555555555555, 0x0000000000000000, 0x5555555555555555, 0x0000000000000000, 0x5555555555555555, 0x0000000000000000, 0x5555555555555555));
    r0 = _mm512_or_si512(r0, L);
    r0 = _mm512_or_si512(r0, H);

//    print_8x64(r0);

    r = _mm512_shuffle_i64x2(r0, r0, 177);
//    print_8x64(r);

    L = _mm512_slli_epi64(r, 2);
    H = _mm512_srli_epi64(r, 2);

    r0 = _mm512_and_si512(r0, _mm512_setr_epi64(0x3333333333333333, 0x3333333333333333, 0xcccccccccccccccc, 0xcccccccccccccccc, 0x3333333333333333, 0x3333333333333333, 0xcccccccccccccccc, 0xcccccccccccccccc));
    L = _mm512_and_si512(L, _mm512_setr_epi64(0xcccccccccccccccc, 0xcccccccccccccccc, 0x0000000000000000, 0x0000000000000000, 0xcccccccccccccccc, 0xcccccccccccccccc, 0x0000000000000000, 0x0000000000000000));
    H = _mm512_and_si512(H, _mm512_setr_epi64(0x0000000000000000, 0x0000000000000000, 0x3333333333333333, 0x3333333333333333, 0x0000000000000000, 0x0000000000000000, 0x3333333333333333, 0x3333333333333333));
    r0 = _mm512_or_si512(r0, L);
    r0 = _mm512_or_si512(r0, H);

//    print_8x64(r0);

    r = _mm512_shuffle_i64x2(r0, r0, 78);

    L = _mm512_slli_epi64(r, 4);
    H = _mm512_srli_epi64(r, 4);

    r0 = _mm512_and_si512(r0, _mm512_setr_epi64(0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0));
    L = _mm512_and_si512(L, _mm512_setr_epi64(0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0xf0f0f0f0f0f0f0f0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000));
    H = _mm512_and_si512(H, _mm512_setr_epi64(0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f, 0x0f0f0f0f0f0f0f0f));
    r0 = _mm512_or_si512(r0, L);
    r0 = _mm512_or_si512(r0, H);

    print_8x64(r0);



//    print_8x64(r0);


#endif

#if 0
    r0 = _mm512_shuffle_epi32(r0, 216);
//    print_8x64(r0);

    __m512i L = _mm512_srli_epi64(r0, 31);
    __m512i H = _mm512_slli_epi64(r0, 31);
    r0 = _mm512_and_si512(r0, _mm512_set1_epi64(0xaaaaaaaa55555555));
    L = _mm512_and_si512(L, _mm512_set1_epi64(0x00000000aaaaaaaa));
    H = _mm512_and_si512(H, _mm512_set1_epi64(0x5555555500000000));
    r0 = _mm512_or_si512(r0, L);
    r0 = _mm512_or_si512(r0, H);

    L = _mm512_srli_epi64(r0, 30);
    H = _mm512_slli_epi64(r0, 30);
    r0 = _mm512_and_si512(r0, _mm512_set1_epi64(0xcccccccc33333333));
    L = _mm512_and_si512(L, _mm512_set1_epi64(0x00000000cccccccc));
    H = _mm512_and_si512(H, _mm512_set1_epi64(0x3333333300000000));
    r0 = _mm512_or_si512(r0, L);
    r0 = _mm512_or_si512(r0, H);

    r0 = _mm512_shuffle_epi32(r0, 216);
    print_8x64(r0);
#endif





#if 0
    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
    cout << "---------" << endl;
//    Intrinsics_x64_AVX512GF::transpose_1x16x16x4(r0, r1);
//    Intrinsics_x64_AVX512GF::transpose_1x16x16x4(r2, r3);
//    Intrinsics_x64_AVX512GF::transpose_16x2x2x2(r0, r1, r2, r3);
    Intrinsics_x64_AVX512GF::transpose_1x64x32(r0, r1, r2, r3);
    r0 = Intrinsics_x64_AVX512GF::bit_reverse(r0);
    r1 = Intrinsics_x64_AVX512GF::bit_reverse(r1);
    r2 = Intrinsics_x64_AVX512GF::bit_reverse(r2);
    r3 = Intrinsics_x64_AVX512GF::bit_reverse(r3);
    Intrinsics_x64_AVX512GF::transpose_1x64x32_bitreverse_in(r0, r1, r2, r3);

    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
#endif

//    r0 = bit_reverse(r0);
//    r1 = bit_reverse(r1);
//    transpose_1x16x16x4_bitreverse_in(r0, r1);

//    print_8x64(r0);
//    print_8x64(r1);


#if 0
    __m512i x = r0;
    print_8x64(x);


    const __m512i INDEX0 = _mm512_setr_epi8(
        63, 55, 47, 39, 31, 23, 15,  7,
        62, 54, 46, 38, 30, 22, 14,  6,
        61, 53, 45, 37, 29, 21, 13,  5,
        60, 52, 44, 36, 28, 20, 12,  4,
        59, 51, 43, 35, 27, 19, 11,  3,
        58, 50, 42, 34, 26, 18, 10,  2,
        57, 49, 41, 33, 25, 17,  9,  1,
        56, 48, 40, 32, 24, 16,  8,  0
    );
    const __m512i INDEX1 = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );

    x = bit_reverse(x);

    x = _mm512_permutexvar_epi8(INDEX0, x);
//    x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);

    cout << "--------------" << endl;
    print_8x64(x);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    print_8x64(x);
    cout << "--------------" << endl;
    x = _mm512_permutexvar_epi8(INDEX1, x);

    print_8x64(x);
#endif


#if 0
    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
    print_8x64(r4);
    print_8x64(r5);
    print_8x64(r6);
    print_8x64(r7);
    cout << "------------" << endl;

    transpose_1x16x16x4(r0, r1);
    transpose_1x16x16x4(r2, r3);
    transpose_1x16x16x4(r4, r5);
    transpose_1x16x16x4(r6, r7);
    transpose_16x2x2x2(r0, r1, r2, r3);
    transpose_16x2x2x2(r4, r5, r6, r7);
    transpose_32x2x2(r0, r1, r2, r3, r4, r5, r6, r7);

    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
    print_8x64(r4);
    print_8x64(r5);
    print_8x64(r6);
    print_8x64(r7);
#endif

#endif

#if 0
    __m512i r0 = _mm512_setr_epi16(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    );
    __m512i r1 = _mm512_setr_epi16(
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
    );
    __m512i r2 = _mm512_setr_epi16(
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95
    );
    __m512i r3 = _mm512_setr_epi16(
        96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    );
    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
    cout << "------------" << endl;

    transpose_16x2x2x2(r0, r1, r2, r3);
    print_8x64(r0);
    print_8x64(r1);
    print_8x64(r2);
    print_8x64(r3);
#endif




#if 0
    __m512i INDEX = _mm512_setr_epi8(
         0,  8, 16, 24, 32, 40, 48, 56,
         1,  9, 17, 25, 33, 41, 49, 57,
         2, 10, 18, 26, 34, 42, 50, 58,
         3, 11, 19, 27, 35, 43, 51, 59,
         4, 12, 20, 28, 36, 44, 52, 60,
         5, 13, 21, 29, 37, 45, 53, 61,
         6, 14, 22, 30, 38, 46, 54, 62,
         7, 15, 23, 31, 39, 47, 55, 63
    );
    __m512i INDEX1 = _mm512_setr_epi8(
        56, 48, 40, 32, 24, 16,  8,  0,
        57, 49, 41, 33, 25, 17,  9,  1,
        58, 50, 42, 34, 26, 18, 10,  2,
        59, 51, 43, 35, 27, 19, 11,  3,
        60, 52, 44, 36, 28, 20, 12,  4,
        61, 53, 45, 37, 29, 21, 13,  5,
        62, 54, 46, 38, 30, 22, 14,  6,
        63, 55, 47, 39, 31, 23, 15,  7
    );
    __m512i INDEX2 = _mm512_setr_epi8(
         7, 15, 23, 31, 39, 47, 55, 63,
         6, 14, 22, 30, 38, 46, 54, 62,
         5, 13, 21, 29, 37, 45, 53, 61,
         4, 12, 20, 28, 36, 44, 52, 60,
         3, 11, 19, 27, 35, 43, 51, 59,
         2, 10, 18, 26, 34, 42, 50, 58,
         1,  9, 17, 25, 33, 41, 49, 57,
         0,  8, 16, 24, 32, 40, 48, 56
    );

    __m512i x = _mm512_setr_epi64(6421819613966474057, 1548476234046137462, 14049977668605968662, \
708058441056082392, 11428559378587094239, 12213545342139629759, \
17862567678864820755, 11987104709490241592);
    print_8x64(x);

    x = _mm512_permutexvar_epi8(INDEX1, x);

    print_8x64(x);

//    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x0102040810204080), x, 0);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);

//    print_8x64(x);

    x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);

    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
//    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x0102040810204080), x, 0);
    x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);

    print_8x64(x);

//    x = _mm512_permutexvar_epi8(INDEX, x);

    x = _mm512_permutexvar_epi8(INDEX2, x);
    print_8x64(x);
#endif




#if 0
    BitSet2D set(100, 30);
    cout << set.get(10, 20) << endl;
    set.set(10, 20);
    cout << set.get(10, 20) << endl;

    size_t x, y;
    cout << set.pop(x, y) << endl;
    cout << x << " " << y << endl;
#endif


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
        auto start = current_time();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink_Default(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = current_time();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = current_time();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst = src.scaled(src.width(), height);
        }
        auto end = current_time();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = current_time();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink<Uint8Scaler_x16_SSE41>(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = current_time();
        cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << endl;
    }
    {
        auto start = current_time();
        for (size_t c = 0; c < 1000000; c++){
            QImage dst(src.width(), height, QImage::Format_ARGB32);
            scale_vertical_shrink<Uint8Scaler_x32_AVX2>(
                src.width(),
                (const uint32_t*)src.constBits(), src.bytesPerLine(), src.height(),
                (uint32_t*)dst.bits(), dst.bytesPerLine(), dst.height()
            );
        }
        auto end = current_time();
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



//    cout << (WallClock::min() < WallClock::max()) << endl;



//    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "asdf");
//    throw UserSetupError(env.logger(), "asdf");


#if 0
    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.20, 0.95, 0.60, 0.03}, 20);

    black_screen1.process_frame(QImage("screenshot-20220221-232325966395.png"), current_time());
#endif

#if 0
    float data[25];
    for (int c = 0; c < 25; c++){
        data[c] = c;
    }
    print(data, 25);

//    auto start = current_time();

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

    QImage image = extract_box_copy(screen, box);
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
