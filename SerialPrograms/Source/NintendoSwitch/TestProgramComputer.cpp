/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QImage>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/AlignedVector.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "TestProgramComputer.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixBase.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Tile_Default.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_SSE42.h"
#ifdef PA_Arch_x64_AVX2
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX2.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX512.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Tile_x64_AVX512.h"
#endif
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

#include "Kernels/AbsFFT/Kernels_AbsFFT.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_Arch.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_TwiddleTable.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_Butterflies.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_ComplexScalar.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_ComplexVector.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_ComplexToAbs.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_BitReverse.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_Reductions.h"
#include "Kernels/AbsFFT/Kernels_AbsFFT_FullTransform.h"

//#include "Kernels/Kernels_x64_AVX2.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;
using namespace Kernels::AbsFFT;


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
void print(const scomplex* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        if (ptr[c].r == 0){
            cout << ptr[c].i << " i";
        }else if (ptr[c].i == 0){
            cout << ptr[c].r;
        }else if (ptr[c].i > 0){
            cout << ptr[c].r << " + " << ptr[c].i << " i";
        }else{
            cout << ptr[c].r << " - " << -ptr[c].i << " i";
        }
    }
    cout << "}" << endl;
}







void TestProgramComputer::program(ProgramEnvironment& env){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonSwSh;
    using namespace Pokemon;


#if 0
    float x[16];
    for (int c = 0; c < 16; c++){
        x[c] = c;
    }
    __m256 r0 = _mm256_loadu_ps(x +  0);
    __m256 r1 = _mm256_loadu_ps(x +  8);
    print(r0);
    print(r1);
    cout << "---------------" << endl;

    r0 = _mm256_permutevar8x32_ps(r0, _mm256_setr_epi32(0, 7, 2, 5, 4, 3, 6, 1));
    r1 = _mm256_permutevar8x32_ps(r1, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
    print(r0);
#endif

#if 0
    float x[16];
    for (int c = 0; c < 16; c++){
        x[c] = c;
    }
    __m256 r0 = _mm256_loadu_ps(x +  0);
    __m256 r1 = _mm256_loadu_ps(x +  8);
    print(r0);
    print(r1);
    cout << "---------------" << endl;

    __m256 a0 = _mm256_permute2f128_ps(r0, r1, 32);
    __m256 a1 = _mm256_permute2f128_ps(r0, r1, 49);
    print(a0);
    print(a1);
    cout << "---------------" << endl;

    r0 = _mm256_permutevar8x32_ps(a0, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
    r1 = _mm256_permutevar8x32_ps(a1, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
//    r0 = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(a0), 216));
//    r1 = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(a1), 216));
    print(r0);
    print(r1);
    cout << "---------------" << endl;
#endif

#if 0
    float x[64];
    for (int c = 0; c < 64; c++){
        x[c] = c;
    }

    __m256 r0 = _mm256_loadu_ps(x +  0);
    __m256 r1 = _mm256_loadu_ps(x +  8);
    __m256 r2 = _mm256_loadu_ps(x + 16);
    __m256 r3 = _mm256_loadu_ps(x + 24);
    __m256 r4 = _mm256_loadu_ps(x + 32);
    __m256 r5 = _mm256_loadu_ps(x + 40);
    __m256 r6 = _mm256_loadu_ps(x + 48);
    __m256 r7 = _mm256_loadu_ps(x + 56);
    print(r0);
    print(r1);
    print(r2);
    print(r3);
    print(r4);
    print(r5);
    print(r6);
    print(r7);
    cout << "---------------" << endl;

    vtranspose(r0, r1, r2, r3, r4, r5, r6, r7);
    cout << "---------------" << endl;


    print(r0);
    print(r1);
    print(r2);
    print(r3);
    print(r4);
    print(r5);
    print(r6);
    print(r7);
#endif



#if 0
    const int k = 5;
    const size_t LENGTH = (size_t)1 << k;

    uint64_t data[LENGTH];
    uint64_t temp[LENGTH];
    for (uint64_t c = 0; c < LENGTH; c++){
        data[c] = c;
    }

    bitreverse_u64_ip(k, data, temp);
    print(data, LENGTH);
//    print(temp, LENGTH);


    for (size_t c = 0; c < LENGTH; c++){
        cout << bitreverse64(c, k) << ", ";
    }
#endif



#if 0
    float in[32] = {
        0, 1, 16, 17, 8, 9, 24, 25, 4, 5, 20, 21, 12, 13, 28, 29,
        2, 3, 18, 19, 10, 11, 26, 27, 6, 7, 22, 23, 14, 15, 30, 31
    };
    float out[32];

    interleave_layers1_f32vk_Default(1, 5, out, in);
    interleave_layers1_f32vk_Default(2, 5, in, out);
    interleave_layers1_f32vk_Default(3, 5, out, in);
    print(in, 32);
    print(out, 32);
#endif


#if 0
    __m128 r0 = _mm_setr_ps(0, 1, 2, 3);
    __m128 r1 = _mm_setr_ps(4, 5, 6, 7);
    print(r0);
    print(r1);

//    r0 = _mm_shuffle_ps(r0, r0, 108);
//    r1 = _mm_shuffle_ps(r1, r1, 108);
//    print(r0);

//    print(_mm_blend_ps(r0, _mm_shuffle_ps(r1, r1, 108), 10));
//    print(_mm_blend_ps(r1, _mm_shuffle_ps(r0, r0, 108), 10));

    print(_mm_shuffle_ps(r0, r1, 68));
    print(_mm_shuffle_ps(r0, r1, 238));


#endif



#if 1
    const int k = 9;
    const size_t LENGTH = (size_t)1 << k;

//    TwiddleTable table(k);
    alignas(64) float R[LENGTH] = {5, 8, 4, 0, 0, 3, 7, 7, 9, 9, 5, 3, 1, 9, 0, 8, 2, 4, 3, 7, 9, 8, 8, \
4, 2, 1, 5, 8, 0, 3, 2, 7, 4, 2, 0, 3, 3, 9, 2, 8, 7, 4, 0, 7, 4, 9, \
5, 8, 8, 3, 8, 1, 9, 0, 3, 8, 8, 3, 9, 9, 0, 9, 0, 8, 2, 2, 1, 7, 0, \
0, 7, 5, 9, 9, 2, 2, 0, 2, 3, 1, 3, 7, 4, 7, 1, 5, 4, 5, 8, 6, 7, 6, \
4, 5, 9, 9, 5, 9, 4, 2, 2, 9, 0, 5, 2, 2, 2, 9, 5, 6, 8, 8, 4, 6, 0, \
4, 1, 2, 8, 8, 2, 3, 1, 3, 4, 7, 7, 9, 7, 4, 6, 8, 5, 3, 4, 0, 1, 0, \
7, 0, 2, 6, 3, 3, 7, 6, 0, 0, 4, 3, 6, 2, 6, 9, 0, 3, 4, 2, 8, 0, 9, \
2, 7, 5, 6, 4, 0, 3, 6, 1, 3, 0, 0, 4, 1, 3, 0, 0, 1, 1, 6, 3, 7, 0, \
7, 2, 5, 7, 7, 5, 9, 7, 2, 2, 2, 6, 5, 8, 0, 3, 7, 0, 2, 8, 9, 4, 7, \
4, 0, 3, 5, 8, 5, 6, 9, 1, 6, 0, 7, 2, 1, 7, 1, 0, 1, 0, 8, 8, 0, 4, \
6, 8, 2, 1, 9, 9, 1, 0, 1, 1, 3, 8, 2, 3, 4, 6, 4, 1, 2, 6, 1, 2, 5, \
5, 1, 7, 3, 4, 7, 2, 8, 7, 8, 0, 1, 2, 3, 4, 7, 3, 7, 5, 3, 1, 7, 3, \
8, 8, 3, 2, 7, 7, 9, 2, 7, 9, 3, 2, 6, 7, 6, 0, 5, 5, 0, 3, 1, 7, 7, \
1, 8, 4, 2, 8, 5, 8, 5, 3, 5, 1, 8, 5, 1, 9, 7, 4, 6, 8, 5, 7, 7, 9, \
8, 3, 0, 3, 9, 4, 6, 7, 2, 2, 9, 9, 5, 8, 3, 5, 2, 4, 6, 4, 5, 5, 6, \
2, 5, 4, 5, 5, 4, 2, 1, 9, 7, 7, 7, 2, 7, 6, 0, 2, 3, 1, 2, 4, 6, 8, \
3, 0, 6, 8, 9, 6, 5, 2, 8, 9, 5, 5, 2, 6, 9, 5, 9, 8, 4, 7, 0, 5, 0, \
6, 2, 7, 2, 2, 1, 4, 8, 4, 0, 0, 4, 4, 5, 9, 6, 4, 4, 2, 4, 5, 9, 2, \
9, 8, 4, 5, 2, 6, 9, 4, 5, 8, 8, 7, 8, 4, 1, 6, 5, 5, 2, 9, 9, 4, 0, \
4, 9, 8, 5, 2, 9, 7, 3, 1, 2, 9, 5, 8, 5, 3, 6, 0, 8, 4, 7, 9, 1, 8, \
5, 1, 5, 9, 1, 7, 9, 2, 6, 3, 5, 4, 7, 9, 9, 3, 9, 3, 4, 1, 7, 3, 0, \
9, 0, 7, 7, 2, 7, 6, 2, 2, 6, 1, 4, 0, 2, 8, 7, 3, 9, 2, 7, 1, 0, 9, \
1, 0, 7, 0, 4, 3};
    alignas(64) float A[LENGTH / 2];

    fft_abs(k, A, R);

//    print(R + 16, 16);
    print(A, LENGTH / 2);
#endif



#if 0
    float x[16];
    for (int c = 0; c < 16; c++){
        x[c] = c;
    }

    __m128 v0 = _mm_loadu_ps(x +  0);
    __m128 v1 = _mm_loadu_ps(x +  4);
    __m128 v2 = _mm_loadu_ps(x +  8);
    __m128 v3 = _mm_loadu_ps(x + 12);
    print(v0);
    print(v1);
    print(v2);
    print(v3);
    cout << "--------------" << endl;
    transpose_f32_4x4_SSE2(v0, v1, v2, v3);
    cout << "--------------" << endl;
    print(v0);
    print(v1);
    print(v2);
    print(v3);
#endif


#if 0
    const int k = 6;
    const size_t LENGTH = (size_t)1 << k;

    TwiddleTable table(k);

    float r[LENGTH] = {6, 5, 0, 8, 0, 3, 2, 0, 6, 8, 6, 7, 8, 5, 4, 2, 0, 7, 5, 2, 3, 0, 6, \
9, 0, 2, 0, 5, 0, 7, 9, 5, 3, 0, 2, 7, 0, 9, 3, 6, 9, 8, 3, 2, 8, 7, \
4, 4, 6, 0, 3, 9, 5, 9, 9, 2, 0, 9, 4, 2, 1, 6, 0, 3};
    float i[LENGTH] = {2, 4, 3, 8, 8, 4, 8, 2, 7, 4, 5, 5, 0, 0, 1, 3, 4, 3, 0, 8, 3, 5, 1, \
6, 1, 5, 0, 0, 9, 9, 0, 0, 7, 8, 2, 2, 5, 9, 9, 6, 4, 2, 4, 7, 5, 9, \
7, 9, 2, 2, 0, 7, 9, 9, 5, 9, 1, 9, 2, 6, 4, 8, 4, 2};
    float T[2*LENGTH];
    float O[2*LENGTH];
    for (size_t c = 0; c < LENGTH; c++){
#if 0
        T[2*c + 0] = r[c];
        T[2*c + 1] = i[c];
#else
        size_t vindex = c / VECTOR_LENGTH;
        size_t sindex = c % VECTOR_LENGTH;
        T[vindex * 16 + sindex + 0] = r[c];
        T[vindex * 16 + sindex + 8] = i[c];
#endif
    }


//    print(T, 2*LENGTH);

//    fft_complex_tk(table, k, T);
    fft_complex_tk(table, k, (vtype*)T);

//    print(T, LENGTH * 2);

    for (size_t c = 0; c < LENGTH; c++){
#if 0
        O[2*c + 0] = T[2*c + 0];
        O[2*c + 1] = T[2*c + 1];
#else
        size_t vindex = c / VECTOR_LENGTH;
        size_t sindex = c % VECTOR_LENGTH;
        O[2*c + 0] = T[vindex * 16 + sindex + 0];
        O[2*c + 1] = T[vindex * 16 + sindex + 8];
#endif
    }
    print(O, LENGTH * 2);

    double sum = 0;
    for (size_t c = 0; c < 2*LENGTH; c++){
        sum += O[c];
    }
    cout << "sum = " << sum << endl;
#endif


//    fft_abssqr_t2(out, in);

#if 0
    int k = 4;
    AlignedVector<vcomplex> row = make_table_row(k, 1);
    size_t size = (1 << k) / 4;
    for (size_t c = 0; c < size; c++){
        const vcomplex& vec = row[c / VECTOR_LENGTH];
        size_t index = c % VECTOR_LENGTH;
        cout << "{" << vec.real(index) << " + " << vec.imag(index) << "}";
    }
    cout << endl;
#endif

//    TwiddleTable table(10);



#if 0
    float in[32] = {7, 9, 8, 9, 9, 0, 2, 9, 7, 8, 8, 2, 9, 3, 0, 9, 6, 4, 7, 0, 2, 5, 4, 8, 9, 3, 4, 2, 9, 7, 3, 6};
    float br[32];
    float out[16] = {};
    cout << br << " : " << br + 32 << endl;

    fft_real_br_t5(table, br, in);
    print(br, 32);
//    br_to_abssqr_k5(out, br);
//    print(out, 16);
#endif


#if 0
    const int k = 3;
    const size_t LENGTH = (size_t)1 << k;


    float r[LENGTH] = {5, 4, 5, 6, 5, 8, 4, 8};
    float i[LENGTH] = {4, 5, 0, 3, 2, 5, 2, 7};
    scomplex T[LENGTH];
    for (size_t c = 0; c < LENGTH; c++){
        T[c].r = r[c];
        T[c].i = i[c];
    }

    print(T, 8);

    fft_complex_posicyclic_br_t3(T);
    print(T, 8);
#endif



#if 0
    float in[8] = {5, 2, 7, 3, 3, 8, 1, 1};
    float br[8];
    float out[4] = {};

    fft_real_br_t3(br, in);
    print(br, 8);
    br_to_abssqr_k3(out, br);
    print(out, 4);
#endif




#if 0
    QImage image("screenshot-20220127-200227422225.png");
    image = extract_box(image, ImageFloatBox{0.4, 0.1, 0.05, 0.2});
    image.save("test.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50, false);
    cout << "objects = " << objects.size() << endl;

    int c = 0;
    for (ImagePixelBox box : objects){
        extract_box(image, box).save("image-" + QString::number(c++) + ".png");
    }

    WaterfillObject obj = objects[3];
    obj.merge_assume_no_overlap(objects[4]);

    extract_box(image, obj).save("image.png");
#endif


#if 0
    QImage image("screenshot-20220125-192616059686.png");

    image = extract_box(image, ImageFloatBox{0.5, 0.05, 0.25, 0.4});
    image.save("test.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50, false);
    cout << "objects = " << objects.size() << endl;

    int c = 0;
    for (ImagePixelBox box : objects){
        extract_box(image, box).save("image-" + QString::number(c++) + ".png");
    }
#endif



//    QImage image("20220125-083836057990-NameOCR-eng.png");
//    std::set<std::string> slugs = read_name(env.logger(), Language::English, image, {0.11, 0.868, 0.135, 0.043});




//    QImage image("screenshot-20220123-215131034370.png");
//    std::vector<ImagePixelBox> objects = find_exclamation_marks(image);

#if 0
    QImage image("Flag-Original.png");
    image = image.scaled(image.width() / 2, image.height() / 2);
    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
//            if (red < 128 && green < 128 && blue < 128){
//                pixel = 0x00000000;
//            }
            if (red < 128 || green < 128 || blue < 128){
                pixel = 0x00000000;
            }
        }
    }
    image.save("Flag-Template0.png");
#endif


#if 0
    QImage image("screenshot-20220123-212415701291.png");
//    std::vector<ImagePixelBox> objects = find_exclamation_marks(image);

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        192, 255,
        192, 255,
        192, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50, false);
    cout << "objects = " << objects.size() << endl;

    int c = 0;
    for (ImagePixelBox box : objects){
        extract_box(image, box).save("image-" + QString::number(c++) + ".png");
    }
#endif



#if 0
    QImage image("screenshot-20220123-203000945405.png");


    std::vector<ImagePixelBox> objects = find_exclamation_marks(image);
#endif



#if 0
    QImage image("20220122-020706133705-Briefcase.png");

    QImage briefcase(RESOURCE_PATH() + "PokemonBDSP/StarterBriefcase.png");
    ImageMatchWatcher detector(briefcase, {0.5, 0.1, 0.5, 0.7}, 100);
    detector.detect(image);

    EventNotificationOption notification("Test Notification", true, true, ImageAttachmentMode::JPG);

    send_program_notification(
        env.logger(),
        notification,
        COLOR_GREEN,
        env.program_info(),
        "Image Test",
        {},
        image, false
    );
#endif


//    QImage image("mark.png");
//    image = image.convertToFormat(QImage::Format::Format_ARGB32);

//    QImage image("screenshot-20220119-184839255629.png");
//    QImage image("screenshot-20220119-190101351053.png");
//    find_exclamation_marks(image);

//    ExclamationMatcher matcher;


//    QImage image("20211008-053920535759.jpg");




#if 0
    QImage image("screenshot-20220119-190101351053.png");
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        192, 255,
        0, 160,
        0, 192
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50, false);
#endif





#if 0
    extract_object_from_inner_feature(
        QImage(100, 100, QImage::Format::Format_ARGB32),
        ImagePixelBox(60, 60, 70, 70),
        ImageFloatBox(0.5, 0.5, 0.5, 0.5)
    );
#endif


#if 0
    QImage image(RESOURCE_PATH() + "PokemonSwSh/ExclamationMark.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        192, 255,
        0, 160,
        0, 192
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50, false);
    if (objects.size() != 0){
        PA_THROW_StringException("Failed to find exactly one object in resource.");
    }
#endif




#if 0
    QImage image(QImage(RESOURCE_PATH() + "PokemonSwSh/ExclamationMark-Original.png"));
    image = image.copy(63, 5, 67, 143);
    image = image.scaled(image.width() / 4, image.height() / 4);
    image = image.convertToFormat(QImage::Format::Format_ARGB32);

    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
            if (red < 128 && green < 128 && blue < 128){
                pixel = 0;
            }
            if (red >= 128 && green >= 128 && blue >= 128){
                pixel = 0xffffffff;
            }
        }
    }
    image.save("test.png");
