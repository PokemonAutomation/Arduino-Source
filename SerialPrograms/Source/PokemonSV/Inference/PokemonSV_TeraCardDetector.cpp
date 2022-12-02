/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QString>
//#include "Common/Qt/StringToolsQt.h"
//#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
//#include "CommonFramework/OCR/OCR_StringNormalization.h"
//#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_TeraCardDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraCardReader::TeraCardReader(Color color)
    : m_color(color)
    , m_top(0.15, 0.13, 0.40, 0.03)
    , m_bottom_left(0.15, 0.80, 0.10, 0.06)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(0.75, 0.67, 0.10, 0.05)
    , m_cursor(0.135, 0.25, 0.05, 0.25)
    , m_stars(0.500, 0.555, 0.310, 0.070)
{}
void TeraCardReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top);
    items.add(m_color, m_bottom_left);
    items.add(m_color, m_bottom_right);
    items.add(m_color, m_label);
    items.add(m_color, m_cursor);
    items.add(m_color, m_stars);
}
bool TeraCardReader::detect(const ImageViewRGB32& screen) const{
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.354167, 0.345833, 0.3})){
        return false;
    }

    ImageStats bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
//    cout << bottom_left.average << bottom_left.stddev << endl;
    if (!is_solid(bottom_left, {0.354167, 0.345833, 0.3})){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << bottom_right.stddev << endl;
    if (!is_solid(bottom_right, {0.354167, 0.345833, 0.3})){
        return false;
    }

    if (euclidean_distance(top.average, bottom_left.average) > 20){
        return false;
    }
    if (euclidean_distance(top.average, bottom_right.average) > 20){
        return false;
    }
    if (euclidean_distance(bottom_left.average, bottom_right.average) > 20){
        return false;
    }

    ImageStats label = image_stats(extract_box_reference(screen, m_label));
//    cout << label.average << label.stddev << endl;
    if (!is_solid(label, {0.370075, 0.369063, 0.260862})){
        return false;
    }

    GradientArrowDetector arrow_detector(COLOR_RED, GradientArrowType::RIGHT, m_cursor);
    if (!arrow_detector.detect(screen)){
        return false;
    }

    return true;
}
size_t TeraCardReader::stars(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 cropped = extract_box_reference(screen, m_stars);

    {
        ImageStats background = image_stats(extract_box_reference(screen, ImageFloatBox{0.55, 0.62, 0.20, 0.03}));
        Color background_average = background.average.round();

        PackedBinaryMatrix matrix = compress_rgb32_to_binary_euclidean(cropped, (uint32_t)background_average, 100);

        matrix.invert();
//        cout << matrix.dump() << endl;
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(100);
        WaterfillObject object;
        size_t count = 0;
        while (iter->find_next(object, false)){
//            extract_box_reference(cropped, object).save("test-" + std::to_string(count) + ".png");
            count++;
        }
        if (1 <= count && count <= 7){
            return count;
        }
    }

    return 0;
}



TeraLobbyReader::TeraLobbyReader(Color color)
    : m_color(color)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(0.75, 0.67, 0.10, 0.05)
    , m_cursor(0.135, 0.25, 0.05, 0.25)
//    , m_stars(0.500, 0.555, 0.310, 0.070)
    , m_timer(0.175, 0.180, 0.100, 0.080)
    , m_code(0.310, 0.180, 0.200, 0.080)
    , m_player1_spinner(0.157, 0.645, 0.037, 0.060)
    , m_player2_spinner(0.157, 0.715, 0.037, 0.060)
    , m_player3_spinner(0.157, 0.790, 0.037, 0.060)
    , m_player0_mon(0.425, 0.575, 0.037, 0.060)
    , m_player1_mon(0.425, 0.645, 0.037, 0.060)
    , m_player2_mon(0.425, 0.715, 0.037, 0.060)
    , m_player3_mon(0.425, 0.790, 0.037, 0.060)
{}
void TeraLobbyReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_right);
    items.add(m_color, m_label);
    items.add(m_color, m_cursor);
    items.add(m_color, m_timer);
    items.add(m_color, m_code);
//    items.add(m_color, m_player1_spinner);
//    items.add(m_color, m_player2_spinner);
//    items.add(m_color, m_player3_spinner);
    items.add(m_color, m_player0_mon);
    items.add(m_color, m_player1_mon);
    items.add(m_color, m_player2_mon);
    items.add(m_color, m_player3_mon);
}
bool TeraLobbyReader::detect(const ImageViewRGB32& screen) const{
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << bottom_right.stddev << endl;
    if (!is_solid(bottom_right, {0.354167, 0.345833, 0.3})){
        return false;
    }

    ImageStats label = image_stats(extract_box_reference(screen, m_label));
//    cout << label.average << label.stddev << endl;
    if (!is_solid(label, {0.370075, 0.369063, 0.260862})){
        return false;
    }

    GradientArrowDetector arrow_detector(COLOR_RED, GradientArrowType::RIGHT, m_cursor);
    if (!arrow_detector.detect(screen)){
        return false;
    }

    return true;
}

