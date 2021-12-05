/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageMatchDetector_H
#define PokemonAutomation_CommonFramework_ImageMatchDetector_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{


class ImageMatchDetector : public VisualInferenceCallback{
public:
    ImageMatchDetector(QImage reference_image, const ImageFloatBox& box, double max_rmsd);

    double rmsd(const QImage& frame);
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
   ) override;

private:
    QImage m_reference_image;
    ImageFloatBox m_box;
    double max_rmsd;
};




}
#endif