#endif



//    QImage image("mark0.png");
//    image.scaled(image.width() / 4, image.height() / 4).save("mark1.png");



#if 0
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
            if (green >= 192 && blue >= 192){
                continue;
            }
            if (red >= 192){
                continue;
            }
            pixel = 0xff000000;
        }
    }
    image.save("test.png");
#endif



#if 0
    QImage image("screenshot-20220119-190101351053.png");
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(image, 192, 192, 192);
    std::vector<WaterfillObject> objects = find_objects(matrix, 200, false);
    cout << objects.size() << endl;

    int c = 0;
    for (const auto& item : objects){
        image.copy(item.min_x, item.min_y, item.width(), item.height()).save("test-" + QString::number(c++) + ".png");
    }
#endif




#if 0
    QImage image("20220111-125825502573-ProgramHang.png");

    NintendoSwitch::PokemonSwSh::MaxLairInternal::BattleMenuDetector detector;
    cout << detector.detect(image) << endl;
#endif


#if 0
    QImage image("screenshot-20220108-185053570093.png");
//    BattleMenuReader reader();

    image = extract_box(image, ImageFloatBox(0.640, 0.600, 0.055, 0.380));
    image.save("test.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_max(image, 64, 64, 64);
#if 0
    cout << "matrix: " << matrix.width() << " x " << matrix.height() << endl;
    for (size_t r = 0; r < matrix.tile_height(); r++){
        for (size_t c = 0; c < matrix.tile_width(); c++){
            cout << "(" << c << ", " << r << ")" << endl;
            cout << matrix.tile(c, r).dump() << endl;
        }
        break;
    }
#endif
    std::vector<WaterfillObject> objects = find_objects(matrix, 200, true);

    cout << objects.size() << endl;
    size_t c = 0;
    for (const WaterfillObject& object : objects){
        ImagePixelBox box(object.min_x, object.min_y, object.max_x, object.max_y);
        cout << object.min_x << ", " << object.min_y << ", " << object.max_x << ", " << object.max_y << endl;
        extract_box(image, box).save("test-" + QString::number(c++) + ".png");
    }

    WaterfillObject& object = objects[1];
//    object.max_x -= 1;
//    cout << matrix.dump(object.min_x, object.min_y, object.max_x, object.max_y) << endl;
    {
        PackedBinaryMatrix object_matrix = object.packed_matrix();
        object_matrix.invert();
        cout << object_matrix.dump(0, 0, object.width(), object.height()) << endl;
    }

    QImage objimg = image.copy(object.min_x, object.min_y, object.width(), object.height());

#if 1
    filter_rgb32(
        object.packed_matrix(),
        (uint32_t*)objimg.bits(), objimg.bytesPerLine(),
        0xffffffff, true
    );
    cout << object.object.width() << " x " << object.object.height() << endl;
    cout << objimg.width() << " x " << objimg.height() << endl;
#endif
#if 0
    for (int r = 0; r < objimg.height(); r++){
        for (int c = 0; c < objimg.width(); c++){
            ((uint32_t*)objimg.bits())[c + objimg.bytesPerLine() / 4 * r] = 0xff00ff00;
        }
    }
#endif
    objimg.save("filtered.png");

#endif




#if 0
    {
        __m512i x = _mm512_set1_epi64(8723783780267303537);
        x = _mm512_shuffle_epi8(
            x,
            _mm512_setr_epi8(
                7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
            )
        );
        x = _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
        print_u8(x);
    }
    {
        __m512i x = _mm512_set1_epi64(8723783780267303537);
        x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
        print_u8(x);
    }
    {
        __m512i x = _mm512_set1_epi64(8723783780267303537);
        x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x0000000000000001), x, 0);
        print_u8(x);
    }