uint8_t TeraLobbyReader::total_players(const ImageViewRGB32& screen) const{
    ImageStats player0 = image_stats(extract_box_reference(screen, m_player0_mon));
    ImageStats player1 = image_stats(extract_box_reference(screen, m_player1_mon));
    ImageStats player2 = image_stats(extract_box_reference(screen, m_player2_mon));
    ImageStats player3 = image_stats(extract_box_reference(screen, m_player3_mon));
//    cout << player0.average << player0.stddev << endl;
//    cout << player1.average << player1.stddev << endl;
//    cout << player2.average << player2.stddev << endl;
//    cout << player3.average << player3.stddev << endl;

    uint8_t total = 0;
    if (player0.stddev.sum() > 80) total++;
    if (player1.stddev.sum() > 80) total++;
    if (player2.stddev.sum() > 80) total++;
    if (player3.stddev.sum() > 80) total++;
    return std::max(total, (uint8_t)1);
}

std::string TeraLobbyReader::raid_code(Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

#if 1

    uint32_t THRESHOLD = 0xff5f5f5f;

    ImageViewRGB32 image = extract_box_reference(screen, m_code);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, 0xff000000, THRESHOLD, true);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, THRESHOLD);

    std::map<size_t, std::string> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(50);
        WaterfillObject object;
        while (iter->find_next(object, true)){
//                cout << object.packed_matrix()->dump() << endl;
            ImageViewRGB32 character = extract_box_reference(filtered, object);
            ImageRGB32 padded = pad_image(character, character.width(), 0xffffffff);

            std::string raw = OCR::ocr_read(Language::English, padded);
            map.emplace(object.min_x, std::move(raw));
        }
    }

//    cout << map.size() << endl;

    static const std::map<char, char> SUBSTITUTIONS{
        {'I', '1'},
        {'l', '1'},
        {'O', '0'},
        {'Z', 'S'},
        {'\\', 'V'},
    };

    std::string raw;
    std::string normalized;
    for (const auto& item : map){
        if (item.second.empty()){
            continue;
        }
        if ((uint8_t)item.second.back() < (uint8_t)32){
            raw += item.second.substr(0, item.second.size() - 1);
        }else{
            raw += item.second;
        }

        char ch = item.second[0];

        //  Upper case.
        if ('a' <= ch && ch <= 'z'){
            ch -= 'a' - 'A';
        }

        //  Character substitution.
        auto iter = SUBSTITUTIONS.find(ch);
        if (iter != SUBSTITUTIONS.end()){
            ch = iter->second;
        }

        normalized += ch;
    }

    std::string log = "Code OCR: \"" + raw + "\" -> \"" + normalized + "\"";
    if (normalized.size() != 4 && normalized.size() != 6){
        logger.log(log, COLOR_RED);
        return "";
    }

    logger.log(log, COLOR_BLUE);
    return normalized;

#else

    ImageRGB32 filtered = to_blackwhite_rgb32_range(extract_box_reference(screen, m_code), 0xff000000, 0xff7f7f7f, true);
//    filtered.save("test.png");

    std::string raw = OCR::ocr_read(Language::English, filtered);
    if (!raw.empty() && raw.back() == '\n'){
        raw.pop_back();
    }

    //  KD normalization.
    QString qstr = QString::fromStdString(raw);
    qstr = qstr.normalized(QString::NormalizationForm_KD);

    static const std::map<char32_t, char32_t> SUBSTITUTIONS{
        {'I', '1'},
        {'l', '1'},
        {'O', '0'},
        {'Z', 'S'},
    };

    std::u32string str32;
    for (char32_t ch : qstr.toStdU32String()){
        if (!QChar::isLetterOrNumber(ch)){
            continue;
        }

        ch = QChar::toUpper(ch);
        auto iter = SUBSTITUTIONS.find(ch);
        if (iter != SUBSTITUTIONS.end()){
            ch = iter->second;
        }
        str32 += ch;
    }

    //  Strip non-alphanumeric.
    std::string normalized = to_utf8(str32);

    std::string log = "Code OCR: \"" + raw + "\" -> \"" + normalized + "\"";


    if (normalized.size() != 4 && normalized.size() != 6){
        logger.log(log, COLOR_RED);
        return "";
    }

    logger.log(log, COLOR_BLUE);
    return normalized;
#endif
}



TeraLobbyReadyWaiter::TeraLobbyReadyWaiter(Color color, uint8_t desired_players)
    : TeraLobbyReader(color)
    , VisualInferenceCallback("TeraLobbyReadyWaiter")
    , m_desired_players(desired_players)
    , m_last_known_player_count(-1)
{}

void TeraLobbyReadyWaiter::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyReadyWaiter::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!detect(frame)){
        return false;
    }
    uint8_t players = total_players(frame);
    m_last_known_player_count.store(players);
    return players >= m_desired_players;
}




















}
}
}
