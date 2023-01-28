/*  Tera Card Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <array>
#include <map>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
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
//#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_TeraCodeReader.h"
#include "PokemonSV_TeraCardDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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






std::string TeraLobbyNameMatchResult::to_str() const{
    std::string ret;
    ret += "\"" + raw_ocr;
    if (ret.back() == '\n'){
        ret.pop_back();
    }
//    ret += "\" -> \"" + normalized_ocr + "\" == \"";
    ret += "\" -> \"";
    ret += entry.name;
    ret += "\" (";
    if (exact_match){
        ret += "exact match, ";
    }else{
        ret += "partial match, ";
    }
    ret += "log10p = " + tostr_default(log10p) + ")";
    return ret;
}



TeraLobbyReader::TeraLobbyReader(Logger& logger, AsyncDispatcher& dispatcher, Color color)
    : m_logger(logger)
    , m_dispatcher(dispatcher)
    , m_color(color)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(0.75, 0.67, 0.10, 0.05)
    , m_cursor(0.135, 0.25, 0.05, 0.25)
//    , m_stars(0.500, 0.555, 0.310, 0.070)
    , m_timer(0.175, 0.180, 0.100, 0.080)
    , m_code(0.310, 0.180, 0.200, 0.080)
{
    for (size_t c = 0; c < 4; c++){
        m_player_spinner[c] = ImageFloatBox{0.157, 0.575 + 0.070*c, 0.037, 0.060};
        m_player_name[c] = ImageFloatBox{0.200, 0.575 + 0.070*c, 0.095, 0.060};
        m_player_mon[c] = ImageFloatBox{0.425, 0.575 + 0.070*c, 0.037, 0.060};
        m_player_ready[c] = ImageFloatBox{0.465, 0.575 + 0.070*c, 0.037, 0.060};
    }
}
void TeraLobbyReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_right);
    items.add(m_color, m_label);
    items.add(m_color, m_cursor);
    items.add(m_color, m_timer);
    items.add(m_color, m_code);
    for (size_t c = 0; c < 4; c++){
        items.add(m_color, m_player_spinner[c]);
        items.add(m_color, m_player_name[c]);
        items.add(m_color, m_player_mon[c]);
        items.add(m_color, m_player_ready[c]);
    }
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

    if (seconds_left(m_logger, m_dispatcher, screen) < 0){
        return false;
    }

    return true;
}

uint8_t TeraLobbyReader::total_players(const ImageViewRGB32& screen) const{
    uint8_t total = 0;
    for (size_t c = 0; c < 4; c++){
        ImageStats player = image_stats(extract_box_reference(screen, m_player_mon[c]));
        if (player.stddev.sum() > 80){
            total++;
        }
    }
    return std::max(total, (uint8_t)1);
}
#if 0
uint8_t TeraLobbyReader::ready_players(const ImageViewRGB32& screen) const{
    uint8_t total = 0;
    for (size_t c = 0; c < 4; c++){
        ImageStats player = image_stats(extract_box_reference(screen, m_player_ready[c]));
//        cout << "Player " << c << ": " << player.average << player.stddev << endl;
        if (player.stddev.sum() > 80){
            total++;
        }
    }
    return total;
}
#endif
uint8_t TeraLobbyReader::ready_joiners(const ImageViewRGB32& screen, uint8_t host_players){
    uint8_t total = 0;
    for (size_t c = host_players; c < 4; c++){
        ImageStats player = image_stats(extract_box_reference(screen, m_player_ready[c]));
//        cout << "Player " << c << ": " << player.average << player.stddev << endl;
        if (player.stddev.sum() > 80){
            total++;
        }
    }
    return total;
}

int16_t TeraLobbyReader::seconds_left(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_timer);
    return read_raid_timer(logger, dispatcher, image);
}
std::string TeraLobbyReader::raid_code(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_code);
    return read_raid_code(logger, dispatcher, image);
}

ImageRGB32 filter_name_image(const ImageViewRGB32& image){
    using namespace Kernels::Waterfill;

    const uint32_t COLOR_THRESHOLD = 0xff8f8f8f;

    //  Waterfill the image and throw out everything that touches the border.

    ImageRGB32 filtered(image.width(), image.height());
    filtered.fill(0xffffffff);
//    cout << image.width() << " x " << image.height() << endl;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, COLOR_THRESHOLD);
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(4);
        WaterfillObject object;
        while (iter->find_next(object, true)){
            auto packed = object.packed_matrix();
//            cout << packed->width() << " x " << packed->height() << " : " << object.min_x << " x " << object.min_y << endl;

            //  Touches border.
            if (object.min_x == 0 ||
                object.min_y == 0 ||
                object.max_x == filtered.width() ||
                object.max_y == filtered.height()
            ){
                continue;
            }
            filter_by_mask(
                std::move(packed),
                filtered, object.min_x, object.min_y,
                Color(0xff000000),
                false
            );
        }
    }

//    static int c = 0;
//    filtered.save("filtered" + std::to_string(c++) + ".png");

    return filtered;

//    size_t pixels;
//    return to_blackwhite_rgb32_range(pixels, image, 0xff000000, COLOR_THRESHOLD, true);
}


std::array<std::map<Language, std::string>, 4> TeraLobbyReader::read_names(
    Logger& logger,
    const std::set<Language>& languages,
    const ImageViewRGB32& screen
) const{
    std::array<std::map<Language, std::string>, 4> ret;
    for (size_t c = 0; c < 4; c++){
        ImageStats sprite = image_stats(extract_box_reference(screen, m_player_mon[c]));
        if (sprite.stddev.sum() <= 80){
            continue;
        }
//        logger.log("Reading player " + std::to_string(c) + "'s name...");
        std::string str = "Player " + std::to_string(c) + ": ";
        bool first = true;
        for (Language language : languages){
            ImageViewRGB32 box = extract_box_reference(screen, m_player_name[c]);
            ImageRGB32 filtered = filter_name_image(box);
//            filtered.save("name" + std::to_string(c) + ".png");

            std::string raw;
            for (char ch : OCR::ocr_read(language, filtered)){
                if (ch < 32){
                    continue;
                }
                raw += ch;
            }
            if (!first){
                str += ", ";
            }
            first = false;
            str += language_data(language).code + "=\"" + raw + "\"";
            ret[c][language] = std::move(raw);
        }
        logger.log(str);
    }
    return ret;
}





#if 0
TeraLobbyReadyWaiter::TeraLobbyReadyWaiter(
    Logger& logger, AsyncDispatcher& dispatcher,
    Color color, uint8_t desired_players
)
    : TeraLobbyReader(logger, dispatcher, color)
    , VisualInferenceCallback("TeraLobbyReadyWaiter")
    , m_desired_players(desired_players)
    , m_last_known_total_players(-1)
{}

void TeraLobbyReadyWaiter::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyReadyWaiter::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!detect(frame)){
        return false;
    }
    uint8_t total_players = this->total_players(frame);
    uint8_t ready_players = this->ready_players(frame);
    m_last_known_total_players.store(total_players);
    return ready_players + 1 >= m_desired_players;
}
#endif












}
}
}
