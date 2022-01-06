/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageMatchDetector_H
#define PokemonAutomation_CommonFramework_ImageMatchDetector_H

#include <chrono>
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{

class ImageMatchDetector : public StaticScreenDetector{
public:
    ImageMatchDetector(
        QImage reference_image, const ImageFloatBox& box,
        double max_rmsd,
        Color color = COLOR_RED
    );

    double rmsd(const QImage& frame) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QImage m_reference_image;
    double m_max_rmsd;

    Color m_color;
    ImageFloatBox m_box;
};


class ImageMatchWatcher : public ImageMatchDetector, public VisualInferenceCallback{
public:
    ImageMatchWatcher(
        QImage reference_image, const ImageFloatBox& box,
        double max_rmsd,
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
