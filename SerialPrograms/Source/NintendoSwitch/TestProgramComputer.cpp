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
#include "CommonFramework/AudioPipeline/TimeSampleWriter.h"
#include "CommonFramework/AudioPipeline/TimeSampleBuffer.h"
#include "CommonFramework/AudioPipeline/TimeSampleBufferReader.h"
#include "CommonFramework/AudioPipeline/AudioNormalization.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"

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


using Type = float;














void TestProgramComputer::program(ProgramEnvironment& env){
    using namespace Kernels;
    using namespace NintendoSwitch::PokemonSwSh;
    using namespace Pokemon;


    BlackScreenOverWatcher black_screen1(COLOR_RED, {0.20, 0.95, 0.60, 0.03}, 20);

    black_screen1.process_frame(QImage("screenshot-20220221-232325966395.png"), std::chrono::system_clock::now());


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
