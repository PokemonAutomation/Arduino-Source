/*  YOLOv5 Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ML_YOLOv5Detector_H
#define PokemonAutomation_ML_YOLOv5Detector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "ML/Models/ML_YOLOv5Model.h"

namespace PokemonAutomation{

class VideoOverlay;

namespace ML{


class YOLOv5Detector : public StaticScreenDetector{
public:
    YOLOv5Detector();
    virtual ~YOLOv5Detector();

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool detect(const ImageViewRGB32& screen) override;

    const std::vector<YOLOv5Session::DetectionBox>& detected_boxes() const { return m_output_boxes; }

    const std::unique_ptr<YOLOv5Session>& session() const { return m_yolo_session; }

protected:
    std::unique_ptr<YOLOv5Session> m_yolo_session;
    std::vector<YOLOv5Session::DetectionBox> m_output_boxes;
};



class YOLOv5Watcher : public VisualInferenceCallback{
public:
    YOLOv5Watcher(VideoOverlay& overlay);
    virtual ~YOLOv5Watcher() {}

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


protected:
    VideoOverlaySet m_overlay_set;
    YOLOv5Detector m_detector;
};



}
}
#endif
