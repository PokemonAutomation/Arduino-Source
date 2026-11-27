/*  YOLO Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ML_YOLODetector_H
#define PokemonAutomation_ML_YOLODetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "ML/Models/ML_YOLOModel.h"

namespace PokemonAutomation{

class VideoOverlay;

namespace ML{


class YOLODetector : public StaticScreenDetector{
public:
    using DetectionBox = YOLOSession::DetectionBox;
    // - model_path: path to the onnx model file. Can be a relative path to `RESOURCE_PATH()`.
    // If model loading fails, InternalProgramError exception is thrown
    YOLODetector(const std::string& model_path);
    virtual ~YOLODetector();

    // If it loads the model successfully
    bool model_loaded() const { return m_yolo_session != nullptr; }

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool detect(const ImageViewRGB32& screen) override;

    const std::vector<DetectionBox>& detected_boxes() const { return m_output_boxes; }
    std::vector<DetectionBox>& detected_boxes(){ return m_output_boxes; }

    const std::unique_ptr<YOLOSession>& session() const { return m_yolo_session; }

    const std::string& label_name(size_t label_idx) const;
    size_t label_index(const std::string& label_name) const;

protected:
    std::string m_model_path;
    bool m_use_gpu;
    // std::vector<std::string> m_labels;
    std::unique_ptr<YOLOSession> m_yolo_session;
    std::vector<DetectionBox> m_output_boxes;
};



class YOLOWatcher : public VisualInferenceCallback{
public:
    using DetectionBox = YOLOSession::DetectionBox;
    // - model_path: path to the onnx model file. Can be a relative path to `RESOURCE_PATH()`.
    // If model loading fails, InternalProgramError exception is thrown
    YOLOWatcher(VideoOverlay& overlay, const std::string& model_path);
    virtual ~YOLOWatcher(){}

    virtual void make_overlays(VideoOverlaySet& items) const override {}
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Thread-safe: Any thread can read this detection result
    std::vector<DetectionBox> detected_boxes();

    const std::string& label_name(size_t label_idx) const {return m_detector.label_name(label_idx);}
    size_t label_index(const std::string& label_name) const {return m_detector.label_index(label_name);}

protected:
    VideoOverlaySet m_overlay_set;
    YOLODetector m_detector;

    std::vector<DetectionBox> m_output_boxes;
    SpinLock m_output_lock;  // Protects m_output_boxes
};



}
}
#endif
