/*  Tera Card Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCardDetector_H
#define PokemonAutomation_PokemonSV_TeraCardDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"
#include "PokemonSV_TeraTypeReader.h"
#include "PokemonSV_TeraSilhouetteReader.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


ImageRGB32 filter_name_image(const ImageViewRGB32& image);



class TeraCardReader : public StaticScreenDetector{
public:
    TeraCardReader(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    uint8_t stars(
        Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
    ) const;
    std::string tera_type(
        Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
    ) const;
    std::set<std::string> pokemon_slug(
        Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen
    ) const;

    static const ImageFloatBox& CARD_LABEL_BOX();
    static bool is_card_label(const ImageViewRGB32& screen);

private:
    Color m_color;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_label;
    ImageFloatBox m_cursor;

    ImageFloatBox m_stars;
    TeraTypeReader m_tera_type;
    TeraSilhouetteReader m_silhouette;
};
class TeraCardWatcher : public DetectorToFinder<TeraCardReader>{
public:
    TeraCardWatcher(Color color = COLOR_RED, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("TeraCardFinder", duration, color)
    {}
};




struct TeraLobbyNameMatchResult{
    PlayerListRowSnapshot entry;
    std::string banlist_source;

    std::string raw_ocr;
    std::string normalized_ocr;
    double log10p;
    bool exact_match;
    std::string notes;

    std::string to_str() const;
};



class TeraLobbyReader : public StaticScreenDetector{
public:
    TeraLobbyReader(Logger& logger, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on an active lobby.
    virtual bool detect(const ImageViewRGB32& screen) override;

    uint8_t total_players(const ImageViewRGB32& screen) const;
//    uint8_t ready_players(const ImageViewRGB32& screen) const;
    uint8_t ready_joiners(const ImageViewRGB32& screen, uint8_t host_players);

    int16_t seconds_left(Logger& logger, const ImageViewRGB32& screen) const;
    std::string raid_code(Logger& logger, const ImageViewRGB32& screen) const;

    //  OCR the player names in all the specified languages.
    //  The returned strings are raw OCR output and are unprocessed.
    std::array<std::map<Language, std::string>, 4> read_names(
        Logger& logger,
        const std::set<Language>& languages,
        const ImageViewRGB32& screen
    ) const;


private:
    Logger& m_logger;
    Color m_color;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_label;
    ImageFloatBox m_cursor;
//    ImageFloatBox m_stars;

    ImageFloatBox m_timer;
    ImageFloatBox m_code;

    ImageFloatBox m_player_spinner[4];
    ImageFloatBox m_player_name[4];
    ImageFloatBox m_player_mon[4];
    ImageFloatBox m_player_ready[4];
};
class TeraLobbyWatcher : public DetectorToFinder<TeraLobbyReader>{
public:
    TeraLobbyWatcher(
        Logger& logger,
        Color color = COLOR_RED,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("TeraLobbyFinder", duration, logger, color)
    {}
};




}
}
}
#endif
