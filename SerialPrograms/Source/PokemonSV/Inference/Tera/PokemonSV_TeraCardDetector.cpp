/*  Tera Card Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <list>
#include <map>
#include "Common/Cpp/PrettyPrint.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
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
    , m_label(CARD_LABEL_BOX())
    , m_cursor(0.135, 0.25, 0.05, 0.25)
    , m_stars(0.500, 0.555, 0.310, 0.070)
    , m_tera_type(color)
    , m_silhouette(color)
{}
void TeraCardReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top);
    items.add(m_color, m_bottom_left);
    items.add(m_color, m_bottom_right);
    items.add(m_color, m_label);
    items.add(m_color, m_cursor);
    items.add(m_color, m_stars);
    m_tera_type.make_overlays(items);
    m_silhouette.make_overlays(items);
}

const ImageFloatBox& TeraCardReader::CARD_LABEL_BOX(){
    static ImageFloatBox box(0.75, 0.67, 0.10, 0.05);
    return box;
}
bool TeraCardReader::is_card_label(const ImageViewRGB32& screen){
    ImageStats label = image_stats(extract_box_reference(screen, CARD_LABEL_BOX()));
    return is_solid(label, {0.370075, 0.369063, 0.260862})              //  Paldea Card
        || is_solid(label, {0.258888, 0.369491, 0.371621}, 0.15, 15);   //  Kitakami Card
}

bool TeraCardReader::detect(const ImageViewRGB32& screen){
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.354167, 0.345833, 0.3})){
//        cout << "bad 1" << endl;
        return false;
    }

    ImageStats bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
//    cout << bottom_left.average << bottom_left.stddev << endl;
    if (!is_solid(bottom_left, {0.354167, 0.345833, 0.3})){
//        cout << "bad 2" << endl;
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << bottom_right.stddev << endl;
    if (!is_solid(bottom_right, {0.354167, 0.345833, 0.3})){
//        cout << "bad 3" << endl;
        return false;
    }

    if (euclidean_distance(top.average, bottom_left.average) > 20){
//        cout << "euclidean_distance 0" << endl;
        return false;
    }
    if (euclidean_distance(top.average, bottom_right.average) > 20){
//        cout << "euclidean_distance 1" << endl;
        return false;
    }
    if (euclidean_distance(bottom_left.average, bottom_right.average) > 20){
//        cout << "euclidean_distance 2" << endl;
        return false;
    }

//    ImageStats label = image_stats(extract_box_reference(screen, m_label));
//    extract_box_reference(screen, m_label).save("test.png");
//    cout << label.average << label.stddev << endl;
    if (!is_card_label(screen)){
        return false;
    }

    GradientArrowDetector arrow_detector(COLOR_RED, GradientArrowType::RIGHT, m_cursor);
    if (!arrow_detector.detect(screen)){
        return false;
    }

    return true;
}
uint8_t TeraCardReader::stars(
    Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 cropped = extract_box_reference(screen, m_stars);

    ImageViewRGB32 background = extract_box_reference(screen, ImageFloatBox{0.55, 0.62, 0.20, 0.03});
//        background.save("background.png");
    ImageStats background_stats = image_stats(background);
    Color background_average = background_stats.average.round();

    //  Iterate through multiple distance filters and find how many
    //  possible stars are in each one. Then do a majority vote.
    const std::vector<double> DISTANCES{70, 80, 90, 100, 110, 120, 130};

    std::map<size_t, size_t> count_map;

    for (double distance : DISTANCES){
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_euclidean(cropped, (uint32_t)background_average, distance);

        matrix.invert();
//        cout << matrix.dump() << endl;
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(100);
        WaterfillObject object;

        std::list<ImagePixelBox> objects;
        while (iter->find_next(object, false)){
            //  Attempt to merge with existing objects.
            ImagePixelBox current(object);
            bool changed;
            do{
                changed = false;
                for (auto iter1 = objects.begin(); iter1 != objects.end();){
                    if (current.overlaps_with(*iter1)){
                        changed = true;
                        current.merge_with(*iter1);
                        objects.erase(iter1);
                        break;
                    }else{
                        ++iter1;
                    }
                }
            }while (changed);
            objects.emplace_back(std::move(current));
        }

#if 0
        static size_t count = 0;
        for (const ImagePixelBox& obj : objects){
            extract_box_reference(cropped, obj).save("test-" + std::to_string(count++) + ".png");
        }
        cout << "objects.size() = " << objects.size() << endl;
#endif


        count_map[objects.size()]++;
    }

    count_map.erase(0);

    if (count_map.empty()){
        dump_image(logger, info, "ReadStarsFailed", screen);
        return 0;
    }

    auto best = count_map.begin();
    for (auto iter = count_map.begin(); iter != count_map.end(); ++iter){
        if (iter->first == 0){
            continue;
        }
        if (iter->second > best->second){
            best = iter;
        }
    }

    size_t stars = best->first;

    if (1 <= stars && stars <= 7){
        return (uint8_t)stars;
    }

    dump_image(logger, info, "ReadStarsFailed", screen);
    return 0;
}
std::string TeraCardReader::tera_type(
    Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
) const{
    ImageMatch::ImageMatchResult type = m_tera_type.read(screen);
    type.log(logger, 100);
    std::string best_type;
    if (!type.results.empty()){
        best_type = type.results.begin()->second;
    }
    if (best_type.empty()){
        dump_image(logger, info, "ReadTypeFailed", screen);
    }else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        dump_debug_image(logger, "PokemonSV/TeraRoller/" + best_type, "", screen);
    }

    return best_type;
}
std::set<std::string> TeraCardReader::pokemon_slug(
    Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
) const{
    ImageMatch::ImageMatchResult silhouette = m_silhouette.read(screen);
    silhouette.log(logger, 110);
//    std::string best_silhouette;
//    if (!silhouette.results.empty()){
//        best_silhouette = silhouette.results.begin()->second;
//    }
    if (silhouette.results.empty()){
        dump_image(logger, info, "ReadSilhouetteFailed", screen);
    }
//    else if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
//        dump_debug_image(logger, "PokemonSV/TeraRoller/" + best_silhouette, "", screen);
//    }

    std::set<std::string> results;
    for (const auto& item : silhouette.results){
        results.insert(std::move(item.second));
    }

    return results;
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



TeraLobbyReader::TeraLobbyReader(Logger& logger, Color color)
    : m_logger(logger)
    , m_color(color)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(TeraCardReader::CARD_LABEL_BOX())
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
bool TeraLobbyReader::detect(const ImageViewRGB32& screen){
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << bottom_right.stddev << endl;
    if (!is_solid(bottom_right, {0.354167, 0.345833, 0.3})){
        return false;
    }

//    ImageStats label = image_stats(extract_box_reference(screen, m_label));
//    cout << label.average << label.stddev << endl;
    if (!TeraCardReader::is_card_label(screen)){
        return false;
    }

    GradientArrowDetector arrow_detector(COLOR_RED, GradientArrowType::RIGHT, m_cursor);
    if (!arrow_detector.detect(screen)){
        return false;
    }

    if (seconds_left(m_logger, screen) < 0){
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

int16_t TeraLobbyReader::seconds_left(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_timer);
    return read_raid_timer(logger, image);
}
std::string TeraLobbyReader::raid_code(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 image = extract_box_reference(screen, m_code);
    return read_raid_code(logger, image);
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
                if ((unsigned)ch < 32){
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















}
}
}
