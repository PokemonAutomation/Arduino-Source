/*  Color Clustering
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "ColorClustering.h"

#include <fstream>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

void PixelEuclideanStatAccumulator::clear(){
    m_count = 0;
    m_sum_x = 0;
    m_sum_y = 0;
    m_sum_z = 0;
    m_sqr_x = 0;
    m_sqr_y = 0;
    m_sqr_z = 0;
}
void PixelEuclideanStatAccumulator::operator+=(FloatPixel pixel){
    m_count++;
    m_sum_x += pixel.r;
    m_sum_y += pixel.g;
    m_sum_z += pixel.b;
    m_sqr_x += pixel.r * pixel.r;
    m_sqr_y += pixel.g * pixel.g;
    m_sqr_z += pixel.b * pixel.b;
}
uint64_t PixelEuclideanStatAccumulator::count() const{
    return m_count;
}
FloatPixel PixelEuclideanStatAccumulator::center() const{
    return FloatPixel(m_sum_x / m_count, m_sum_y / m_count, m_sum_z / m_count);
}
double PixelEuclideanStatAccumulator::deviation() const{
    if (m_count < 1){
        return 0;
    }
    double variance = m_sqr_x + m_sqr_y + m_sqr_z;
    variance -= (m_sum_x*m_sum_x + m_sum_y*m_sum_y + m_sum_z*m_sum_z) / m_count;
    return std::sqrt(variance / (m_count - 1));
}


double cluster_fit_2(
    const ImageViewRGB32& image,
    Color color0, PixelEuclideanStatAccumulator& cluster0,
    Color color1, PixelEuclideanStatAccumulator& cluster1
){
    size_t width = image.width();
    size_t height = image.height();

    FloatPixel f0(color0);
    FloatPixel f1(color1);

    PixelEuclideanStatAccumulator stats0;
    PixelEuclideanStatAccumulator stats1;

//    image.save("points.png");
//    std::fstream ss("points.txt", std::fstream::out);

    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            Color pixel(image.pixel(c, r));
//            ss << "{" << (int)pixel.red() << ", " << (int)pixel.green() << ", " << (int)pixel.blue() << "}," << endl;
            FloatPixel p0 = f0 - pixel;
            FloatPixel p1 = f1 - pixel;
            double distance0 = (p0 * p0).sum();
            double distance1 = (p1 * p1).sum();
            if (distance0 < distance1){
                stats0 += pixel;
//                cout << (int)pixel.red() << ", " << (int)pixel.green() << ", " << (int)pixel.blue() << ": " << std::sqrt(distance0) << endl;
            }else{
                stats1 += pixel;
//                cout << (int)pixel.red() << ", " << (int)pixel.green() << ", " << (int)pixel.blue() << ": " << std::sqrt(distance1) << endl;
//                cout << "stats = " << stats1.center() << endl;
            }
        }
    }

#if 0
    cout << "color0 = " << stats0.count() << ": " << stats0.center() << ", " << stats0.deviation() << endl;
    cout << "color1 = " << stats1.count() << ": " << stats1.center() << ", " << stats1.deviation() << endl;
#endif

    cluster0 = stats0;
    cluster1 = stats1;

//    cout << "stats = " << stats1.center() << endl;

    return (stats0.deviation() * stats0.count() + stats1.deviation() * stats1.count()) / (stats0.count() + stats1.count());
}

bool cluster_fit_2(
    const ImageViewRGB32& image,
    Color color0, double ratio0,
    Color color1, double ratio1,
    double ratio_threshold,
    double deviation_threshold,
    double distance_threshold
){
    const size_t NUM_CLUSTERS = 2;

    if (!image){
        return false;
    }
    size_t total = image.width() * image.height();

    FloatPixel center_desired[NUM_CLUSTERS] = {color0, color1};
    double count_ratio_desired[NUM_CLUSTERS] = {ratio0, ratio1};

    PixelEuclideanStatAccumulator cluster[NUM_CLUSTERS];
    cluster_fit_2(image, color0, cluster[0], color1, cluster[1]);
    color0 = cluster[0].center().round();
    color1 = cluster[1].center().round();
    double deviation = cluster_fit_2(image, color0, cluster[0], color1, cluster[1]);
//    cout << "deviation = " << deviation << ", threshold = " << deviation_threshold << endl;
//    cout << cluster[0].count() << " / " << cluster[1].count() << endl;
    if (deviation > deviation_threshold){
//        cout << "deviation out" << endl;
//        image.save("test.png");
        return false;
    }

    FloatPixel center_actual[NUM_CLUSTERS];
    double count_ratio_actual[NUM_CLUSTERS];

    double ratio_sum = 0;
    for (size_t c = 0; c < NUM_CLUSTERS; c++){
        ratio_sum += count_ratio_desired[c];
        center_actual[c] = cluster[c].center();
    }
//    cout << "---------------" << endl;
    for (size_t c = 0; c < NUM_CLUSTERS; c++){
//        cout << cluster[c].count() << ": " << cluster[c].center() << ", " << cluster[c].deviation() << endl;

        count_ratio_desired[c] /= ratio_sum;
        count_ratio_actual[c] = (double)cluster[c].count() / total;
        double diff = std::abs(count_ratio_actual[c] - count_ratio_desired[c]);
//        cout << "Color Ratio Diff: " << diff << endl;
        if (diff > ratio_threshold){
//            cout << "Color Ratio Diff is Too Large: " << diff << endl;
//            cout << "desired = " << count_ratio_desired[c] << endl;
//            cout << "actual = " << count_ratio_actual[c] << endl;
            return false;
        }

        if (center_desired[c].sum() < 50){
//            cout << "Black" << endl;
            double distance = euclidean_distance(center_desired[c], center_actual[c]) * count_ratio_desired[c];
            if (distance > 40){
//                cout << "Black Distance is Too Large: " << distance << endl;
                return false;
            }
        }else{
//            cout << "Not Black" << endl;
            FloatPixel ratio_desired = center_desired[c] / center_desired[c].sum();
            FloatPixel ratio_actual = center_actual[c] / center_actual[c].sum();
            double distance = euclidean_distance(ratio_desired, ratio_actual);
//            cout << ratio_desired << " / " << ratio_actual << " = " << distance << endl;
            if (distance > distance_threshold){
//                cout << distance << " > " << distance_threshold << endl;
                return false;
            }
        }
    }

//    cout << "match" << endl;
    return true;
}




}

