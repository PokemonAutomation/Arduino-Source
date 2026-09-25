/*  ML Paddle OCR Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Rectangle.tpp"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "ML/Models/ML_ONNXRuntimeHelpers.h"
#include "ML_PaddleOCRPipeline.h"
#include "ML_PaddleOCRDetector.h"

namespace PokemonAutomation{
namespace ML{



Rectangle<float> PaddleOCRTextBox::extent() const{
    return Rectangle<float>(
        std::min({corners[0].x, corners[1].x, corners[2].x, corners[3].x}),
        std::min({corners[0].y, corners[1].y, corners[2].y, corners[3].y}),
        std::max({corners[0].x, corners[1].x, corners[2].x, corners[3].x}),
        std::max({corners[0].y, corners[1].y, corners[2].y, corners[3].y})
    );
}
ImagePixelBox PaddleOCRTextBox::pixel_box(size_t image_width, size_t image_height) const{
    const Rectangle<float> e = extent();
    auto to_pixel = [](float v){
        return (size_t)std::max(0.0f, v);
    };
    ImagePixelBox box(
        to_pixel(std::floor(e.min_x)),
        to_pixel(std::floor(e.min_y)),
        to_pixel(std::ceil(e.max_x)),
        to_pixel(std::ceil(e.max_y))
    );
    box.clip(image_width, image_height);
    box.min_x = std::min(box.min_x, box.max_x);
    box.min_y = std::min(box.min_y, box.max_y);
    return box;
}

namespace{

cv::Rect to_cv_rect(const ImagePixelBox& box){
    return cv::Rect((int)box.min_x, (int)box.min_y, (int)box.width(), (int)box.height());
}

cv::Point2f to_cv_point(const PaddleOCRPoint& p){
    return cv::Point2f(p.x, p.y);
}

}



namespace{

// Order the four corners of a rotated rectangle as top-left, top-right, bottom-right,
// bottom-left. This is PaddleOCR's `get_mini_boxes()` ordering: sort by x, then the two
// leftmost points are split into top/bottom by y, and likewise for the two rightmost.
std::array<PaddleOCRPoint, 4> ordered_corners(const cv::RotatedRect& rect){
    cv::Point2f pts[4];
    rect.points(pts);
    std::sort(pts, pts + 4, [](const cv::Point2f& a, const cv::Point2f& b){
        return a.x < b.x;
    });
    cv::Point2f tl = pts[0], bl = pts[1];
    if (tl.y > bl.y){
        std::swap(tl, bl);
    }
    cv::Point2f tr = pts[2], br = pts[3];
    if (tr.y > br.y){
        std::swap(tr, br);
    }
    return {
        PaddleOCRPoint{tl.x, tl.y},
        PaddleOCRPoint{tr.x, tr.y},
        PaddleOCRPoint{br.x, br.y},
        PaddleOCRPoint{bl.x, bl.y},
    };
}

// Mean of the probability map inside the polygon `corners`. This is PaddleOCR's
// `box_score_fast()`: rasterize the polygon as a mask over its bounding rectangle and
// average the probabilities under the mask.
float box_score(const cv::Mat& prob, const std::array<PaddleOCRPoint, 4>& corners){
    float min_x = corners[0].x, max_x = corners[0].x;
    float min_y = corners[0].y, max_y = corners[0].y;
    for (const PaddleOCRPoint& p : corners){
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }
    int x0 = std::clamp((int)std::floor(min_x), 0, prob.cols - 1);
    int x1 = std::clamp((int)std::ceil(max_x), 0, prob.cols - 1);
    int y0 = std::clamp((int)std::floor(min_y), 0, prob.rows - 1);
    int y1 = std::clamp((int)std::ceil(max_y), 0, prob.rows - 1);

    cv::Mat mask = cv::Mat::zeros(y1 - y0 + 1, x1 - x0 + 1, CV_8UC1);
    std::vector<cv::Point> poly;
    for (const PaddleOCRPoint& p : corners){
        poly.emplace_back((int)std::round(p.x) - x0, (int)std::round(p.y) - y0);
    }
    cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{poly}, cv::Scalar(1));

    cv::Mat roi = prob(cv::Rect(x0, y0, x1 - x0 + 1, y1 - y0 + 1));
    return (float)cv::mean(roi, mask)[0];
}

// Expand a rotated rectangle outward by `distance = area * unclip_ratio / perimeter`.
// PaddleOCR does this with a polygon offset (pyclipper) followed by `minAreaRect()`.
// For a rectangle input, offsetting every edge outward by `distance` and taking the
// minimum bounding rectangle of the rounded result is exactly a rectangle with both
// sides grown by `2 * distance`, so we can compute it directly.
cv::RotatedRect unclip(const cv::RotatedRect& rect, float unclip_ratio){
    float w = rect.size.width;
    float h = rect.size.height;
    float area = w * h;
    float perimeter = 2 * (w + h);
    if (perimeter <= 0){
        return rect;
    }
    float distance = area * unclip_ratio / perimeter;
    return cv::RotatedRect(rect.center, cv::Size2f(w + 2 * distance, h + 2 * distance), rect.angle);
}

// Angle of the top edge of the box in degrees. 0 for axis-aligned text.
double box_angle_degrees(const PaddleOCRTextBox& box){
    const PaddleOCRPoint& tl = box.corners[0];
    const PaddleOCRPoint& tr = box.corners[1];
    return std::atan2(tr.y - tl.y, tr.x - tl.x) * 180.0 / CV_PI;
}

// Boxes rotated by less than this are treated as axis-aligned.
constexpr double AXIS_ALIGNED_MAX_ANGLE = 5.0;

// Round `x` to the nearest multiple of 32 (DBNet downsamples by 32), at least 32.
int round_to_32(double x){
    return std::max(32, (int)std::round(x / 32.0) * 32);
}

} // anonymous namespace



PaddleOCRDetector::PaddleOCRDetector(const std::string& model_path, PaddleOCRDetectorParams params)
    : m_params(params)
    , m_session(
        create_session(
            model_path,
            ML_MODEL_CACHE_PATH() + "PaddleOCRDetector/",
            PerformanceOptions::instance().ONNX_OPTIONS.USE_GPU
        )
    )
    , m_input_name(m_session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
    , m_output_name(m_session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions{}).get())
{}


std::vector<PaddleOCRTextBox> PaddleOCRDetector::detect(const cv::Mat& image_rgb) const{
    if (image_rgb.empty() || image_rgb.type() != CV_8UC3){
        return {};
    }

    const int image_w = image_rgb.cols;
    const int image_h = image_rgb.rows;

    // 1. Pad the border with the background color so text touching the edge of a
    //    pre-cropped box is still detected.
    cv::Mat padded;
    const int pad = std::max(0, m_params.border_padding);
    if (pad > 0){
        cv::copyMakeBorder(
            image_rgb, padded, pad, pad, pad, pad,
            cv::BORDER_CONSTANT, estimate_background_color(image_rgb)
        );
    }else{
        padded = image_rgb;
    }

    // 2. Resize so that the shorter side >= min_side_len and the longer side <=
    //    max_side_len, with both sides rounded to multiples of 32.
    double ratio = 1.0;
    const int short_side = std::min(padded.cols, padded.rows);
    const int long_side = std::max(padded.cols, padded.rows);
    if (short_side < m_params.min_side_len){
        ratio = (double)m_params.min_side_len / short_side;
    }
    if (long_side * ratio > m_params.max_side_len){
        ratio = (double)m_params.max_side_len / long_side;
    }
    const int input_w = round_to_32(padded.cols * ratio);
    const int input_h = round_to_32(padded.rows * ratio);

    cv::Mat resized;
    cv::resize(padded, resized, cv::Size(input_w, input_h), 0, 0, cv::INTER_LINEAR);

    // 3. Normalize. PaddleOCR reads images as BGR with cv2 and applies ImageNet
    //    mean/std in that channel order, so we convert to BGR to match its training.
    cv::cvtColor(resized, resized, cv::COLOR_RGB2BGR);
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);
    cv::subtract(resized, cv::Scalar(0.485, 0.456, 0.406), resized);
    cv::divide(resized, cv::Scalar(0.229, 0.224, 0.225), resized);

    // 4. HWC -> NCHW and run the model.
    std::vector<float> input_values = preprocess_NCHW(resized);
    std::vector<int64_t> input_shape = {1, 3, input_h, input_w};

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = create_tensor<float>(memory_info, input_values, input_shape);

    const char* input_names[] = {m_input_name.c_str()};
    const char* output_names[] = {m_output_name.c_str()};

    std::vector<Ort::Value> outputs;
    try{
        outputs = m_session.Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1
        );
    }catch (Ort::Exception& e){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "PaddleOCRDetector::detect(): Failed. " + std::string(e.what())
        );
    }

    // 5. The output is a text probability map of shape {1, 1, H, W}.
    std::vector<int64_t> output_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    if (output_shape.size() != 4 || output_shape[2] != input_h || output_shape[3] != input_w){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "PaddleOCRDetector::detect(): Unexpected output shape " + to_string(output_shape) +
            " for input shape " + to_string(input_shape)
        );
    }
    cv::Mat prob(input_h, input_w, CV_32FC1, outputs[0].GetTensorMutableData<float>());

    // Map model input pixels -> padded image pixels -> original image pixels.
    const float scale_x = (float)padded.cols / input_w;
    const float scale_y = (float)padded.rows / input_h;
    std::vector<PaddleOCRTextBox> boxes = postprocess(
        prob, scale_x, scale_y,
        (float)pad,
        image_w, image_h
    );

    if (STATIC_GLOBALS.PADDLE_OCR_DEBUG){
        global_logger_tagged().log(
            "[OCR-DET-DEBUG] Input " + std::to_string(image_w) + "x" + std::to_string(image_h) +
            " -> model " + std::to_string(input_w) + "x" + std::to_string(input_h) +
            ", found " + std::to_string(boxes.size()) + " text boxes."
        );
    }
    return boxes;
}


std::vector<PaddleOCRTextBox> PaddleOCRDetector::postprocess(
    const cv::Mat& prob,
    float scale_x, float scale_y,
    float offset,
    int image_width, int image_height
) const{
    // 1. Binarize the probability map.
    cv::Mat bitmap;
    cv::threshold(prob, bitmap, m_params.binary_threshold, 255, cv::THRESH_BINARY);
    bitmap.convertTo(bitmap, CV_8UC1);

    // 2. Each connected text blob becomes a candidate.
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bitmap, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    std::vector<PaddleOCRTextBox> boxes;
    const size_t num_candidates = std::min(contours.size(), m_params.max_candidates);
    for (size_t i = 0; i < num_candidates; i++){
        const std::vector<cv::Point>& contour = contours[i];
        if (contour.size() < 3){
            continue;
        }

        // 3. Fit a rotated rectangle and score it against the probability map.
        cv::RotatedRect rect = cv::minAreaRect(contour);
        if (std::min(rect.size.width, rect.size.height) < m_params.min_box_size){
            continue;
        }
        float score = box_score(prob, ordered_corners(rect));
        if (score < m_params.box_threshold){
            continue;
        }

        // 4. Expand the shrunk text kernel to cover the full text.
        cv::RotatedRect expanded = unclip(rect, m_params.unclip_ratio);
        if (std::min(expanded.size.width, expanded.size.height) < m_params.min_box_size + 2){
            continue;
        }

        // 5. Map back to the original image and clip.
        PaddleOCRTextBox box;
        box.score = score;
        std::array<PaddleOCRPoint, 4> corners = ordered_corners(expanded);
        for (size_t c = 0; c < 4; c++){
            float x = corners[c].x * scale_x - offset;
            float y = corners[c].y * scale_y - offset;
            box.corners[c].x = std::clamp(x, 0.0f, (float)image_width);
            box.corners[c].y = std::clamp(y, 0.0f, (float)image_height);
        }

        //  Skip boxes that ended up entirely inside the padding.
        ImagePixelBox bounds = box.pixel_box(image_width, image_height);
        if (bounds.width() < 2 || bounds.height() < 2){
            continue;
        }
        boxes.emplace_back(box);
    }
    return boxes;
}



std::vector<std::vector<size_t>> group_text_boxes_into_lines(const std::vector<PaddleOCRTextBox>& boxes){
    std::vector<size_t> order(boxes.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b){
        Rectangle<float> ea = boxes[a].extent();
        Rectangle<float> eb = boxes[b].extent();
        return ea.min_y + ea.max_y < eb.min_y + eb.max_y;
    });

    struct Line{
        float min_y;
        float max_y;
        std::vector<size_t> members;
    };
    std::vector<Line> lines;
    for (size_t index : order){
        const PaddleOCRTextBox& box = boxes[index];
        const Rectangle<float> extent = box.extent();
        const float box_min = extent.min_y;
        const float box_max = extent.max_y;
        const float box_h = box_max - box_min;

        // Find the line with the most vertical overlap.
        Line* best = nullptr;
        float best_overlap = 0;
        for (Line& line : lines){
            float overlap = std::min(box_max, line.max_y) - std::max(box_min, line.min_y);
            float min_h = std::min(box_h, line.max_y - line.min_y);
            if (min_h > 0 && overlap >= 0.5f * min_h && overlap > best_overlap){
                best = &line;
                best_overlap = overlap;
            }
        }
        if (best == nullptr){
            lines.push_back(Line{box_min, box_max, {index}});
        }else{
            best->members.push_back(index);
            best->min_y = std::min(best->min_y, box_min);
            best->max_y = std::max(best->max_y, box_max);
        }
    }

    std::sort(lines.begin(), lines.end(), [](const Line& a, const Line& b){
        return a.min_y + a.max_y < b.min_y + b.max_y;
    });

    std::vector<std::vector<size_t>> ret;
    for (Line& line : lines){
        std::sort(line.members.begin(), line.members.end(), [&](size_t a, size_t b){
            return boxes[a].extent().min_x < boxes[b].extent().min_x;
        });
        ret.emplace_back(std::move(line.members));
    }
    return ret;
}


std::vector<PaddleOCRTextBox> merge_text_boxes_in_line(
    const std::vector<PaddleOCRTextBox>& line_boxes,
    float max_gap_ratio
){
    if (line_boxes.empty()){
        return {};
    }

    Rectangle<float> line_extent = line_boxes[0].extent();
    for (const PaddleOCRTextBox& box : line_boxes){
        line_extent.merge_with(box.extent());
    }
    const float line_min_y = line_extent.min_y;
    const float line_max_y = line_extent.max_y;
    const float max_gap = max_gap_ratio * (line_max_y - line_min_y);

    // A segment being built: its horizontal extent and the sum of the box scores.
    struct Segment{
        float min_x;
        float max_x;
        float score_sum;
        size_t count;
    };
    std::vector<PaddleOCRTextBox> ret;
    std::optional<Segment> current;
    auto flush = [&](){
        if (!current){
            return;
        }
        PaddleOCRTextBox merged;
        merged.corners = {
            PaddleOCRPoint{current->min_x, line_min_y},
            PaddleOCRPoint{current->max_x, line_min_y},
            PaddleOCRPoint{current->max_x, line_max_y},
            PaddleOCRPoint{current->min_x, line_max_y},
        };
        merged.score = current->score_sum / current->count;
        ret.emplace_back(merged);
        current.reset();
    };

    for (const PaddleOCRTextBox& box : line_boxes){
        if (std::abs(box_angle_degrees(box)) >= AXIS_ALIGNED_MAX_ANGLE){
            flush();
            ret.emplace_back(box);
            continue;
        }
        const Rectangle<float> extent = box.extent();
        if (current && extent.min_x - current->max_x <= max_gap){
            current->max_x = std::max(current->max_x, extent.max_x);
            current->score_sum += box.score;
            current->count++;
            continue;
        }
        flush();
        current = Segment{extent.min_x, extent.max_x, box.score, 1};
    }
    flush();
    return ret;
}


cv::Mat crop_text_box(const cv::Mat& image_rgb, const PaddleOCRTextBox& box){
    const cv::Point2f tl = to_cv_point(box.corners[0]);
    const cv::Point2f tr = to_cv_point(box.corners[1]);
    const cv::Point2f br = to_cv_point(box.corners[2]);
    const cv::Point2f bl = to_cv_point(box.corners[3]);

    // Game UI text is axis-aligned, but `minAreaRect()` on pixel text blobs often returns
    // a box rotated by a degree or so. Warping by that tiny angle resamples the image
    // and blurs sharp pixel fonts (e.g. FRLG), which noticeably hurts recognition.
    // So if the box is nearly axis-aligned, crop its bounding rectangle directly.
    if (std::abs(box_angle_degrees(box)) < AXIS_ALIGNED_MAX_ANGLE){
        ImagePixelBox rect = box.pixel_box(image_rgb.cols, image_rgb.rows);
        if (rect.width() < 1 || rect.height() < 1){
            return {};
        }
        return image_rgb(to_cv_rect(rect)).clone();
    }

    int crop_w = (int)std::round(std::max(cv::norm(tl - tr), cv::norm(bl - br)));
    int crop_h = (int)std::round(std::max(cv::norm(tl - bl), cv::norm(tr - br)));
    if (crop_w < 1 || crop_h < 1){
        return {};
    }

    const cv::Point2f src[4] = {tl, tr, br, bl};
    const cv::Point2f dst[4] = {
        {0, 0},
        {(float)crop_w, 0},
        {(float)crop_w, (float)crop_h},
        {0, (float)crop_h},
    };
    cv::Mat transform = cv::getPerspectiveTransform(src, dst);

    cv::Mat cropped;
    cv::warpPerspective(
        image_rgb, cropped, transform, cv::Size(crop_w, crop_h),
        cv::INTER_CUBIC, cv::BORDER_REPLICATE
    );
    return cropped;
}



}
}
