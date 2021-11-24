/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleDialogDetector_H
#define PokemonAutomation_PokemonBDSP_BattleDialogDetector_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShortDialogDetector : public ScreenDetector{
public:
    ShortDialogDetector(VideoOverlay& overlay);

    virtual bool detect(const QImage& screen) const override;

private:
    InferenceBoxScope m_bottom;
    InferenceBoxScope m_left_white;
    InferenceBoxScope m_left;
    InferenceBoxScope m_right;
};
class ShortDialogDetectorCallback : public ShortDialogDetector, public VisualInferenceCallback{
public:
    ShortDialogDetectorCallback(VideoOverlay& overlay);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};




class BattleDialogDetector : public ScreenDetector{
public:
    BattleDialogDetector(VideoOverlay& overlay);

    virtual bool detect(const QImage& screen) const override;

private:
    InferenceBoxScope m_bottom;
    InferenceBoxScope m_left_white;
    InferenceBoxScope m_left;
    InferenceBoxScope m_right;
};



}
}
}
#endif
