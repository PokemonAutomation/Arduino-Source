/*  BDSP AutoStory - Menu Cursor Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detects which icon the cursor is hovering over in the X-menu.
 *
 *  Two independent detection approaches are provided for testing:
 *
 *  Approach 1 — dark pixel count (detect_dark_pixels):
 *    Each icon has BOXES_PER_ICON small boxes placed over a region that gains
 *    dark pixels when hovered. A box fires if >= MIN_DARK_PIXEL_COUNT pixels
 *    in it have R, G, B all <= BLACK_CHANNEL_MAX. An icon is selected when its
 *    fire count reaches REQUIRED_VOTES.
 *    Tune BLACK_CHANNEL_MAX upward if real dark pixels are being missed.
 *    Tune it downward (or raise MIN_DARK_PIXEL_COUNT) to reduce false positives.
 *
 *  Approach 2 — template matching (detect_image_match):
 *    Each icon has one larger search box and a hovered-state reference PNG loaded
 *    at runtime via set_icon_reference(). The reference PNG is slid (at its native
 *    pixel size, without resizing) across the search box and the minimum RMSD over
 *    all window positions is computed. The icon with the lowest min-RMSD that is
 *    also <= MATCH_RMSD_THRESHOLD is considered selected.
 *    Tune MATCH_RMSD_THRESHOLD downward to require a closer match.
 *
 *  detect() is the production method and currently delegates to approach 1.
 *  It will become a combined vote once both approaches are validated.
 *
 *  Menu layouts:
 *    5-icon (early game):  POKEDEX  POKEMON  BAG  CARD  /  OPTIONS
 *    8-icon (late game):   POKEDEX  POKEMON  BAG  CARD  /  MAP  CAPSULES  OPTIONS  MYSTERYGIFT
 *
 *  Note: OPTIONS sits at a different screen position in each layout.
 *  Use separate MenuCursorDetector instances with different box coordinates
 *  if you need to detect OPTIONS reliably in both layouts.
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_MenuCursorDetector_H
#define PokemonAutomation_PokemonBDSP_AutoStory_MenuCursorDetector_H

#include <array>
#include <optional>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class MenuCursorPosition{
    UNKNOWN     = -1,
    POKEDEX     = 0,
    POKEMON     = 1,
    BAG         = 2,
    CARD        = 3,
    OPTIONS     = 4,
    MAP         = 5,
    CAPSULES    = 6,
    MYSTERYGIFT = 7,
};

static constexpr int MENU_CURSOR_NUM_ICONS = 8;

std::string menu_cursor_to_string(MenuCursorPosition pos);


class MenuCursorDetector{
public:
    // ── Approach 1: dark pixel detection ─────────────────────────────────────
    static constexpr int    BOXES_PER_ICON        = 3;
    static constexpr int    REQUIRED_VOTES        = 1;    // out of BOXES_PER_ICON
    static constexpr uint8_t BLACK_CHANNEL_MAX    = 50;   // R,G,B <= this → dark pixel
    static constexpr size_t  MIN_DARK_PIXEL_COUNT = 1;    // dark pixels per box to fire

    // ── Approach 2: template match ───────────────────────────────────────────
    // Reference PNGs are the UN-hovered state of each icon. When hovered, the
    // icon gains dark pixels and diverges from the reference, so RMSD increases.
    // detect_image_match() selects the icon with the HIGHEST RMSD >= threshold.
    // Tune upward to reduce false positives; downward if the correct icon is missed.
    static constexpr double MATCH_RMSD_THRESHOLD  = 53.0;

    MenuCursorDetector(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    // Approach 1: votes across dark-pixel boxes, returns icon with highest
    // vote count >= REQUIRED_VOTES, or UNKNOWN.
    MenuCursorPosition detect_dark_pixels(const ImageViewRGB32& screen) const;

    // Approach 2: returns the icon with the highest RMSD against its reference,
    // provided that RMSD >= MATCH_RMSD_THRESHOLD (high RMSD = hovered).
    // Returns UNKNOWN if no references are loaded or nothing exceeds threshold.
    MenuCursorPosition detect_image_match(const ImageViewRGB32& screen) const;

    // Production method — currently delegates to detect_dark_pixels().
    MenuCursorPosition detect(const ImageViewRGB32& screen) const;

    // Load a hovered-state reference image for one icon (enables approach 2 for
    // that icon). The match box for that icon must already be set in the
    // constructor (m_icon_match_box); this method just attaches the reference.
    void set_icon_reference(MenuCursorPosition icon, ImageRGB32 reference);

    // Returns the raw RMSD of `icon` against its reference on the given frame.
    // Returns -1.0 if no reference is loaded for that icon.
    double icon_rmsd(const ImageViewRGB32& screen, MenuCursorPosition icon) const;

private:
    Color m_color;

    // ── Approach 1 boxes ─────────────────────────────────────────────────────
    // 3 boxes per icon targeting regions that gain dark pixels when hovered.
    // All initialized to (0,0,0,0) — replace with real normalized coordinates.
    ImageFloatBox m_pokedex_box[BOXES_PER_ICON];
    ImageFloatBox m_pokemon_box[BOXES_PER_ICON];
    ImageFloatBox m_bag_box[BOXES_PER_ICON];
    ImageFloatBox m_card_box[BOXES_PER_ICON];
    ImageFloatBox m_options_box[BOXES_PER_ICON];
    ImageFloatBox m_map_box[BOXES_PER_ICON];
    ImageFloatBox m_capsules_box[BOXES_PER_ICON];
    ImageFloatBox m_mysterygift_box[BOXES_PER_ICON];

    // ── Approach 2 search boxes + references ────────────────────────────────
    // One search box per icon (the area to slide the template across).
    // References are populated via set_icon_reference() at runtime.
    ImageFloatBox m_icon_match_box[MENU_CURSOR_NUM_ICONS];
    std::array<std::optional<ImageRGB32>, MENU_CURSOR_NUM_ICONS> m_icon_references;
};


// Captures the current frame from `stream` and runs detect() on it.
MenuCursorPosition detect_menu_cursor(VideoStream& stream, ProControllerContext& context);


}
}
}
#endif
