/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <map>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
//#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
//#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ErrorDumper.h"
//#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
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
    ret += entry.name + "\" (log10p = " + tostr_default(log10p) + ")";
    return ret;
}



TeraLobbyReader::TeraLobbyReader(Color color)
    : m_color(color)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(0.75, 0.67, 0.10, 0.05)
    , m_cursor(0.135, 0.25, 0.05, 0.25)
//    , m_stars(0.500, 0.555, 0.310, 0.070)
    , m_timer(0.175, 0.180, 0.100, 0.080)
    , m_code(0.310, 0.180, 0.200, 0.080)
{
    for (size_t c = 0; c < 4; c++){
        m_player_spinner[c] = ImageFloatBox{0.157, 0.575 + 0.070*c, 0.037, 0.060};
        m_player_name[c] = ImageFloatBox{0.200, 0.575 + 0.070*c, 0.200, 0.060};
        m_player_mon[c] = ImageFloatBox{0.425, 0.575 + 0.070*c, 0.037, 0.060};
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

std::string TeraLobbyReader::raid_code(Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

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
        {']', 'V'},
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
}

bool TeraLobbyReader::check_ban_for_image(
    Logger& logger,
    std::vector<TeraLobbyNameMatchResult>& matches,
    std::map<Language, CacheEntry>& cache, const ImageViewRGB32& image,
    const PlayerListRowSnapshot& ban_entry
) const{
    auto iter = cache.find(ban_entry.language);
    if (iter == cache.end()){
        CacheEntry entry;
        entry.raw_ocr = OCR::ocr_read(ban_entry.language, image);
        if (!entry.raw_ocr.empty() && entry.raw_ocr.back() == '\n'){
            entry.raw_ocr.pop_back();
        }
        entry.normalized = OCR::normalize_utf32(entry.raw_ocr);
        logger.log(
            "OCR Result (" + language_data(ban_entry.language).name + "): \"" +
            entry.raw_ocr + "\" -> \"" + to_utf8(entry.normalized) + "\""
        );
        iter = cache.emplace(ban_entry.language, std::move(entry)).first;
    }

    if (iter->second.normalized.empty()){
        return false;
    }

    std::u32string normalized_ban_entry = OCR::normalize_utf32(ban_entry.name);

    size_t distance = OCR::levenshtein_distance(
        iter->second.normalized,
        normalized_ban_entry
    );

    if (distance >= normalized_ban_entry.size()){
        return false;
    }

    double probability = OCR::random_match_probability(
        normalized_ban_entry.size(),
        normalized_ban_entry.size() - distance,
        language_data(ban_entry.language).random_match_chance
    );
    double log10p = std::log10(probability);

//    cout << ban_entry.name << " -> " << to_utf8(normalized_ban_entry)
//         << " : " << to_utf8(iter->second.normalized) << ", log10p = " << log10p << endl;

#if 0
    TeraLobbyNameMatchResult result{
        ban_entry,
        iter->second.raw_ocr,
        to_utf8(iter->second.normalized),
        log10p
    };
    cout << result.to_str() << endl;
#endif

    //  Not a match. Return now.
    if (log10p > ban_entry.log10p){
        return false;
    }


#if 1
    //  Matched. Check against the whitelist.
    static const std::vector<std::string> WHITELIST{
        "Alice",
        "Gael",
        "Dhruv",
        "Nikki",
        "denvoros",
        "Halazea",
    };
    for (const std::string& name : WHITELIST){
        std::u32string normalized_white_entry = OCR::normalize_utf32(name);
        size_t w_distance = OCR::levenshtein_distance(
            OCR::normalize_utf32(name),
            normalized_ban_entry
        );
        if (w_distance >= normalized_white_entry.size()){
            continue;
        }
        double w_probability = OCR::random_match_probability(
            normalized_white_entry.size(),
            normalized_white_entry.size() - w_distance,
            language_data(ban_entry.language).random_match_chance
        );
        double w_log10p = std::log10(w_probability);
        if (w_log10p <= ban_entry.log10p){
            if (PreloadSettings::instance().DEVELOPER_MODE){
                logger.log("Cannot ban whitelisted user: " + name + " (log10p = " + tostr_default(w_log10p) + ")", COLOR_RED);
            }
            return false;
        }
    }
#endif

    matches.emplace_back(TeraLobbyNameMatchResult{
        ban_entry,
        iter->second.raw_ocr,
        to_utf8(iter->second.normalized),
        log10p
    });

    return true;
}
PA_NO_INLINE std::vector<TeraLobbyNameMatchResult> TeraLobbyReader::check_ban_list(
    Logger& logger,
    const std::vector<PlayerListRowSnapshot>& ban_list,
    const ImageViewRGB32& screen,
    bool include_host
) const{
    if (ban_list.empty()){
        return {};
    }

    const uint32_t COLOR_THRESHOLD = 0xff5f5f5f;

    std::vector<TeraLobbyNameMatchResult> ret;

    size_t start = include_host ? 0 : 1;

    for (size_t c = start; c < 4; c++){
        ImageStats sprite = image_stats(extract_box_reference(screen, m_player_mon[c]));
        if (sprite.stddev.sum() <= 80){
            continue;
        }

        logger.log("Reading player " + std::to_string(c) + "'s name...");
        ImageViewRGB32 box = extract_box_reference(screen, m_player_name[c]);
        size_t pixels;
        ImageRGB32 filtered = to_blackwhite_rgb32_range(pixels, box, 0xff000000, COLOR_THRESHOLD, true);

        std::map<Language, CacheEntry> cache;
        for (const PlayerListRowSnapshot& entry : ban_list){
            check_ban_for_image(logger, ret, cache, filtered, entry);
        }
    }

    return ret;
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




TeraLobbyBanWatcher::TeraLobbyBanWatcher(Logger& logger, Color color, PlayerListTable& table, bool include_host)
    : TeraLobbyReader(color)
    , VisualInferenceCallback("TeraLobbyBanWatcher")
    , m_logger(logger)
    , m_table(table)
    , m_include_host(include_host)
{}
std::vector<TeraLobbyNameMatchResult> TeraLobbyBanWatcher::detected_banned_players() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_last_known_bans;
}
void TeraLobbyBanWatcher::make_overlays(VideoOverlaySet& items) const{
    TeraLobbyReader::make_overlays(items);
}
bool TeraLobbyBanWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    std::vector<TeraLobbyNameMatchResult> banned = check_ban_list(
        m_logger, m_table.snapshot(), frame, m_include_host
    );
    if (banned.empty()){
        return false;
    }
    std::lock_guard<std::mutex> lg(m_lock);
    m_last_known_bans = std::move(banned);
    return true;
}
















}
}
}
