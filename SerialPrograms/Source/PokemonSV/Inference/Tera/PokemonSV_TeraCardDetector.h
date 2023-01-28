/*  Tera Card Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCardDetector_H
#define PokemonAutomation_PokemonSV_TeraCardDetector_H

#include <atomic>
#include <mutex>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"

namespace PokemonAutomation{
    class AsyncDispatcher;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


ImageRGB32 filter_name_image(const ImageViewRGB32& image);



class TeraCardReader : public StaticScreenDetector{
public:
    TeraCardReader(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    size_t stars(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_label;
    ImageFloatBox m_cursor;
    ImageFloatBox m_stars;
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
    TeraLobbyReader(Logger& logger, AsyncDispatcher& dispatcher, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on an active lobby.
    virtual bool detect(const ImageViewRGB32& screen) const override;

    uint8_t total_players(const ImageViewRGB32& screen) const;
//    uint8_t ready_players(const ImageViewRGB32& screen) const;
    uint8_t ready_joiners(const ImageViewRGB32& screen, uint8_t host_players);

    int16_t seconds_left(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& screen) const;
    std::string raid_code(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& screen) const;

    //  OCR the player names in all the specified languages.
    //  The returned strings are raw OCR output and are unprocessed.
    std::array<std::map<Language, std::string>, 4> read_names(
        Logger& logger,
        const std::set<Language>& languages,
        const ImageViewRGB32& screen
    ) const;


private:
    Logger& m_logger;
    AsyncDispatcher& m_dispatcher;
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
        Logger& logger, AsyncDispatcher& dispatcher,
        Color color = COLOR_RED, std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("TeraLobbyFinder", duration, logger, dispatcher, color)
    {}
};

#if 0
class TeraLobbyReadyWaiter : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyReadyWaiter(
        Logger& logger, AsyncDispatcher& dispatcher,
        Color color, uint8_t desired_players
    );

    int8_t last_known_total_players() const{
        return m_last_known_total_players.load(std::memory_order_relaxed);
    }
    int8_t last_known_ready_players() const{
        return m_last_known_ready_players.load(std::memory_order_relaxed);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    uint8_t m_desired_players;
    std::atomic<int8_t> m_last_known_total_players;
    std::atomic<int8_t> m_last_known_ready_players;
};
#endif



}
}
}
#endif
