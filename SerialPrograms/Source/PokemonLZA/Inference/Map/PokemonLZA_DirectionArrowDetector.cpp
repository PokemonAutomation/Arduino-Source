/* Direction Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA_DirectionArrowDetector.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include <opencv2/opencv.hpp>
#include <cmath>

// #define DEBUG_DIRECTION_ARROW

#ifdef DEBUG_DIRECTION_ARROW
#include <iostream>
using std::cout, std::endl;
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



DirectionArrowDetector::DirectionArrowDetector(Color color)
    : m_color(color)
    , m_search_box(0.076, 0.138, 0.031, 0.062)  // Default: search entire image
    , m_detected_angle(-1.0)
{
}

void DirectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_search_box);
}

bool DirectionArrowDetector::detect(const ImageViewRGB32& screen){
    m_detected_angle = -1.0;

    ImageViewRGB32 image_crop = extract_box_reference(screen, m_search_box);

    // Convert to OpenCV Mat (BGRA format)
    cv::Mat image = image_crop.to_opencv_Mat();
    if (image.empty()){
        return false;
    }

    // Convert BGRA to BGR (remove alpha channel)
    cv::Mat bgr;
    cv::cvtColor(image, bgr, cv::COLOR_BGRA2BGR);

    // Convert BGR to HSV for color thresholding
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // Define cyan/turquoise color range in HSV
    // Cyan is approximately 180° in hue (90 in OpenCV's 0-180 range)
    cv::Scalar lower_cyan(90, 200, 200);
    cv::Scalar upper_cyan(110, 255, 255);

    // Threshold the image to get cyan pixels
    cv::Mat mask;
    cv::inRange(hsv, lower_cyan, upper_cyan, mask);

#ifdef DEBUG_DIRECTION_ARROW
    // Save the cyan-filtered mask to verify color thresholding
    // white color in the debugging image should be the arrow pixels while the black color should be
    // background.
    cv::imwrite("./cyan_filter.png", mask);
    cout << "Saved cyan-filtered mask to ./cyan_filter.png" << endl;
#endif

    // Find connected components and select the largest one
    cv::Mat labels, stats, centroids;
    int num_components = cv::connectedComponentsWithStats(mask, labels, stats, centroids, 4, CV_32S);

    if (num_components <= 1){
        // No components found (component 0 is background)
#ifdef DEBUG_DIRECTION_ARROW
        cout << "No connected components found in cyan mask" << endl;
#endif
        return false;
    }

    // Find the largest connected component (excluding background at index 0)
    int largest_component = 1;
    int largest_area = stats.at<int>(1, cv::CC_STAT_AREA);

    for (int i = 2; i < num_components; ++i){
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area > largest_area){
            largest_area = area;
            largest_component = i;
        }
    }

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Found " << num_components - 1 << " connected components" << endl;
    cout << "Largest component: " << largest_component << " with " << largest_area << " pixels" << endl;

    // Save mask with only the largest component for debugging
    cv::Mat largest_comp_mask = cv::Mat::zeros(labels.size(), CV_8U);
    for (int y = 0; y < labels.rows; ++y){
        for (int x = 0; x < labels.cols; ++x){
            if (labels.at<int>(y, x) == largest_component){
                largest_comp_mask.at<uint8_t>(y, x) = 255;
            }
        }
    }
    cv::imwrite("./cyan_filter_largest_comp.png", largest_comp_mask);
    cout << "Saved largest component mask to ./cyan_filter_largest_comp.png" << endl;
#endif

    // Extract all pixels from the largest connected component for PCA
    std::vector<cv::Point2f> points;
    for (int y = 0; y < labels.rows; ++y){
        for (int x = 0; x < labels.cols; ++x){
            if (labels.at<int>(y, x) == largest_component){
                points.push_back(cv::Point2f(static_cast<float>(x), static_cast<float>(y)));
            }
        }
    }

    // Check if the largest component is the right size to be an arrow
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    // For 1080P screenshots, the arrow takes about 300 pixels
    double min_area_1080p = 250.0, max_area_1080p = 350.0;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);
    const size_t max_area = size_t(screen_rel_size_2 * max_area_1080p);

    if (static_cast<size_t>(largest_area) < min_area || static_cast<size_t>(largest_area) > max_area){
#ifdef DEBUG_DIRECTION_ARROW
        cout << "Largest component size out of range: " << largest_area << " (" << min_area << "," << max_area << ")" << endl;
#endif
        return false;
    }

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Using " << points.size() << " pixels from largest component for PCA" << endl;
#endif
    // Perform PCA
    cv::Mat data_pts = cv::Mat(static_cast<int>(points.size()), 2, CV_32F);
    for (size_t i = 0; i < points.size(); ++i){
        data_pts.at<float>(static_cast<int>(i), 0) = points[i].x;
        data_pts.at<float>(static_cast<int>(i), 1) = points[i].y;
    }

    cv::PCA pca_analysis(data_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);

    // Get the centroid and principal axis
    cv::Point2f center(pca_analysis.mean.at<float>(0, 0), pca_analysis.mean.at<float>(0, 1));
    cv::Point2f eigenvec(pca_analysis.eigenvectors.at<float>(0, 0), pca_analysis.eigenvectors.at<float>(0, 1));

#ifdef DEBUG_DIRECTION_ARROW
    cout << "PCA center (centroid): (" << center.x << ", " << center.y << ")" << endl;
    cout << "PCA principal eigenvector: (" << eigenvec.x << ", " << eigenvec.y << ")" << endl;
#endif

    // Determine arrow direction by counting pixels on each side of the eigenvector
    // Arrow has more pixels at the base (wide) than tip (pointy)
    // So if more pixels on positive side, arrow points to negative side
    int positive_side_count = 0;
    int negative_side_count = 0;

    for (const auto& pt : points){
        // Vector from center to this pixel
        cv::Point2f centered = pt - center;

        // Dot product with eigenvector determines which side
        float projection = centered.dot(eigenvec);

        if (projection > 0){
            positive_side_count++;
        } else {
            negative_side_count++;
        }
    }

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Pixels on positive side: " << positive_side_count << endl;
    cout << "Pixels on negative side: " << negative_side_count << endl;
#endif
    // If more pixels on positive side, the base is there, so arrow points negative
    // Flip the eigenvector to point towards the tip
    if (positive_side_count > negative_side_count){
        eigenvec = -eigenvec;
#ifdef DEBUG_DIRECTION_ARROW
        cout << "Arrow points in negative eigenvector direction (flipped)" << endl;
#endif
    } else {
#ifdef DEBUG_DIRECTION_ARROW
        cout << "Arrow points in positive eigenvector direction" << endl;
#endif
    }

    // Calculate and print eigenvector angle
    double eigenvec_angle_deg = std::atan2(eigenvec.y, eigenvec.x) * 180.0 / CV_PI + 90.0;
    while (eigenvec_angle_deg < 0.0){
        eigenvec_angle_deg += 360.0;
    }
    while (eigenvec_angle_deg >= 360.0){
        eigenvec_angle_deg -= 360.0;
    }

    // eigen vector angle is based on the x,y coord system of the image matrix
    // but human intuition of the angle is the clock angle. 
    // so: 0 -> 90, 270 -> 0
    // old angle a -> a + 90 % 360
#ifdef DEBUG_DIRECTION_ARROW
    cout << "Found angle: " << eigenvec_angle_deg << " degrees." << endl;
#endif
    // Store result
    m_detected_angle = eigenvec_angle_deg;

    return true;
}




}
}
}
