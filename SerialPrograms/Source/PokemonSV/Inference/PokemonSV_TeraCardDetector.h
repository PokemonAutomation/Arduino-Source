/*  Dialog Detector
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
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



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
    std::string raw_ocr;
    std::string normalized_ocr;
    double log10p;

    std::string to_str() const;
};



class TeraLobbyReader : public StaticScreenDetector{
public:
    TeraLobbyReader(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on an active lobby.
    virtual bool detect(const ImageViewRGB32& screen) const override;

    uint8_t total_players(const ImageViewRGB32& screen) const;
    std::string raid_code(Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen);

    //  Returns true if someone is on the ban list.
    std::vector<TeraLobbyNameMatchResult> check_ban_list(
        Logger& logger,
        const std::vector<PlayerListRowSnapshot>& ban_list,
        const ImageViewRGB32& screen,
        bool include_host
    ) const;

private:
    struct CacheEntry{
        std::string raw_ocr;
        std::u32string normalized;
    };
    bool check_ban_for_image(
        Logger& logger,
        std::vector<TeraLobbyNameMatchResult>& matches,
        std::map<Language, CacheEntry>& cache, const ImageViewRGB32& image,
        const PlayerListRowSnapshot& entry
    ) const;


private:
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
};
class TeraLobbyWatcher : public DetectorToFinder<TeraLobbyReader>{
public:
    TeraLobbyWatcher(Color color = COLOR_RED, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("TeraLobbyFinder", duration, color)
    {}
};
class TeraLobbyReadyWaiter : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyReadyWaiter(Color color, uint8_t desired_players);

    int8_t last_known_player_count() const{
        return m_last_known_player_count.load(std::memory_order_relaxed);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    uint8_t m_desired_players;
    std::atomic<int8_t> m_last_known_player_count;
};
class TeraLobbyBanWatcher : public TeraLobbyReader, public VisualInferenceCallback{
public:
    TeraLobbyBanWatcher(Logger& logger, Color color, PlayerListTable& table, bool include_host);

    std::vector<TeraLobbyNameMatchResult> detected_banned_players() const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    PlayerListTable& m_table;
    bool m_include_host;
    mutable std::mutex m_lock;
    std::vector<TeraLobbyNameMatchResult> m_last_known_bans;
};



}
}
}
#endif