#endif



#if 0
    QImage image("20211008-053920535759.jpg");
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        255, 255,
        192, 255,
        192, 255,
        128, 255
    );

    std::vector<WaterfillObject> objects1 = find_objects(matrix, 1, false);
    cout << "objects = " << objects1.size() << endl;
    std::multimap<uint64_t, WaterfillObject> sorted1;
    for (const auto& item : objects1){
        sorted1.emplace(item.area, item);
    }
    for (const auto& item : sorted1){
        cout << "area = " << item.second.area
             << " - (" << item.second.min_x
             << "," << item.second.max_x
             << ")(" << item.second.min_y
             << "," << item.second.max_y
             << ") - (" << item.second.center_x()
             << "," << item.second.center_y()
             << ")"
             << " sum_y = " << item.second.sum_y
             << endl;
    }
//    cout << matrix.dump(1046, 588, 1048, 592) << endl;
//    cout << matrix.tile(16, 73).dump() << endl;
//    cout << matrix.tile(16, 74).dump() << endl;

//    PackedBinaryMatrix submatrix = matrix.submatrix(849, 365, 863-849 - 1, 376-365 - 1);
//    cout << submatrix.width() << " x " << submatrix.height() << endl;
//    cout << submatrix.dump() << endl;
//    cout << submatrix.tile(0, 0).dump() << endl;
//    cout << submatrix.tile(0, 1).dump() << endl;
//    cout << submatrix.tile(0, 2).dump() << endl;
#endif

