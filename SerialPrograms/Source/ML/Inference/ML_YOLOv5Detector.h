/*  YOLOv5 Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ML_YOLOv5Detector_H
#define PokemonAutomation_ML_YOLOv5Detector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "ML/Models/ML_YOLOv5Model.h"

namespace PokemonAutomation{

class VideoOverlay;

namespace ML{


class YOLOv5Detector : public StaticScreenDetector{
public:
    using DetectionBox = YOLOv5Session::DetectionBox;
    // - model_path: path to the onnx model file. Can be a relative path to `RESOURCE_PATH()`.
    // If model loading fails, InternalProgramError exception is thrown
    YOLOv5Detector(const std::string& model_path);
    virtual ~YOLOv5Detector();

    // If it loads the model successfully
    bool model_loaded() const { return m_yolo_session != nullptr; }

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool detect(const ImageViewRGB32& screen) override;

    const std::vector<DetectionBox>& detected_boxes() const { return m_output_boxes; }
    std::vector<DetectionBox>& detected_boxes(){ return m_output_boxes; }

    const std::unique_ptr<YOLOv5Session>& session() const { return m_yolo_session; }

    const std::string& label_name(size_t label_idx) const;
    size_t label_index(const std::string& label_name) const;

protected:
    std::string m_model_path;
    bool m_use_gpu;
    // std::vector<std::string> m_labels;
    std::unique_ptr<YOLOv5Session> m_yolo_session;
    std::vector<DetectionBox> m_output_boxes;
};



class YOLOv5Watcher : public VisualInferenceCallback{
public:
    using DetectionBox = YOLOv5Session::DetectionBox;
    // - model_path: path to the onnx model file. Can be a relative path to `RESOURCE_PATH()`.
    // If model loading fails, InternalProgramError exception is thrown
    YOLOv5Watcher(VideoOverlay& overlay, const std::string& model_path);
    virtual ~YOLOv5Watcher() {}

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Thread-safe: Any thread can read this detection result
    std::vector<DetectionBox> detected_boxes();

    const std::string& label_name(size_t label_idx) const {return m_detector.label_name(label_idx);}
    size_t label_index(const std::string& label_name) const {return m_detector.label_index(label_name);}

protected:
    VideoOverlaySet m_overlay_set;
    YOLOv5Detector m_detector;

    std::vector<DetectionBox> m_output_boxes;
    SpinLock m_output_lock;  // Protects m_output_boxes
};



}
}
#endif
