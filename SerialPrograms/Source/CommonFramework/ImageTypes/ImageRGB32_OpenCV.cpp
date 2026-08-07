/*  Image (RGB32) Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/opencv.hpp>
#include "Common/Cpp/FileIO.h"
#include "CommonFramework/Logging/Logger.h"
#include "ImageRGB32_OpenCV.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ImageRGB32 OpenCV_scale_image(const ImageViewRGB32& image, size_t width, size_t height){
    ImageRGB32 ret(width, height);

    cv::Mat src(
        (int)image.height(),
        (int)image.width(),
        CV_8UC4,
        (uint8_t*)image.data(),
        image.bytes_per_row()
    );
    cv::Mat dst(
        (int)height,
        (int)width,
        CV_8UC4,
        (uint8_t*)ret.data(),
        ret.bytes_per_row()
    );

    cv::resize(src, dst, dst.size(), 0, 0, cv::INTER_LINEAR);

    return ret;
}

ImageRGB32 OpenCV_load_image(const std::string& filename){
    std::string data = file_to_string(filename);

    cv::Mat img = cv::imdecode(
        cv::_InputArray(data.data(), (int)data.size()),
        cv::IMREAD_UNCHANGED
    );

//    cout << "Image: " << filename << endl;
    if (img.empty()){
        global_logger_tagged().log("Image is empty:" + filename);
        return ImageRGB32();
    }

    if (img.type() != CV_8UC4){
        switch (img.channels()) {
        case 3: // BGR or RGB (OpenCV type 16)
            // This adds a 100% opaque alpha channel (A=255) automatically
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
            break;

        case 1: // Grayscale (OpenCV type 0)
            // This replicates the gray channel to B, G, and R, and adds A=255
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
            break;

        case 2: // Rare format: Grayscale + Alpha
            global_logger_tagged().log("2-channel image is not supported:" + filename);
            return ImageRGB32();

        default:
            // Fallback for exotic formats (like 16-bit or float images):
            // Force depth scale down to 8-bit, then recurse or convert
            img.convertTo(img, CV_8UC4);
        }
    }

    auto ret = ImageRGB32(std::make_unique<ImageRGB32OpenCV>(std::move(img)));

    return ret;
}
bool OpenCV_save_image(const ImageViewRGB32& image, const std::string& filename){
    Filesystem::Path path(filename);

//    cout << (void*)image.data() << endl;
//    cout << "width = " << image.width() << endl;
//    cout << "height = " << image.height() << endl;
//    cout << "step = " << image.bytes_per_row() << endl;

    cv::Mat argb32(
        (int)image.height(),
        (int)image.width(),
        CV_8UC4,
        (uint8_t*)image.data(),
        image.bytes_per_row()
    );
//    cv::Mat abgr32;
//    cv::cvtColor(argb32, abgr32, cv::COLOR_RGB2BGR);

    std::vector<uint8_t> compressed_buffer;
    bool success = cv::imencode(path.extension().string(), argb32, compressed_buffer);
    if (!success || compressed_buffer.empty()){
        global_logger_tagged().log("Failed to save image:" + filename);
        return false;
    }

    FileIO file(path, FileMode::WRITE | FileMode::BINARY);
    size_t bytes_written = file.write(compressed_buffer.data(), compressed_buffer.size());
    if (bytes_written < compressed_buffer.size()){
        global_logger_tagged().log("Failed to save image:" + filename);
        return false;
    }

    return true;
}





ImageRGB32OpenCV::ImageRGB32OpenCV(cv::Mat image)
    : m_image(std::move(image))
{}

ImageViewRGB32 ImageRGB32OpenCV::get_view() const{
    return ImageViewRGB32(
        (uint32_t*)m_image.data,
        m_image.step,
        m_image.cols,
        m_image.rows
    );
}




}
