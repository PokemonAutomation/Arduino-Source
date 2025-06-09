/*  Blueberry Quest Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryQuestDetector_H
#define PokemonAutomation_PokemonSV_BlueberryQuestDetector_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Language.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{


// Detect the quest in a given position (four positions before having to scroll)
class BlueberryQuestDetector : public StaticScreenDetector{
public:
    //Max quests: 4 in singleplayer (1 red, 3 blue). 17 in multiplayer (with 4 players: 1 gold, 4 red, 12 blue).
    //For multiplayer, never read other player's quests. So 1 gold, 4 red, 3 blue.
    //Only 4 quests are visible at a time. Need to scroll down.
    enum class QuestPosition{
        FIRST,
        SECOND,
        THIRD,
        FOURTH
    };
    BlueberryQuestDetector(Logger& logger, Color color, Language language, QuestPosition position);
//    virtual ~BlueberryQuestDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Return detected quest slug. Return empty string if not detected
    std::string detect_quest(const ImageViewRGB32& screen) const;

protected:
    Logger& m_logger;
    Color m_color;
    Language m_language;
    QuestPosition m_position;
    ImageFloatBox m_box;
};


// Detect the quest in a given position (four positions before having to scroll)
class BlueberryQuestWatcher : public VisualInferenceCallback{
public:
    using Side = BlueberryQuestDetector::QuestPosition;
    ~BlueberryQuestWatcher();
    BlueberryQuestWatcher(Logger& logger, Color color, VideoOverlay& overlay,  Language language, BlueberryQuestDetector::QuestPosition position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::string& quest_name() const { return m_quest_name; }


protected:
    VideoOverlay& m_overlay;
    BlueberryQuestDetector m_detector;
    std::string m_quest_name;
};




}
}
}
#endif
