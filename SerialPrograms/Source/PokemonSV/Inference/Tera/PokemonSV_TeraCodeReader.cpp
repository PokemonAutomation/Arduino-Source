/*  Tera Code Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonSV_TeraCodeReader.h"

//#define PA_ENABLE_CODE_DEBUG

#ifdef PA_ENABLE_CODE_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class CharacterTemplates{
    CharacterTemplates()
        : ENG_5(RESOURCE_PATH() + "PokemonSV/TeraCode/TeraCode-5-eng.png")
        , ENG_S(RESOURCE_PATH() + "PokemonSV/TeraCode/TeraCode-S-eng.png")
        , CHI_5(RESOURCE_PATH() + "PokemonSV/TeraCode/TeraCode-5-chi.png")
        , CHI_S(RESOURCE_PATH() + "PokemonSV/TeraCode/TeraCode-S-chi.png")
    {}

public:
    static const CharacterTemplates& instance(){
        static CharacterTemplates templates;
        return templates;
    }

    ImageMatch::ExactImageMatcher ENG_5;
    ImageMatch::ExactImageMatcher ENG_S;
    ImageMatch::ExactImageMatcher CHI_5;
    ImageMatch::ExactImageMatcher CHI_S;
};

void preload_code_templates(){
    CharacterTemplates::instance();
}





char read_5S(
    const ImageViewRGB32& image,
    const Kernels::Waterfill::WaterfillObject& object, char OCR_result
){
    //  5 and S are commonly misread as each other. Here we try an extra step to
    //  distinguish them.

//    cout << matrix.dump() << endl;

//    cout << (object.center_of_gravity_x() - object.min_x) / object.width() << ", " << (object.center_of_gravity_y() - object.min_y) / object.height() << endl;

    const CharacterTemplates& templates = CharacterTemplates::instance();

    ImageViewRGB32 cropped = extract_box_reference(image, object);
//    static int count = 0;
//    cropped.save("testC-" + std::to_string(count++) + ".png");

    double best_rmsd = 1000;
    char best_result = OCR_result;

    double rmsd;

    rmsd = templates.ENG_5.rmsd(cropped);
//    cout << "eng-5 = " << rmsd << endl;
    if (best_rmsd > rmsd){
        best_rmsd = rmsd;
        best_result = '5';
    }

    rmsd = templates.ENG_S.rmsd(cropped);
//    cout << "eng-S = " << rmsd << endl;
    if (best_rmsd > rmsd){
        best_rmsd = rmsd;
        best_result = 'S';
    }

    rmsd = templates.CHI_5.rmsd(cropped);
//    cout << "chi-5 = " << rmsd << endl;
    if (best_rmsd > rmsd){
        best_rmsd = rmsd;
        best_result = '5';
    }

    rmsd = templates.CHI_S.rmsd(cropped);
//    cout << "chi-S = " << rmsd << endl;
    if (best_rmsd > rmsd){
//        best_rmsd = rmsd;
        best_result = 'S';
    }

    return best_result;

#if 0
    PackedBinaryMatrix matrix = object.packed_matrix();

    size_t width = matrix.width();
    size_t height = matrix.height();

    for (size_t row = 0; row < height; row++){
        size_t top_black_pixels = 0;
        for (size_t c = 0; c < width; c++){
            if (matrix.get(c, 0)){
                top_black_pixels++;
            }
        }
        double ratio = (double)top_black_pixels / width;
        cout << "top_black_pixels = " << top_black_pixels << ", " << ratio << endl;
        if (ratio > 0.70){
            return '5';
        }
        if (ratio > 0.10){
            return 'S';
        }
    }

    return OCR_result;
#endif
}


struct WaterfillOCRResult{
    Kernels::Waterfill::WaterfillObject object;
    std::string ocr;
};


std::vector<WaterfillOCRResult> waterfill_OCR(
    const ImageViewRGB32& image,
    uint32_t threshold
){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, true, 0xff000000, threshold);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, threshold);

    std::map<size_t, WaterfillObject> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (map.size() < 16 && iter->find_next(object, true)){
            map.emplace(object.min_x, std::move(object));
        }
    }

    std::vector<WaterfillOCRResult> ret;
    for (auto& item : map){
        ret.emplace_back(WaterfillOCRResult{std::move(item.second), ""});
    }

    GlobalThreadPools::realtime_inference().run_in_parallel(
        [&](size_t index){
            WaterfillObject& object = ret[index].object;
            ImageRGB32 cropped = extract_box_reference(filtered, object).copy();
            PackedBinaryMatrix tmp(object.packed_matrix());
            filter_by_mask(tmp, cropped, Color(0xffffffff), true);
            ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
            ret[index].ocr = OCR::ocr_read(Language::English, padded);
        },
        0, ret.size()
    );

#ifdef PA_ENABLE_CODE_DEBUG
    static size_t count = 0;
    for (size_t c = 0; c < ret.size(); c++){
        cout << ret[c].ocr << endl;
        ImageRGB32 cropped = extract_box_reference(filtered, ret[c].object).copy();
        filter_by_mask(ret[c].object.packed_matrix(), cropped, Color(0xffffffff), true);
        ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
        padded.save("letter-" + std::to_string(count) + ".png");
        count++;
    }
#endif

    return ret;
}


int16_t read_raid_timer(Logger& logger, const ImageViewRGB32& image){
    std::vector<WaterfillOCRResult> characters = waterfill_OCR(image, 0xff7f7f7f);

//    cout << "map.size() = " << map.size() << endl;
//    for (auto& item : map){
//        cout << item.second << endl;
//    }

    static const std::map<char, char> SUBSTITUTIONS{
        {'I', '1'},
        {'i', '1'},
        {'l', '1'},
        {'O', '0'},
        {'S', '5'},
        {'s', '5'},
        {'/', '7'},
        {']', '1'},
    };

    std::string raw;
    std::string normalized;
    for (const auto& item : characters){
        const std::string& ocr = item.ocr;
        if (ocr.empty()){
            continue;
        }
        if ((uint8_t)ocr.back() < (uint8_t)32){
            raw += ocr.substr(0, ocr.size() - 1);
        }else{
            raw += ocr;
        }

        char ch = ocr[0];

        //  Character substitution.
        auto iter = SUBSTITUTIONS.find(ch);
        if (iter != SUBSTITUTIONS.end()){
            ch = iter->second;
        }

        if ('0' <= ch && ch <= '9'){
            normalized += ch;
        }
    }

    std::string log = "Timer OCR: \"" + raw + "\" -> \"" + normalized + "\"";
    if (normalized.size() != 3){
        logger.log(log, COLOR_RED);
        return -1;
    }

    logger.log(log, COLOR_BLUE);

    int16_t ret = 60 * (normalized[0] - '0');
    ret += 10 * (normalized[1] - '0');
    ret += normalized[2] - '0';

    return ret;
}


std::string read_raid_code(Logger& logger, const ImageViewRGB32& image){
    std::vector<uint32_t> filters{
        0xff5f5f5f,
        0xff7f7f7f,
    };

    for (uint32_t filter : filters){
        std::vector<WaterfillOCRResult> characters = waterfill_OCR(image, filter);

        static const std::map<char, char> SUBSTITUTIONS{
            {'I', '1'},
            {'i', '1'},
            {'l', '1'},
            {'O', '0'},
            {'Z', 'S'},
            {'\\', 'V'},
            {'/', '7'},
            {']', '1'},
            {')', 'J'},
            {'(', 'K'},
            {'[', 'C'},
        };

        bool contains_letters = false;

        std::string raw;
        std::string normalized;
        for (const auto& item : characters){
            const std::string& ocr = item.ocr;
            if (ocr.empty()){
                continue;
            }
            if ((uint8_t)ocr.back() < (uint8_t)32){
                raw += ocr.substr(0, ocr.size() - 1);
            }else{
                raw += ocr;
            }

            char ch = ocr[0];

            //  Upper case.
            if ('a' <= ch && ch <= 'z'){
                ch -= 'a' - 'A';
            }

            //  Character substitution.
            auto iter = SUBSTITUTIONS.find(ch);
            if (iter != SUBSTITUTIONS.end()){
                ch = iter->second;
            }

            //  Distinguish 5 and S.
            switch (ch){
            case '5':
            case 'S':
                ch = read_5S(image, item.object, ch);
            }

            contains_letters |= 'A' <= ch && ch <= 'Z';

            normalized += ch;
        }

        std::string log = "Code OCR: \"" + raw + "\" -> \"" + normalized + "\"";
        size_t length = normalized.size();
        if ((contains_letters && length == 6) ||
            (!contains_letters && (length == 4 || length == 6 || length == 8))
        ){
            logger.log(log, COLOR_BLUE);
            return normalized;
        }
        logger.log(log, COLOR_RED);
    }

    return "";
}



}
}
}
