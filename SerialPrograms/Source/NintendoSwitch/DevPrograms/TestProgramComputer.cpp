/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <mutex>
#include <condition_variable>
#include <QImage>
#include <QJsonObject>
#include <QDir>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "TestProgramComputer.h"
#include "ClientSource/Libraries/Logging.h"
#include "Common/Cpp/Containers/Pimpl.tpp"

#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x4_Default.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h"
#include "Kernels/SpikeConvolution/Kernels_SpikeConvolution.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "Kernels/AudioStreamConversion/AudioStreamConversion.h"
#include "Common/Cpp/StreamConverters.h"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "3rdParty/nlohmann/json.hpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageHSV32.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "Common/Cpp/StringTools.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "Common/Cpp/EnumDatabase.h"
#include "PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"

#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"

#ifdef PA_ARCH_x86
// #include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
// #include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
// #include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.h"
//#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/Kernels_x64_AVX2.h"
//#include "Kernels/Kernels_x64_AVX512.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.h"
#endif

//#include <opencv2/core.hpp>


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;


TestProgramComputer_Descriptor::TestProgramComputer_Descriptor()
    : ComputerProgramDescriptor(
        "Computer:TestProgram",
        "Computer", "Test Program (Computer)",
        "",
        "Test Program"
    )
{}
TestProgramComputer::TestProgramComputer()
    : STATIC_TEXT("test text")
{
    PA_ADD_OPTION(STATIC_TEXT);
}

WallClock REFERENCE = current_time();


using namespace Kernels;








void TestProgramComputer::program(ProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
//    using namespace NintendoSwitch::PokemonSwSh;
    using namespace NintendoSwitch::PokemonSV;
    using namespace Pokemon;

//    using namespace NintendoSwitch::PokemonSwSh::MaxLairInternal;


    ImageRGB32 image("SV-Buttons2.png");
    WhiteButtonDetector detector(WhiteButton::ButtonB, ImageFloatBox{0, 0, 1, 1});
    detector.detect(image);


#if 0
    ImageRGB32 image("SV-Buttons2.png");

    ImageFloatBox box(0.027, 0.922, .02, .035);
    ImageViewRGB32 region = extract_box_reference(image, box);

    region.save("tmp.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff808080, 0xffffffff);
    auto objects = Waterfill::find_objects_inplace(matrix, 20);
    extract_box_reference(region, objects[0]).save("cropped.png");
#endif


//    DialogArrowDetector detector({0.45, 0.9, .1, .05});
//    detector.detect(image);

#if 0
    ImageFloatBox box(0.72, 0.85, 0.02, 0.035);

    ImageViewRGB32 region = extract_box_reference(image, box);

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff000000, 0xff7f7fbf);

    cout << matrix.dump() << endl;

    auto arrow = Waterfill::find_objects_inplace(matrix, 20);
    extract_box_reference(region, arrow[0]).save("cropped.png");
#endif

#if 0
    ImageRGB32 image("SV-BattleMenu.png");
    image.scale_to(1920, 1080);

    GradientArrowDetector detector({0.75, 0.63, 0.05, 0.1});
    detector.detect(image);
#endif


#if 0
    ImageRGB32 image("Arrow-Alpha.png");
    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            uint32_t& pixel = image.pixel(c, r);
            if (pixel == 0xffffffff){
                pixel = 0;
            }
        }
    }
    image.save("Arrow-Alpha2.png");
#endif




#if 0
    ImageRGB32 image("SV-BattleMenu.png");
    image.scale_to(1920, 1080);

    ImageViewRGB32 box = extract_box_reference(image, ImageFloatBox({0.75, 0.63, 0.05, 0.1}));


    PackedBinaryMatrix yellow = compress_rgb32_to_binary_range(box, 0xff808000, 0xffffff7f);
    PackedBinaryMatrix blue = compress_rgb32_to_binary_range(box, 0xff004080, 0xff8fffff);

    cout << yellow.dump() << endl;
    cout << blue.dump() << endl;

//    auto session = make_WaterfillSession(yellow);
//    session.

    auto yellows = Waterfill::find_objects_inplace(yellow, 100);
    auto blues = Waterfill::find_objects_inplace(blue, 100);
    cout << yellows.size() << endl;
    cout << blues.size() << endl;

    size_t c = 0;
    for (auto& item : yellows){
        extract_box_reference(box, item).save("yellow-" + std::to_string(c++) + ".png");
    }
    c = 0;
    for (auto& item : blues){
        extract_box_reference(box, item).save("blue-" + std::to_string(c++) + ".png");
    }

    Waterfill::WaterfillObject obj = yellows[1];
    obj.merge_assume_no_overlap(blues[0]);

    extract_box_reference(box, obj).save("Arrow.png");
