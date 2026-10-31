/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <opencv2/imgproc.hpp>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_BattleMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


BattleMenuDetector::BattleMenuDetector(BattleType battle_type, Color color)
    : m_color(color)
    , m_battle_type(battle_type)
    , m_left0_status    (0.140, 0.922, 0.100, 0.010)
    , m_left1_status    (0.140, 0.910, 0.100, 0.010)
    , m_right_status    (0.405, 0.925, 0.100, 0.010)
//    , m_opponent_left   (0.685, 0.065, 0.020, 0.030)
//    , m_opponent_right  (0.960, 0.065, 0.020, 0.030)
    , m_opponent_left   (0.708, 0.070, 0.005, 0.028)
    , m_opponent_right  (0.982, 0.070, 0.005, 0.028)
    , m_ball_left       (0.890, 0.475, 0.02, 0.03)
    , m_ball_right      (0.960, 0.475, 0.03, 0.03)
    , m_menu_battle     (0.817, 0.585 + 0 * 0.1075, 0.150, 0.070)
    , m_menu_pokemon    (0.817, 0.585 + 1 * 0.1075, 0.150, 0.070)
    , m_menu_bag        (0.817, 0.585 + 2 * 0.1075, 0.150, 0.070)
    , m_menu_run        (0.817, 0.585 + 3 * 0.1075, 0.150, 0.070)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_left0_status);
    items.add(m_color, m_left1_status);
    items.add(m_color, m_right_status);
    items.add(m_color, m_opponent_left);
    items.add(m_color, m_opponent_right);
    items.add(m_color, m_ball_left);
    items.add(m_color, m_ball_right);
    items.add(m_color, m_menu_battle);
    items.add(m_color, m_menu_pokemon);
    items.add(m_color, m_menu_bag);
    items.add(m_color, m_menu_run);
}
bool BattleMenuDetector::is_battle_button(const ImageViewRGB32& screen) const{
    ImageStats stats = image_stats(extract_box_reference(screen, m_menu_battle));
//    cout << stats.average << stats.stddev << endl;
    double stddev = stats.stddev.sum();
    if (stddev < 30 || stddev > 150){
        return false;
    }
    double average = stats.average.sum();
    FloatPixel actual = stats.average / average;
    if (euclidean_distance(actual, {0.541961, 0.231317, 0.226722}) > 0.2){
        return false;
    }
    return true;
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    {
        bool left = is_white(extract_box_reference(screen, m_opponent_left));
        bool right = is_white(extract_box_reference(screen, m_opponent_right));
        if (!left && !right){
//            cout << "opp left or right" << endl;
            return false;
        }
    }
    {
        bool left0 = is_white(extract_box_reference(screen, m_left0_status));
        bool left1 = is_white(extract_box_reference(screen, m_left1_status));
        bool right = is_white(extract_box_reference(screen, m_right_status));
        if (!left0 && !left1 && !right){
            return false;
        }
    }

    if (m_battle_type == BattleType::STARTER){
        return is_battle_button(screen);
    }


    if (m_battle_type == BattleType::STANDARD){
        if (!is_white(extract_box_reference(screen, m_ball_left))){
//            cout << "Not white" << endl;
            return false;
        }
        if (!is_white(extract_box_reference(screen, m_ball_right))){
//            cout << "Not white" << endl;
            return false;
        }
    }

    if (!is_battle_button(screen)){
        return false;
    }
//    cout << "Start Checking Pokemon" << endl;
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_pokemon));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 150){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.255944, 0.582771, 0.161285}) > 0.2){
            return false;
        }
    }
    //    cout << "Start Checking bag" << endl;
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_bag));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 100){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.485857, 0.414946, 0.099197}) > 0.2){
            return false;
        }
    }
    {
        ImageStats stats = image_stats(extract_box_reference(screen, m_menu_run));
//        cout << stats.average << stats.stddev << endl;
        double stddev = stats.stddev.sum();
        if (stddev < 30 || stddev > 100){
            return false;
        }
        double average = stats.average.sum();
        FloatPixel actual = stats.average / average;
        if (euclidean_distance(actual, {0.250977, 0.269868, 0.479155}) > 0.2){
            return false;
        }
    }

    return true;
}




BattleMenuOption BattleMenuDetector::selected_option(const ImageViewRGB32& screen){
    if (!screen || !detect(screen)){
        return BattleMenuOption::UNKNOWN;
    }
    int selected = -1;
    for (int row = 0; row < 4; row++){
        const double offset = row * 0.1075;
        bool brackets = false;
        // The corner brackets wiggle; all four must agree on one small offset.
        const double offsets[] = {0.0, -0.003, 0.003, -0.006, 0.006};
        for (double dy : offsets){
            for (double dx : offsets){
                const ImageFloatBox corners[] = {
                    {0.793 + dx, 0.562 + offset + dy, 0.018, 0.023},
                    {0.976 + dx, 0.562 + offset + dy, 0.018, 0.023},
                    {0.793 + dx, 0.652 + offset + dy, 0.018, 0.023},
                    {0.976 + dx, 0.652 + offset + dy, 0.018, 0.023},
                };
                bool candidate = true;
                for (const ImageFloatBox& box : corners){
                    const ImageViewRGB32 patch = extract_box_reference(screen, box);
                    size_t white = 0;
                    const size_t total = patch.width() * patch.height();
                    for (size_t y = 0; y < patch.height(); y++){
                        for (size_t x = 0; x < patch.width(); x++){
                            const uint32_t pixel = patch.pixel(x, y);
                            const unsigned r = (pixel >> 16) & 255;
                            const unsigned g = (pixel >> 8) & 255;
                            const unsigned b = pixel & 255;
                            const unsigned low = std::min(r, std::min(g, b));
                            const unsigned high = std::max(r, std::max(g, b));
                            white += low > 210 && high - low < 35;
                        }
                    }
                    if (total == 0 || static_cast<double>(white) / total < 0.15){
                        candidate = false;
                        break;
                    }
                }
                if (candidate){
                    brackets = true;
                    break;
                }
            }
            if (brackets){
                break;
            }
        }
        if (brackets){
            if (selected >= 0){
                return BattleMenuOption::UNKNOWN;
            }
            selected = row;
        }
    }
    return static_cast<BattleMenuOption>(selected);
}