#if 0
    BinaryTile_AVX512 tile;
    tile.set_ones();
    cout << tile.dump() << endl;

    {
        BinaryTile_AVX512 dest;
        dest.set_zero();
        tile.copy_to_shift_pp(dest, 5, 9);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX512 dest;
        dest.set_zero();
        tile.copy_to_shift_np(dest, 59, 9);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX512 dest;
        dest.set_zero();
        tile.copy_to_shift_pn(dest, 5, 55);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX512 dest;
        dest.set_zero();
        tile.copy_to_shift_nn(dest, 59, 55);
        cout << dest.dump() << endl;
    }
#endif

#if 0
    BinaryTile_AVX2 tile;
    tile.set_ones();
    cout << tile.dump() << endl;

    {
        BinaryTile_AVX2 dest;
        dest.set_zero();
        tile.copy_to_shift_pp(dest, 5, 3);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX2 dest;
        dest.set_zero();
        tile.copy_to_shift_np(dest, 59, 3);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX2 dest;
        dest.set_zero();
        tile.copy_to_shift_pn(dest, 5, 13);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_AVX2 dest;
        dest.set_zero();
        tile.copy_to_shift_nn(dest, 59, 13);
        cout << dest.dump() << endl;
    }
#endif


#if 0
    BinaryTile_SSE42 tile;
    tile.set_ones();
    cout << tile.dump() << endl;

    {
        BinaryTile_SSE42 dest;
        dest.set_zero();
        tile.copy_to_shift_pp(dest, 2, 2);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_SSE42 dest;
        dest.set_zero();
        tile.copy_to_shift_np(dest, 62, 2);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_SSE42 dest;
        dest.set_zero();
        tile.copy_to_shift_pn(dest, 2, 6);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_SSE42 dest;
        dest.set_zero();
        tile.copy_to_shift_nn(dest, 62, 6);
        cout << dest.dump() << endl;
    }
