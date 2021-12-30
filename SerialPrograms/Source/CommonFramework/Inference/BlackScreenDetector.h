/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#ifndef PokemonAutomation_CommonFramework_BlackScreenDetector_H
#define PokemonAutomation_CommonFramework_BlackScreenDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{


class BlackScreenDetector : public StaticScreenDetector{
public:
    BlackScreenDetector(
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        QColor color = Qt::red
    );

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
    ImageFloatBox m_box;
};


class BlackScreenWatcher : public BlackScreenDetector, public VisualInferenceCallback{
public:
    using BlackScreenDetector::BlackScreenDetector;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;
};


class BlackScreenOverWatcher : public VisualInferenceCallback{
public:
    BlackScreenOverWatcher(
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        QColor color = Qt::red
    );

    bool black_is_over(const QImage& frame);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
   ) override;

private:
    BlackScreenDetector m_detector;
    bool m_has_been_black = false;
};



}
#endif
