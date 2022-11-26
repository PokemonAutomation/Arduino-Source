/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCardDetector_H
#define PokemonAutomation_PokemonSV_TeraCardDetector_H

#include <atomic>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

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
class TeraCardFinder : public DetectorToFinder<TeraCardReader>{
public:
    TeraCardFinder(Color color = COLOR_RED, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("TeraCardFinder", duration, color)
    {}
};



class TeraLobbyReader : public StaticScreenDetector{
public:
    TeraLobbyReader(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Returns true if we are on an active lobby.
    virtual bool detect(const ImageViewRGB32& screen) const override;

    uint8_t total_players(const ImageViewRGB32& screen) const;
    std::string raid_code(Logger& logger, const ProgramInfo& info, const ImageViewRGB32& screen);


private:
    Color m_color;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_label;
    ImageFloatBox m_cursor;
//    ImageFloatBox m_stars;

    ImageFloatBox m_timer;
    ImageFloatBox m_code;

    ImageFloatBox m_player1_spinner;
    ImageFloatBox m_player2_spinner;
    ImageFloatBox m_player3_spinner;

    ImageFloatBox m_player0_mon;
    ImageFloatBox m_player1_mon;
    ImageFloatBox m_player2_mon;
    ImageFloatBox m_player3_mon;
};
class TeraLobbyFinder : public DetectorToFinder<TeraLobbyReader>{
public:
    TeraLobbyFinder(Color color = COLOR_RED, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
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



}
}
}
#endif
