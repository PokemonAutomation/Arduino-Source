/*  Test Dudunsparce Form Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "3rdParty/ONNX/OnnxToolsPA.h"
#include "Common/Cpp/Time.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Async/InferenceSession.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "TestDudunsparceFormDetector.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <onnxruntime_cxx_api.h>

#include <limits>
#include <vector>
#include <iostream>
using std::cout, std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

class DudunsparceFormDetector : public VisualInferenceCallback{
public:
    DudunsparceFormDetector(VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

    std::string get_label() const {
        int detected_label_id = m_detected.load(std::memory_order_acquire);
        return labels[detected_label_id];
    }

private:
    VideoOverlaySet m_overlay_set;
    std::string model_path;
    ImagePixelBox m_pixel_box_1080p;
    ImageFloatBox m_float_box;
    std::atomic<int> m_detected;

    Ort::Env env;
    Ort::RunOptions runOptions;
    Ort::Session session;

    const char* labels[3]{"none", "three", "two"};
};


DudunsparceFormDetector::DudunsparceFormDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("DudunsparceFormDetector")
    , m_overlay_set(overlay)
    , model_path("../../PAMLExperiments/dudunsparce/dudunsparce_form_detector.onnx")
    , m_detected(0)
    , session(nullptr)
{
    // The input data for the ML model is created by cropping an 1080P frame from Switch
    // The crop is at image[500:750, 1500:1750]. 
    m_pixel_box_1080p = ImagePixelBox(1500, 500, 1750, 750);
    m_float_box = pixelbox_to_floatbox(1920, 1080, m_pixel_box_1080p);

    // learned from ONN Runtime example: https://github.com/cassiebreviu/cpp-onnxruntime-resnet-console-app/blob/main/OnnxRuntimeResNet/OnnxRuntimeResNet.cpp
    session = Ort::Session(env, str_to_onnx_str(model_path).c_str(), Ort::SessionOptions{nullptr});
}

void DudunsparceFormDetector::make_overlays(VideoOverlaySet& items) const {}

bool DudunsparceFormDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_overlay_set.clear();

    ImageViewRGB32 cropped_frame = (frame.height() == 1080) ? extract_box_reference(frame, m_pixel_box_1080p)
        : extract_box_reference(frame, m_float_box);
    cv::Mat cropped_mat = cropped_frame.to_opencv_Mat();
    cv::Mat resized_mat;  // resize to the shape for the ML model input
    cv::resize(cropped_mat, resized_mat, cv::Size(25, 25));
    cv::Mat resized_mat_gray;
    cv::cvtColor(resized_mat, resized_mat_gray, cv::COLOR_BGRA2GRAY);

    // cv::imwrite("./model_input.png", resized_mat_gray);

    cv::Mat float_mat;
    resized_mat_gray.convertTo(float_mat, CV_32F, 1./255);

    // ML stuff: prepare ML model input and output
    const std::array<int64_t, 3> inputShape = {1, 25, 25};
    const std::array<int64_t, 2> outputShape = {1, 3};

    std::array<float, 25 * 25> modelInput;
    std::array<float, 3> modelOutput;
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    auto inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, modelInput.data(), modelInput.size(), inputShape.data(), inputShape.size());
    auto outputTensor = Ort::Value::CreateTensor<float>(memoryInfo, modelOutput.data(), modelOutput.size(), outputShape.data(), outputShape.size());

    for (int row = 0, p_loc=0; row < 25; row++){
        for(int col = 0; col < 25; col++){
            float p = float_mat.at<float>(row, col);
            modelInput[p_loc++] = p;
        }
    }

    Ort::AllocatorWithDefaultOptions ort_alloc;
    Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, ort_alloc);
    Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, ort_alloc);
    const std::array<const char*, 1> inputNames = {inputName.get()};
    const std::array<const char*, 1> outputNames = {outputName.get()};

    session.Run(runOptions, inputNames.data(), &inputTensor, 1, outputNames.data(), &outputTensor, 1);

    float max_value = -std::numeric_limits<float>::max();
    int max_value_label = 0;
    for (int i = 0; i < 3; i++){
        // cout << labels[i] << ": " << modelOutput[i] << ", ";
        if (modelOutput[i] > max_value){
            max_value = modelOutput[i];
            max_value_label = i;
        }
    }
    cout << "Detector dudunsparce form: " << labels[max_value_label] << endl;
    m_detected.store(max_value_label);

    return false;
}


TestDudunsparceFormDetector_Descriptor::TestDudunsparceFormDetector_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:DudunsparceFormDetector",
        "Nintendo Switch", "Test Dudunsparce Form Detector",
        "",
        "Test ML model on Dudunsparce form in SV box system",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}

TestDudunsparceFormDetector::TestDudunsparceFormDetector(){}


void TestDudunsparceFormDetector::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    DudunsparceFormDetector detector(env.console.overlay());

    // ImageRGB32 test_image("../../datasets/Bidoof/images/test/im0005.png");
    // detector.process_frame(test_image, current_time());

    // return;
    // InferenceSession session(
    //     context, env.console,
    //     {{detector, std::chrono::milliseconds(100)}}
    // );
    // context.wait_until_cancel();

    std::string last_label = "";
    run_until<ProControllerContext>(
        env.console, context, [&](ProControllerContext& context){
            while (true){
                std::string cur_label = detector.get_label();
                if (cur_label != last_label){
                    last_label = cur_label;
                    env.console.overlay().add_log("Detected " + last_label);
                }
                context.wait_for(std::chrono::milliseconds(100));
            }
        },
        {detector},
        std::chrono::milliseconds(100)
    );


    std::cout << "ML detection test program finished." << std::endl;
}


}
}