#endif


//    cout << "\u274c\u2705" << endl;


//    SummaryShinySymbolDetector detector;


#if 0
    BattleMenuDetector detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image("screenshot-20220814-144223026390.png");

    StandardBattleMenuDetector detector(false);
    cout << detector.detect(image) << endl;
#endif


//    STATIC_TEXT.set_text("123456789");



//    cv::Mat image;


//    env.log(QString(QChar(0x2728)));
//    env.log("\u2728\u2733");


//    throw UserSetupError(env.logger(), "Can't find any shinies? Join our Discord server and DM Elvis for FREE SHINIES!");



//    ImageRGB32 image("20220806-151454864146-rmsd_precropped_input.png");

//    image = filter_rgb32_range(image, 0xffb0b0b0, 0xffffffff, Color(0xffff0000), true);

//    image.save("test.png");


//    cout << StringTools::replace("asdf asdf adsf", "123", "---") << endl;



#if 0
    ImageRGB32 image("screenshot-20220725-170822724101.png");
    ImageHSV32 hsv(image);
#endif


#if 0
    ImageRGB32 image(100, 100);
    image.fill(0xffff0000);
    image.save("test.png");
#endif

#if 0
    ImageRGB32 image0("Avermedia-Qt5.png");
    ImageRGB32 image1(std::move(image0));

    image1.sub_image(100, 100, 100, 100).save("test.png");
#endif


#if 0
//    QImage image("20220714-114859147833-connect_to_internet_with_inference.png");
//    QImage image("MyPin-Qt6.png");

    YCommMenuDetector detector(true);
    cout << detector.detect(QImage("MiraBox-Qt5.png")) << endl;
    cout << detector.detect(QImage("MiraBox-Qt5.png")) << endl;
    cout << detector.detect(QImage("MyPin-Qt5.png")) << endl;
    cout << detector.detect(QImage("MyPin-Qt6.png")) << endl;
    cout << detector.detect(QImage("NoBrand-Qt6.png")) << endl;
    cout << detector.detect(QImage("ShadowCast-Qt6.png")) << endl;
    #endif



#if 0
    GlobalState state;
    state.boss = "dialga";
    state.players[0].pokemon = "cradily";
    state.players[1].console_id = 0;
    state.players[1].pokemon = "heatmor";
    state.players[2].pokemon = "crawdaunt";
    state.players[3].pokemon = "marowak-alola";
    state.opponent = {"flareon"};


    select_move(env.logger(), state, 1);
#endif

#if 0
    using namespace nlohmann;

    json j2 = {
      {"pi", 3.141},
      {"happy", true},
      {"name", "Niels"},
      {"nothing", nullptr},
      {"answer", {
        {"everything", 42}
      }},
      {"list", {1, 0, 2}},
      {"object", {
        {"currency", "USD"},
        {"value", 42.99}
      }}
    };

//    cout << "str = " << 123 << endl;
//    std::string str = j2.get<std::string>();
//    cout << "str = " << str << endl;

    cout << json((int8_t)1).dump() << endl;
    cout << json((int16_t)1).dump() << endl;
    cout << json((int32_t)1).dump() << endl;
    cout << json((int64_t)1).dump() << endl;

    {
        cout << j2.dump() << endl;
        JsonValue2 value = from_nlohmann(j2);
        json j3 = to_nlohmann(value);
        cout << j3.dump() << endl;
    }
    QJsonObject obj = QJsonDocument::fromJson(j2.dump().c_str()).object();
    {
        cout << j2.dump() << endl;
        JsonValue2 value = from_QJson(obj);
        QJsonValue obj2 = to_QJson(value);
//        cout << QJson_to_nlohmann(obj2).dump() << endl;
    }

//    cout << QDir::current().relativeFilePath(RESOURCE_PATH()).toStdString() << endl;
#endif

#if 0
    std::string str;


    cout << j2.dump(4) << endl;

    json j3 = "asdf";
    cout << j3.dump(4) << endl;

    QJsonDocument doc(QJsonDocument::fromJson("asdf"));
    cout << doc.toJson().data() << endl;
#endif

#if 0
    int16_t in[4] = {1, 1, 2, -2};
    print_u8((uint8_t*)in, 8);



    AudioSourceReader reader(AudioSampleFormat::SINT16, 1, false);
    AudioListener listener(1);
    reader += listener;


//    reader.push_bytes(in, 8);
    reader.push_bytes((char*)in + 0, 5);
    reader.push_bytes((char*)in + 5, 3);
#endif



//    AudioStreamReader2 reader(2, AudioSampleFormat::SINT16);



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