#if 0
    BinaryTile_SSE42 dest;
    dest.set_zero();
    tile.copy_to_shift_pp(dest, 2, 1);
    tile.copy_to_shift_np(dest, 62, 1);
    tile.copy_to_shift_pn(dest, 2, 7);
    tile.copy_to_shift_nn(dest, 62, 7);
    cout << dest.dump() << endl;
#endif
#endif


#if 0
    BinaryTile_Default tile;
    tile.set_ones();
//    tile.vec[0] = 14506275885351140242ull;
//    tile.vec[1] = 11153623425119109148ull;
//    tile.vec[2] = 4283232600608028042ull;
//    tile.vec[3] = 11483095140456459570ull;
    cout << tile.dump() << endl;

    {
        BinaryTile_Default dest;
        dest.set_zero();
        tile.copy_to_shift_pp(dest, 2, 1);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_Default dest;
        dest.set_zero();
        tile.copy_to_shift_np(dest, 62, 1);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_Default dest;
        dest.set_zero();
        tile.copy_to_shift_pn(dest, 2, 3);
        cout << dest.dump() << endl;
    }
    {
        BinaryTile_Default dest;
        dest.set_zero();
        tile.copy_to_shift_nn(dest, 62, 3);
        cout << dest.dump() << endl;
    }
    BinaryTile_Default dest;
    dest.set_zero();
    tile.copy_to_shift_pp(dest, 2, 1);
    tile.copy_to_shift_np(dest, 62, 1);
    tile.copy_to_shift_pn(dest, 2, 3);
    tile.copy_to_shift_nn(dest, 62, 3);
    cout << dest.dump() << endl;
#endif


