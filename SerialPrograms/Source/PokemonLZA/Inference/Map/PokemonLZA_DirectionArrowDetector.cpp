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
    cout << "Using " << points.size() << " pixels from largest component" << endl;
#endif

    // Find convex hull to get the arrow outline
    std::vector<cv::Point> points_int;
    points_int.reserve(points.size());
    for (const auto& pt : points){
        points_int.push_back(cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)));
    }

    std::vector<cv::Point> hull;
    cv::convexHull(points_int, hull);

    // Calculate centroid of arrow pixels
    cv::Point2f arrow_center(0, 0);
    for (const auto& pt : points){
        arrow_center.x += pt.x;
        arrow_center.y += pt.y;
    }
    arrow_center.x /= points.size();
    arrow_center.y /= points.size();

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Arrow center: (" << arrow_center.x << ", " << arrow_center.y << ")" << endl;
    cout << "Convex hull has " << hull.size() << " points" << endl;
#endif

    // Create a mask for the convex hull
    cv::Mat hull_mask = cv::Mat::zeros(labels.size(), CV_8U);
    std::vector<std::vector<cv::Point>> hull_contours;
    hull_contours.push_back(hull);
    cv::fillPoly(hull_mask, hull_contours, cv::Scalar(255));

    // Find background pixels inside the convex hull
    std::vector<cv::Point2f> background_pixels_in_hull;
    for (int y = 0; y < labels.rows; ++y){
        for (int x = 0; x < labels.cols; ++x){
            // Check if pixel is inside hull but not part of the arrow
            if (hull_mask.at<uint8_t>(y, x) > 0 && labels.at<int>(y, x) != largest_component){
                background_pixels_in_hull.push_back(cv::Point2f(static_cast<float>(x), static_cast<float>(y)));
            }
        }
    }

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Found " << background_pixels_in_hull.size() << " background pixels inside convex hull" << endl;

    // Save convex hull visualization
    cv::Mat hull_image = cv::Mat::zeros(labels.size(), CV_8UC3);
    // Draw the filled arrow region in white
    for (int y = 0; y < labels.rows; ++y){
        for (int x = 0; x < labels.cols; ++x){
            if (labels.at<int>(y, x) == largest_component){
                hull_image.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
            }
        }
    }
    // Draw convex hull in green
    cv::drawContours(hull_image, hull_contours, 0, cv::Scalar(0, 255, 0), 2);

    // Draw background pixels inside hull in blue
    for (const auto& bg_pt : background_pixels_in_hull){
        int x = static_cast<int>(bg_pt.x);
        int y = static_cast<int>(bg_pt.y);
        if (x >= 0 && x < hull_image.cols && y >= 0 && y < hull_image.rows){
            hull_image.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 0, 0); // Blue
        }
    }

    // Draw arrow center in red
    cv::circle(hull_image, cv::Point(static_cast<int>(arrow_center.x), static_cast<int>(arrow_center.y)),
               3, cv::Scalar(0, 0, 255), -1);

    cv::imwrite("./convex_hull.png", hull_image);
    cout << "Saved convex hull visualization to ./convex_hull.png (before finding furthest pixel)" << endl;
#endif

    if (background_pixels_in_hull.empty()){
#ifdef DEBUG_DIRECTION_ARROW
        cout << "No background pixels found inside convex hull" << endl;
#endif
        return false;
    }

    // Compute average vector from arrow center to background pixels
    cv::Point2f avg_vector(0, 0);
    for (const auto& bg_pt : background_pixels_in_hull){
        cv::Point2f vec = bg_pt - arrow_center;
        avg_vector.x += vec.x;
        avg_vector.y += vec.y;
    }
    avg_vector.x /= background_pixels_in_hull.size();
    avg_vector.y /= background_pixels_in_hull.size();

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Average vector to background pixels: (" << avg_vector.x << ", " << avg_vector.y << ")" << endl;
#endif

    // Negate the average vector to get arrow direction
    // (background pixels are at the base/notch, opposite to arrow direction)
    cv::Point2f direction = -avg_vector;

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Initial arrow direction vector (negated): (" << direction.x << ", " << direction.y << ")" << endl;
#endif

    // Normalize the direction vector
    float dir_length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (dir_length < 0.001f){
#ifdef DEBUG_DIRECTION_ARROW
        cout << "Direction vector too small to normalize" << endl;
#endif
        return false;
    }
    cv::Point2f normalized_direction(direction.x / dir_length, direction.y / dir_length);

    // Find the arrow pixel that is furthest along this direction
    float max_projection = -std::numeric_limits<float>::max();
    cv::Point2f furthest_pixel;

    for (const auto& pt : points){
        cv::Point2f vec = pt - arrow_center;
        float projection = vec.dot(normalized_direction);

        if (projection > max_projection){
            max_projection = projection;
            furthest_pixel = pt;
        }
    }

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Furthest pixel along direction: (" << furthest_pixel.x << ", " << furthest_pixel.y << ")" << endl;
    cout << "Projection distance: " << max_projection << endl;
#endif

    // Refine the direction by using vector from arrow center to furthest pixel
    cv::Point2f refined_direction = furthest_pixel - arrow_center;

#ifdef DEBUG_DIRECTION_ARROW
    cout << "Refined arrow direction vector: (" << refined_direction.x << ", " << refined_direction.y << ")" << endl;

    // Create final visualization with arrow direction
    cv::Mat final_image = cv::Mat::zeros(labels.size(), CV_8UC3);
    // Draw the filled arrow region in white
    for (int y = 0; y < labels.rows; ++y){
        for (int x = 0; x < labels.cols; ++x){
            if (labels.at<int>(y, x) == largest_component){
                final_image.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
            }
        }
    }
    // Draw convex hull in green
    std::vector<std::vector<cv::Point>> hull_viz;
    hull_viz.push_back(hull);
    cv::drawContours(final_image, hull_viz, 0, cv::Scalar(0, 255, 0), 1);

    // Draw arrow center in red
    cv::circle(final_image, cv::Point(static_cast<int>(arrow_center.x), static_cast<int>(arrow_center.y)),
               3, cv::Scalar(0, 0, 255), -1);

    // Draw furthest pixel in cyan
    cv::circle(final_image, cv::Point(static_cast<int>(furthest_pixel.x), static_cast<int>(furthest_pixel.y)),
               3, cv::Scalar(255, 255, 0), -1);

    // Draw arrow direction line from center to furthest pixel in yellow
    cv::line(final_image,
             cv::Point(static_cast<int>(arrow_center.x), static_cast<int>(arrow_center.y)),
             cv::Point(static_cast<int>(furthest_pixel.x), static_cast<int>(furthest_pixel.y)),
             cv::Scalar(0, 255, 255), 2);

    cv::imwrite("./arrow_direction.png", final_image);
    cout << "Saved arrow direction visualization to ./arrow_direction.png" << endl;
#endif

    // Calculate angle from refined direction vector
    double eigenvec_angle_deg = std::atan2(refined_direction.y, refined_direction.x) * 180.0 / CV_PI + 90.0;
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
