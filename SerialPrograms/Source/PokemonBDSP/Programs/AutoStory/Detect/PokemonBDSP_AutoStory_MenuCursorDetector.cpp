/*  BDSP AutoStory - Menu Cursor Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <limits>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonBDSP_AutoStory_MenuCursorDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


std::string menu_cursor_to_string(MenuCursorPosition pos){
    switch (pos){
    case MenuCursorPosition::POKEDEX:     return "POKEDEX";
    case MenuCursorPosition::POKEMON:     return "POKEMON";
    case MenuCursorPosition::BAG:         return "BAG";
    case MenuCursorPosition::CARD:        return "CARD";
    case MenuCursorPosition::OPTIONS:     return "OPTIONS";
    case MenuCursorPosition::MAP:         return "MAP";
    case MenuCursorPosition::CAPSULES:    return "CAPSULES";
    case MenuCursorPosition::MYSTERYGIFT: return "MYSTERYGIFT";
    default:                              return "UNKNOWN";
    }
}


MenuCursorDetector::MenuCursorDetector(Color color)
    : m_color(color)
    // -------------------------------------------------------------------------
    //  Top row — left to right
    // -------------------------------------------------------------------------
    // TODO: replace each ImageFloatBox(0,0,0,0) with the real normalized
    // (x, y, width, height) for a small region that gains dark pixels when
    // that icon is hovered. Use the test program overlays to tune coordinates.
    // -------------------------------------------------------------------------
    , m_pokedex_box{
        ImageFloatBox(0.180000, 0.115000, 0.135000, 0.195000),
    }
    , m_pokemon_box{
        ImageFloatBox(0.347500, 0.115000, 0.135000, 0.195000),
    }
    , m_bag_box{
        ImageFloatBox(0.515000, 0.115000, 0.135000, 0.195000),
    }
    , m_card_box{
        ImageFloatBox(0.682500, 0.115000, 0.135000, 0.195000),
    }
    // -------------------------------------------------------------------------
    //  Bottom row — left to right
    //  5-icon layout: OPTIONS is the only bottom icon (far-left position).
    //  8-icon layout: MAP, CAPSULES, OPTIONS, MYSTERYGIFT from left to right.
    //  OPTIONS sits at a different x-position in each layout — use a separate
    //  MenuCursorDetector instance if you need to detect OPTIONS in both.
    // -------------------------------------------------------------------------
    , m_options_box{
        ImageFloatBox(0.515000, 0.364000, 0.135000, 0.195000),
    }
    , m_map_box{
        ImageFloatBox(0.180000, 0.364000, 0.135000, 0.195000),
    }
    , m_capsules_box{
        ImageFloatBox(0.347500, 0.364000, 0.135000, 0.195000),
    }
    , m_mysterygift_box{
        ImageFloatBox(0.682500, 0.364000, 0.135000, 0.195000),
    }
    // -------------------------------------------------------------------------
    //  Approach 2 search boxes — one per icon, same area as the dark-pixel box.
    //  The template PNG is slid across this area at its native size.
    // -------------------------------------------------------------------------
    , m_icon_match_box{
        ImageFloatBox(0.180000, 0.115000, 0.135000, 0.195000),  // 0 POKEDEX
        ImageFloatBox(0.347500, 0.115000, 0.135000, 0.195000),  // 1 POKEMON
        ImageFloatBox(0.515000, 0.115000, 0.135000, 0.195000),  // 2 BAG
        ImageFloatBox(0.682500, 0.115000, 0.135000, 0.195000),  // 3 CARD
        ImageFloatBox(0.515000, 0.364000, 0.135000, 0.195000),  // 4 OPTIONS (8-icon layout)
        ImageFloatBox(0.180000, 0.364000, 0.135000, 0.195000),  // 5 MAP
        ImageFloatBox(0.347500, 0.364000, 0.135000, 0.195000),  // 6 CAPSULES
        ImageFloatBox(0.682500, 0.364000, 0.135000, 0.195000),  // 7 MYSTERYGIFT
    }
{}


void MenuCursorDetector::make_overlays(VideoOverlaySet& items) const{
    for (const ImageFloatBox* icon_boxes : {
        m_pokedex_box, m_pokemon_box, m_bag_box, m_card_box,
        m_options_box, m_map_box, m_capsules_box, m_mysterygift_box,
    }){
        for (int i = 0; i < BOXES_PER_ICON; i++){
            if (icon_boxes[i].width > 0.0 && icon_boxes[i].height > 0.0)
                items.add(m_color, icon_boxes[i]);
        }
    }
    for (int i = 0; i < MENU_CURSOR_NUM_ICONS; i++){
        if (m_icon_match_box[i].width > 0.0 && m_icon_match_box[i].height > 0.0)
            items.add(m_color, m_icon_match_box[i]);
    }
}


namespace{

// Count how many of `n` boxes contain >= MIN_DARK_PIXEL_COUNT pixels with
// R, G, B all <= BLACK_CHANNEL_MAX. Boxes with zero width are skipped.
int count_dark_boxes(
    const ImageViewRGB32& screen,
    const ImageFloatBox* boxes, int n
){
    const uint32_t ch   = uint32_t(MenuCursorDetector::BLACK_CHANNEL_MAX);
    const uint32_t maxs = 0xff000000u | (ch << 16) | (ch << 8) | ch;

    int count = 0;
    for (int i = 0; i < n; i++){
        if (boxes[i].width == 0.0) continue;
        size_t dark_pixels = 0;
        filter_rgb32_range(
            dark_pixels,
            extract_box_reference(screen, boxes[i]),
            0xff000000u, maxs,
            Color(0xffffffff), false
        );
        if (dark_pixels >= MenuCursorDetector::MIN_DARK_PIXEL_COUNT) count++;
    }
    return count;
}

// Pixel-wise RMSD between two same-size images over RGB channels only.
double compute_rmsd(const ImageViewRGB32& a, const ImageViewRGB32& b){
    size_t n = a.width() * a.height();
    if (n == 0) return 0.0;
    double sum_sq = 0.0;
    for (size_t y = 0; y < a.height(); y++){
        for (size_t x = 0; x < a.width(); x++){
            uint32_t pa = a.pixel(x, y);
            uint32_t pb = b.pixel(x, y);
            int dr = int((pa >> 16) & 0xff) - int((pb >> 16) & 0xff);
            int dg = int((pa >>  8) & 0xff) - int((pb >>  8) & 0xff);
            int db = int( pa        & 0xff) - int( pb        & 0xff);
            sum_sq += double(dr*dr + dg*dg + db*db);
        }
    }
    return std::sqrt(sum_sq / (3.0 * n));
}

// Slide `tmpl` (at its native pixel size) across the region of `screen`
// defined by `box`. Returns the minimum RMSD found over all window positions.
double template_match_rmsd(
    const ImageViewRGB32& screen,
    const ImageFloatBox&  box,
    const ImageViewRGB32& tmpl
){
    ImageViewRGB32 region = extract_box_reference(screen, box);
    size_t rw = tmpl.width(),   rh = tmpl.height();
    size_t lw = region.width(), lh = region.height();
    if (rw == 0 || rh == 0 || rw > lw || rh > lh)
        return 0.0;

    double min_rmsd = std::numeric_limits<double>::max();
    for (size_t dy = 0; dy <= lh - rh; dy++){
        for (size_t dx = 0; dx <= lw - rw; dx++){
            double r = compute_rmsd(region.sub_image(dx, dy, rw, rh), tmpl);
            if (r < min_rmsd) min_rmsd = r;
        }
    }
    return min_rmsd;
}

} // anonymous namespace


MenuCursorPosition MenuCursorDetector::detect_dark_pixels(
    const ImageViewRGB32& screen
) const {
    struct IconEntry{
        MenuCursorPosition position;
        const ImageFloatBox* boxes;
    };
    const IconEntry icons[] = {
        { MenuCursorPosition::POKEDEX,     m_pokedex_box     },
        { MenuCursorPosition::POKEMON,     m_pokemon_box     },
        { MenuCursorPosition::BAG,         m_bag_box         },
        { MenuCursorPosition::CARD,        m_card_box        },
        { MenuCursorPosition::OPTIONS,     m_options_box     },
        { MenuCursorPosition::MAP,         m_map_box         },
        { MenuCursorPosition::CAPSULES,    m_capsules_box    },
        { MenuCursorPosition::MYSTERYGIFT, m_mysterygift_box },
    };

    MenuCursorPosition best_pos   = MenuCursorPosition::UNKNOWN;
    int                best_count = 0;

    for (const auto& entry : icons){
        int count = count_dark_boxes(screen, entry.boxes, BOXES_PER_ICON);
        if (count > best_count){
            best_count = count;
            best_pos   = entry.position;
        }
    }

    return (best_count >= REQUIRED_VOTES) ? best_pos : MenuCursorPosition::UNKNOWN;
}


MenuCursorPosition MenuCursorDetector::detect_image_match(
    const ImageViewRGB32& screen
) const {
    MenuCursorPosition best_pos  = MenuCursorPosition::UNKNOWN;
    double             best_rmsd = MATCH_RMSD_THRESHOLD;   // must exceed this

    for (int i = 0; i < MENU_CURSOR_NUM_ICONS; i++){
        if (!m_icon_references[i].has_value()) continue;
        if (m_icon_match_box[i].width == 0.0) continue;
        double r = template_match_rmsd(screen, m_icon_match_box[i], *m_icon_references[i]);
        if (r > best_rmsd){
            best_rmsd = r;
            best_pos  = static_cast<MenuCursorPosition>(i);
        }
    }
    return best_pos;
}


MenuCursorPosition MenuCursorDetector::detect(
    const ImageViewRGB32& screen
) const {
    MenuCursorPosition p1 = detect_dark_pixels(screen);
    MenuCursorPosition p2 = detect_image_match(screen);

    if (p1 == p2) return p1;                               // both agree (or both UNKNOWN)
    if (p1 == MenuCursorPosition::UNKNOWN) return p2;     // only approach 2 fired
    if (p2 == MenuCursorPosition::UNKNOWN) return p1;     // only approach 1 fired
    return p1;                                             // conflict: prefer approach 1
}


void MenuCursorDetector::set_icon_reference(
    MenuCursorPosition icon,
    ImageRGB32 reference
){
    m_icon_references[static_cast<int>(icon)].emplace(std::move(reference));
}


double MenuCursorDetector::icon_rmsd(
    const ImageViewRGB32& screen, MenuCursorPosition icon
) const {
    int idx = static_cast<int>(icon);
    if (!m_icon_references[idx].has_value() || m_icon_match_box[idx].width == 0.0)
        return -1.0;
    return template_match_rmsd(screen, m_icon_match_box[idx], *m_icon_references[idx]);
}


MenuCursorPosition detect_menu_cursor(VideoStream& stream, ProControllerContext& /*context*/){
    VideoSnapshot screen = stream.video().snapshot();
    if (!screen) return MenuCursorPosition::UNKNOWN;
    MenuCursorDetector detector;
    return detector.detect(screen);
}


}
}
}
