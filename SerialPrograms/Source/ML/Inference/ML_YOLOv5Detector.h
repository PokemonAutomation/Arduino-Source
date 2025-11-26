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
    // - model_path: path to the onnx model file. The label name file should be the same
    //   file path and basename and with _label.txt suffix.
    //   e.g. .../yolo.onnx, .../yolo_label.txt
    // If model loading fails, InternalProgramError exception is thrown
    YOLOv5Detector(const std::string& model_path);
    virtual ~YOLOv5Detector();

    // If it loads the model successfully
    bool model_loaded() const { return m_yolo_session != nullptr; }

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool detect(const ImageViewRGB32& screen) override;

    const std::vector<YOLOv5Session::DetectionBox>& detected_boxes() const { return m_output_boxes; }

    const std::unique_ptr<YOLOv5Session>& session() const { return m_yolo_session; }

protected:
    std::string m_model_path;
    bool m_use_gpu;
    // std::vector<std::string> m_labels;
    std::unique_ptr<YOLOv5Session> m_yolo_session;
    std::vector<YOLOv5Session::DetectionBox> m_output_boxes;
};



class YOLOv5Watcher : public VisualInferenceCallback{
public:
    // - model_path: path to the onnx model file. The label name file should be the same
    //   file path and basename and with _label.txt suffix.
    //   e.g. .../yolo.onnx, .../yolo_label.txt
    // If model loading fails, InternalProgramError exception is thrown
    YOLOv5Watcher(VideoOverlay& overlay, const std::string& model_path);
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