BattleMoveMenuState detect_battle_move_menu(const ImageViewRGB32& screen){
    BattleMoveMenuState result;
    if (!screen) return result;
    // Every populated move has a white PP panel, irrespective of move type or
    // selected row. Find connected panels rather than assume a 4-row layout.
    const ImageFloatBox search{0.90, 0.48, 0.075, 0.505};
    const ImageViewRGB32 patch = extract_box_reference(screen, search);
    cv::Mat white((int)patch.height(), (int)patch.width(), CV_8UC1);
    for (size_t y = 0; y < patch.height(); y++) for (size_t x = 0; x < patch.width(); x++) {
        uint32_t p = patch.pixel(x, y);
        int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
        white.at<uint8_t>((int)y, (int)x) = std::min(r, std::min(g, b)) > 225 &&
            std::max(r, std::max(g, b)) - std::min(r, std::min(g, b)) < 30 ? 255 : 0;
    }
    cv::Mat labels, stats, centers;
    int components = cv::connectedComponentsWithStats(white, labels, stats, centers);
    std::vector<cv::Rect> panels;
    for (int component = 1; component < components; component++) {
        cv::Rect box(stats.at<int>(component, cv::CC_STAT_LEFT), stats.at<int>(component, cv::CC_STAT_TOP),
            stats.at<int>(component, cv::CC_STAT_WIDTH), stats.at<int>(component, cv::CC_STAT_HEIGHT));
        if (box.width < patch.width() * 0.60 || box.height < screen.height() * 0.055 ||
            box.height > screen.height() * 0.13) continue;
        double area = (double)stats.at<int>(component, cv::CC_STAT_AREA);
        if (area / box.area() < 0.55) continue;
        panels.push_back(box);
    }
    if (panels.empty() || panels.size() > 4) return result;
    std::sort(panels.begin(), panels.end(), [](const cv::Rect& a, const cv::Rect& b){ return a.y < b.y; });
    // Reject inconsistent/overlapping geometry rather than count noise as moves.
    for (size_t i = 1; i < panels.size(); i++) {
        if (panels[i].y <= panels[i - 1].y + panels[i - 1].height ||
            std::abs(panels[i].height - panels[0].height) > screen.height() * 0.025) return result;
    }
    result.move_count = panels.size();
    auto is_corner = [&](double x, double y){
        if (x < 0 || y < 0 || x + 0.018 > 1 || y + 0.023 > 1) return false;
        ImageViewRGB32 corner = extract_box_reference(screen, ImageFloatBox{x, y, 0.018, 0.023});
        size_t white_count = 0, total = corner.width() * corner.height();
        for (size_t py = 0; py < corner.height(); py++) for (size_t px = 0; px < corner.width(); px++) {
            uint32_t p = corner.pixel(px, py);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            white_count += std::min(r, std::min(g, b)) > 210 &&
                std::max(r, std::max(g, b)) - std::min(r, std::min(g, b)) < 35;
        }
        return total && (double)white_count / total >= 0.15;
    };
    for (size_t slot = 0; slot < panels.size(); slot++) {
        double top = search.y + (double)panels[slot].y / screen.height();
        double bottom = top + (double)panels[slot].height / screen.height();
        result.buttons[slot] = ImageFloatBox{0.665, std::max(0.0, top - 0.01), 0.333,
            std::min(1.0, bottom + 0.01) - std::max(0.0, top - 0.01)};
        bool selected = false;
        for (double dy : {0.0, -0.003, 0.003, -0.006, 0.006, -0.010, 0.010}) {
            for (double dx : {0.0, -0.003, 0.003}) {
                if (is_corner(0.663 + dx, top - 0.010 + dy) &&
                    is_corner(0.976 + dx, top - 0.010 + dy) &&
                    is_corner(0.663 + dx, bottom - 0.014 + dy) &&
                    is_corner(0.976 + dx, bottom - 0.014 + dy)) { selected = true; break; }
            }
            if (selected) break;
        }
        if (selected) {
            if (result.selected_slot) { result.selected_slot = 0; return result; }
            result.selected_slot = slot + 1;
        }
    }
    return result;
}

BattleMenuWatcher::BattleMenuWatcher(BattleType battle_type, Color color)
    : BattleMenuDetector(battle_type, color)
    , VisualInferenceCallback("BattleMenuWatcher")
{}
void BattleMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    BattleMenuDetector::make_overlays(items);
}
bool BattleMenuWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!detect(frame)){
//        cout << "no detect" << endl;
        m_trigger_count = 0;
        return false;
    }
//    cout << "detected" << endl;
    m_trigger_count++;
    return m_trigger_count >= 5;
}




}
}
}
