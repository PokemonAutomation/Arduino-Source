/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSelect_H

#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PathSelectDetector : public VisualInferenceCallbackWithCommandStop{
public:
    PathSelectDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen) const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_bottom_main;
    InferenceBoxScope m_bottom_right;
    InferenceBoxScope m_dialog_left;
    InferenceBoxScope m_dialog_middle;
    InferenceBoxScope m_dialog_right;
};


class PathPartyReader{
public:
    PathPartyReader(VideoOverlay& overlay);

    void read_sprites(Logger& logger, const QImage& screen, std::string slugs[4]);

private:
    InferenceBoxScope m_sprite0;
    InferenceBoxScope m_sprite1;
    InferenceBoxScope m_sprite2;
    InferenceBoxScope m_sprite3;
};




}
}
}
}
#endif
