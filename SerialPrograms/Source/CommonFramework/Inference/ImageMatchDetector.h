/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageMatchDetector_H
#define PokemonAutomation_CommonFramework_ImageMatchDetector_H

#include <QImage>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{

class ImageMatchDetector : public StaticScreenDetector{
public:
    ImageMatchDetector(
        QImage reference_image, const ImageFloatBox& box,
        double max_rmsd, bool scale_brightness = false,
        Color color = COLOR_RED
    );

    double rmsd(const QImage& frame) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QImage m_reference_image;
    FloatPixel m_average_brightness;

    double m_max_rmsd;
    bool m_scale_brightness;

    Color m_color;
    ImageFloatBox m_box;
};


class ImageMatchWatcher : public ImageMatchDetector, public VisualInferenceCallback{
public:
    ImageMatchWatcher(
        QImage reference_image, const ImageFloatBox& box,
        double max_rmsd, bool scale_brightness = false,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(0),
        Color color = COLOR_RED
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
   ) override;

private:
    std::chrono::milliseconds m_hold_duration;

    bool m_last_match;
    std::chrono::system_clock::time_point m_start_of_match;
};




}
#endif