#if 0
    QImage image("20220101-214116617784.jpg");

    cout << (void*)image.constBits() << endl;
    cout << image.bytesPerLine() << endl;

    image = image.copy(100, 100, 100, 100);

    cout << (void*)image.constBits() << endl;
    cout << image.bytesPerLine() << endl;
#endif


#if 0
    QImage image("20220101-214116617784.jpg");
    {
        auto time0 = std::chrono::system_clock::now();
        CellMatrix matrix0(image);
        BrightYellowLightFilter filter;
        matrix0.apply_filter(image, filter);
        auto time1 = std::chrono::system_clock::now();
        std::vector<FillGeometry> objects0 = find_all_objects(matrix0, 1, false, 10);
        auto time2 = std::chrono::system_clock::now();
        cout << "filter  = " << time1 - time0 << endl;
        cout << "process = " << time2 - time1 << endl;
        cout << "objects = " << objects0.size() << endl;
    }
    {
        auto time0 = std::chrono::system_clock::now();
        PackedBinaryMatrix matrix1 = filter_rgb32_range(
            image,
            255, 255,
            192, 255,
            192, 255,
            128, 255
        );
        auto time1 = std::chrono::system_clock::now();
        std::vector<WaterfillObject> objects1 = find_objects(matrix1, 10, false);
        auto time2 = std::chrono::system_clock::now();
        cout << "filter  = " << time1 - time0 << endl;
        cout << "process = " << time2 - time1 << endl;
        cout << "objects = " << objects1.size() << endl;
    }
#endif



