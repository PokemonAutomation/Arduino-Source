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
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;
using namespace Kernels::Waterfill;


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












void TestProgramComputer::program(ProgramEnvironment& env){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonSwSh;

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
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
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
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
    if (objects.size() != 0){
        PA_THROW_StringException("Failed to find exactly one object in resource.");
    }
#endif




#if 0
    QImage image(QImage(RESOURCE_PATH() + "PokemonSwSh/ExclamationMark.png"));
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
    std::vector<WaterFillObject> objects = find_objects(matrix, 200, false);
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
    std::vector<WaterFillObject> objects = find_objects(matrix, 200, true);

    cout << objects.size() << endl;
    size_t c = 0;
    for (const WaterFillObject& object : objects){
        ImagePixelBox box(object.min_x, object.min_y, object.max_x, object.max_y);
        cout << object.min_x << ", " << object.min_y << ", " << object.max_x << ", " << object.max_y << endl;
        extract_box(image, box).save("test-" + QString::number(c++) + ".png");
    }

    WaterFillObject& object = objects[1];
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

    std::vector<WaterFillObject> objects1 = find_objects(matrix, 1, false);
    cout << "objects = " << objects1.size() << endl;
    std::multimap<uint64_t, WaterFillObject> sorted1;
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
        std::vector<WaterFillObject> objects1 = find_objects(matrix1, 10, false);
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
        std::vector<WaterFillObject> objects1 = find_objects(matrix1, 1, false);
        cout << "objects = " << objects1.size() << endl;
        std::multimap<uint64_t, WaterFillObject> sorted1;
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
    WaterFillObject object;
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
    std::vector<WaterFillObject> objects1 = find_objects(matrix1);
    cout << "objects = " << objects1.size() << endl;



    std::multimap<uint64_t, FillGeometry> sorted0;
    for (const auto& item : objects0){
        sorted0.emplace(item.area, item);
    }

    std::multimap<uint64_t, WaterFillObject> sorted1;
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
        std::vector<WaterFillObject> objects = find_objects(matrix1);
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


//    WaterFillObject object;
//    find_object(matrix, object, 0, 0);
    std::vector<WaterFillObject> objects = find_objects(matrix);

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








}