#if 0
    QImage image("20220101-214116617784.jpg");
    {
        CellMatrix matrix0(image);
        BrightYellowLightFilter filter;
        matrix0.apply_filter(image, filter);
        std::vector<FillGeometry> objects0 = find_all_objects(matrix0, 1, false, 1);
        cout << "objects = " << objects0.size() << endl;
        std::multimap<uint64_t, FillGeometry> sorted0;
        for (const auto& item : objects0){
            sorted0.emplace(item.area, item);
        }
        for (const auto& item : sorted0){
            cout << "area = " << item.second.area
                 << " - (" << item.second.box.min_x
                 << "," << item.second.box.max_x
                 << ")(" << item.second.box.min_y
                 << "," << item.second.box.max_y
                 << ") - (" << item.second.center_x()
                 << "," << item.second.center_y()
                 << ")" << endl;
        }
    }
    {
        PackedBinaryMatrix matrix1 = filter_rgb32_range(
            image,
            255, 255,
            192, 255,
            192, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects1 = find_objects(matrix1, 1, false);
        cout << "objects = " << objects1.size() << endl;
        std::multimap<uint64_t, WaterfillObject> sorted1;
        for (const auto& item : objects1){
            sorted1.emplace(item.area, item);
        }
        for (const auto& item : sorted1){
            cout << "area = " << item.second.area
                 << " - (" << item.second.min_x
                 << "," << item.second.max_x
                 << ")(" << item.second.min_y
                 << "," << item.second.max_y
                 << ") - (" << item.second.center_x()
                 << "," << item.second.center_y()
                 << ")" << endl;
        }
    }
#endif


#if 0
    QImage image("20220101-214116617784.jpg");
    CellMatrix matrix0(image);
    BrightYellowLightFilter filter;
    matrix0.apply_filter(image, filter);
    PackedBinaryMatrix matrix1 = filter_rgb32_range(
        image,
        255, 255,
        192, 255,
        192, 255,
        128, 255
    );

//    cout << matrix0[425][819] << endl;
//    cout << matrix0[425][820] << endl;
//    cout << matrix1.get(819, 425) << endl;
//    cout << matrix1.get(820, 425) << endl;
    cout << matrix1.dump(694, 525, 719, 541) << endl;
    cout << matrix1.dump(694, 526, 704, 540) << endl;
//    matrix1.set(819, 425, 0);
//    matrix1.set(820, 425, 0);
//    cout << matrix1.dump(811, 403, 835, 435) << endl;

//    cout << matrix1.dump(64*12, 8*53, 64*12 + 64, 8*53 + 8) << endl;

#if 0
    BinaryTile_AVX512& mask = matrix1.tile(12, 6);
    cout << mask.dump() << endl;

    BinaryTile_AVX512 tile;
    tile.set_zero();
    tile.set_bit(63, 32, 1);

    cout << "------------" << endl;
    Waterfill::waterfill_expand(mask, tile);
    cout << tile.dump() << endl;
#endif

#if 0
    WaterfillObject object;
    find_object(matrix1, object, 12, 50);
    cout << "area = " << object.m_area
         << " - (" << object.m_min_x
         << "," << object.m_max_x
         << ")(" << object.m_min_y
         << "," << object.m_max_y
         << ")" << endl;
#endif

#if 0
    std::vector<FillGeometry> objects0 = find_all_objects(matrix0, 1, false, 1);
    cout << "objects = " << objects0.size() << endl;
    std::vector<WaterfillObject> objects1 = find_objects(matrix1);
    cout << "objects = " << objects1.size() << endl;



    std::multimap<uint64_t, FillGeometry> sorted0;
    for (const auto& item : objects0){
        sorted0.emplace(item.area, item);
    }

    std::multimap<uint64_t, WaterfillObject> sorted1;
    for (const auto& item : objects1){
        sorted1.emplace(item.m_area, item);
    }
#endif

#if 0
    for (const auto& item : sorted0){
        cout << "area = " << item.second.area
             << " - (" << item.second.box.min_x
             << "," << item.second.box.max_x
             << ")(" << item.second.box.min_y
             << "," << item.second.box.max_y
             << ")" << endl;
    }
    for (const auto& item : sorted1){
        cout << "area = " << item.second.m_area
             << " - (" << item.second.m_min_x
             << "," << item.second.m_max_x
             << ")(" << item.second.m_min_y
             << "," << item.second.m_max_y
             << ")" << endl;
    }
#endif
#endif



#if 0
    QImage image("20220101-214116617784.jpg");


    CellMatrix matrix0(image);
    BrightYellowLightFilter filter;
    matrix0.apply_filter(image, filter);

    PackedBinaryMatrix matrix1 = filter_rgb32_range(
        image,
        255, 255,
        192, 255,
        192, 255,
        128, 255
    );

    for (size_t r = 0; r < (size_t)matrix0.height(); r++){
        for (size_t c = 0; c < (size_t)matrix0.width(); c++){
            if (!!matrix0[r][c] != matrix1.get(c, r)){
                cout << "(" << c << "," << r << ")" << endl;
            }
        }
    }
    cout << "asdf" << endl;

#if 0
    {
        std::vector<FillGeometry> objects;
        objects = find_all_objects(matrix0, 1, false, 1);
        cout << "objects = " << objects.size() << endl;
    }
    {
        std::vector<WaterfillObject> objects = find_objects(matrix1);
        cout << "objects = " << objects.size() << endl;
    }
#endif
#endif


#if 0
    QImage image("screenshot-20211227-082121670685.png");
    image = extract_box(image, ImageFloatBox({0.95, 0.10, 0.05, 0.10}));
    image.save("test.png");

//    QImage image("screenshot-20211227-082121670685.png");

#if 0
    PackedBinaryMatrix matrix(image.width(), image.height());
    cout << "width  = " << matrix.width() << endl;
    cout << "height = " << matrix.height() << endl;
    cout << "width  = " << matrix.tile_width() << endl;
    cout << "height = " << matrix.tile_height() << endl;

    Compressor_RgbRange_x64_AVX512 filter(
        255, 255,
        128, 255,
        0, 128,
        0, 128
    );
#endif

#if 1
    PackedBinaryMatrix matrix = filter_rgb32_range(
        image,
        255, 255,
        128, 255,
        0, 128,
        0, 128
    );
    cout << "width  = " << matrix.tile_width() << endl;
    cout << "height = " << matrix.tile_height() << endl;
    cout << matrix.dump() << endl;


//    WaterfillObject object;
//    find_object(matrix, object, 0, 0);
    std::vector<WaterfillObject> objects = find_objects(matrix);

    for (const auto& object : objects){
        cout << "x = (" << object.m_min_x << "," << object.m_max_x << ")" << endl;
        cout << "y = (" << object.m_min_y << "," << object.m_max_y << ")" << endl;
        cout << "area = " << object.m_area << endl;
        cout << "sum x = " << object.m_sum_x << endl;
        cout << "sum y = " << object.m_sum_y << endl;
    }

#endif

//    cout << matrix.tile(1, 1).dump() << endl;
#endif




#if 0
    BinaryTile_AVX2 tile;
    tile.set_zero();
    tile.set_bit(10, 13, true);
    tile.set_bit(20, 1, true);
//    tile.set_bit(0, 0, true);
//    tile.set_bit(63, 3, true);

    cout << tile.dump() << endl;

    size_t min_x, max_x, min_y, max_y;
    boundaries(tile, min_x, max_x, min_y, max_y);
    cout << "x = (" << min_x << "," << max_x << ")" << endl;
    cout << "y = (" << min_y << "," << max_y << ")" << endl;

#if 0
    size_t x, y;
    if (find_bit(x, y, tile)){
        cout << "(" << x << "," << y << ")" << endl;
    }
#endif
#endif


#if 0
    BinaryMatrixBase<BinaryTile_SSE42> r0(100, 21);
    r0.set_ones();

    cout << r0.dump() << endl;
    cout << r0.tile(0, 2).dump() << endl;
    cout << r0.tile(1, 2).dump() << endl;
#endif


#if 0
    BinaryTile_AVX512 m0, r0, rL;
    m0.set_ones();
    r0.set_zero();
//    r0.set_bit(0, 2, true);

    rL.set_zero();
    rL.set_bit(0, 10, true);
    cout << rL.dump() << endl;


    bool changed = waterfill_touch_right(m0, r0, rL);
    cout << "changed = " << changed << endl;
    cout << r0.dump() << endl;
#endif

#if 0
    BinaryTile_Default m0, r0;
    m0.set_ones();
    r0.set_zero();
    r0.set_bit(4, 0, 1);

    bool changed = waterfill_touch_top(m0, r0, 16);
    cout << "changed = " << changed << endl;
    cout << r0.dump() << endl;
#endif




#if 0


    BinaryTile_AVX512 tile;
    tile.set_zero();
    tile.set_bit(10, 11, 1);
    cout << tile.dump() << endl;
    tile.set_bit(10, 11, 0);
    cout << tile.dump() << endl;
#endif

#if 0
    BinaryTile_AVX512 tile;
    for (size_t c = 0; c < 64; c++){
        tile.row(c) = -1;
    }

    uint64_t popcount, sum_x, sum_y;
    popcount = popcount_sumcoord(sum_x, sum_y, tile);

    cout << popcount << endl;
    cout << sum_x << endl;
    cout << sum_y << endl;
#endif

#if 0
    __m512i popcount;
    __m512i indexsum;
    popcount = popcount_indexsum(indexsum, _mm512_set1_epi64(-1));

//    cout << popcount << " " << indexsum << endl;
    print_u64(popcount);
    print_u64(indexsum);
#endif

#if 0
    __m512i r0 = _mm512_set1_epi64(0x0123456789abcdef);
    print_u64(bit_reverse0(r0));
    print_u64(bit_reverse0(r0));
#endif

#if 0
    __m128i r0 = _mm_set1_epi64x(0x3b);
    cout << dump8(*(uint8_t*)&r0) << endl;

    r0 = _mm_gf2p8affine_epi64_epi8(r0, _mm_set1_epi64x(0x8040201008040201), 0);
//    r0 = _mm_gf2p8affine_epi64_epi8(r0, _mm_set1_epi64x(0x0102040810204080), 0);

    cout << dump8(*(uint8_t*)&r0) << endl;
#endif

#if 0
    BinaryTile_AVX512 m, x;
    m.set_zero();
    x.set_zero();
    for (size_t c = 0; c < 64; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
    m.row( 8) = 0b1111111101011011;
//    m.row(10) = 0;
    m.row(12) = 0b1111110111011111;
    cout << m.dump() << endl;
    cout << "-------------" << endl;

    x.row(63) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif


#if 0
    __m512i r0 = _mm512_setr_epi64(10, 11, 12, 13, 14, 15, 16, 17);

    print_u64(r0);
    r0 = _mm512_permutexvar_epi64(_mm512_set1_epi64(7), r0);
    print_u64(r0);
#endif

#if 0
    {
        __m512i r0 = _mm512_setr_epi64(0, 0, 0, 0, 1, 1, 1, 1);
        __m512i r1 = _mm512_setr_epi64(0, 0, 1, 1, 0, 0, 1, 1);
        __m512i r2 = _mm512_setr_epi64(0, 1, 0, 1, 0, 1, 0, 1);

        __m512i s, x;
        s = _mm512_xor_si512(r1, r2);
        s = _mm512_and_si512(s, r2);
        x = _mm512_or_si512(r0, s);

        print_u64(x);
    }
    {
        __m512i r0 = _mm512_setr_epi64(0, 0, 0, 0, 1, 1, 1, 1);
        __m512i r1 = _mm512_setr_epi64(0, 0, 1, 1, 0, 0, 1, 1);
        __m512i r2 = _mm512_setr_epi64(0, 1, 0, 1, 0, 1, 0, 1);

        r0 = _mm512_ternarylogic_epi64(r0, r1, r2, 0b11110010);

        print_u64(r0);
    }
#endif


#if 0
    __m512i x;
    uint8_t s[64];
    for (size_t c = 0; c < 64; c++){
        s[c] = (uint8_t)c;
    }
    memcpy(&x, s, 64);

    print_u8(x);
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );
    print_u8(x);
#endif

#if 0
    __m256i x = _mm256_setr_epi64x(10, 11, 12, 13);
    print_u64(x);
    print_u64(_mm256_permute4x64_epi64(x, 255));
#endif

#if 0
    BinaryTile_AVX2 m, x;
    m.set_zero();
    x.set_zero();
    for (size_t c = 0; c < 16; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
    m.row( 8) = 0b1111111101011011;
//    m.row(10) = 0;
    m.row(12) = 0b1111110111011111;
    cout << m.dump() << endl;
    cout << "-------------" << endl;

    x.row(15) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif
#if 0
    BinaryTile_SSE42 m, x;
    m.set_zero();
    x.set_zero();
#if 0
    m.row( 0) = 1;
    m.row( 1) = 1;
    m.row( 2) = 1;
    m.row( 3) = 0;
    m.row( 4) = 1;
    m.row( 5) = 1;
    m.row( 6) = 1;
    m.row( 7) = 1;
#endif
#if 1
    for (size_t c = 0; c < 8; c++){
        m.row(c) = -1;
    }
    m.row( 0) = 0b1111111011111011;
    m.row( 1) = 0b1110111011111011;
    m.row( 2) = 0b1011111011011111;
    m.row( 3) = 0b1110101011010011;
    m.row( 4) = 0b1111111001001011;
//    m.row( 5) = 0;
#endif
    cout << m.dump() << endl;
    cout << "-------------" << endl;

//    x.row(3) = 1;
    x.row(7) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif

//    __m256i v = _mm256_set1_epi64x(123);
//    cout << _mm256_testz_si256(v, v) << endl;


#if 0
    __m128i x = _mm_set_epi64x(10, 20);
    print_u64(x);
    print_u64(_mm_shuffle_epi32(x, 78));
#endif

#if 0
    BinaryTile_Default m, x;
    m.set_zero();
    x.set_zero();
    m.row(0) = 0b1111111011111011;
    m.row(1) = 0b1111111001001011;
    m.row(2) = 0b1111111101011011;
    m.row(3) = 0b1111110111011111;
    cout << m.dump() << endl;

    x.row(0) = 0b1000000000000000;

    waterfill_expand(m, x);
    cout << x.dump() << endl;
#endif


#if 0
    uint64_t m0 = 0b11011110;
    uint64_t x0 = 0b00000011;
    expand_forward(x0, m0);
    print64(m0);
    print64(x0);
#endif

#if 0
    uint64_t m0 = 0b11011110;
    uint64_t m1 = 0b11001111;
    uint64_t x0 = 0b00000010;
    uint64_t x1 = 0b00000000;

    print64(m0);
    print64(m1);
    print64(x0);
    print64(x1);

    expand(x0, m0);
    touch_vertical(x1, x0, m1);

    print64(m0);
    print64(m1);
    print64(x0);
    print64(x1);
#endif

}


std::set<std::string> read_name(
    Logger& logger,
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
