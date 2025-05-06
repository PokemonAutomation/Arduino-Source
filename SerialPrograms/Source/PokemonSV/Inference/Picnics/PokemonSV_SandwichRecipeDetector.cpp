/*  Sandwich Recipe Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/imgproc.hpp>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_SandwichRecipeDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class FixedLimitVector<NintendoSwitch::PokemonSV::GradientArrowWatcher>;

namespace NintendoSwitch{
namespace PokemonSV{

SandwichRecipeNumberDetector::SandwichRecipeNumberDetector(Logger& logger, Color color): m_logger(logger), m_color(color){
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 2; x++){
            // m_arrow_boxes[y*2+x] = ImageFloatBox(x * 0.26 + 0.103, y * 0.275 + 0.074, 0.068, 0.085);
            m_id_boxes[y*2+x] = ImageFloatBox(x * 0.26 + 0.015, y * 0.277 + 0.240, 0.042, 0.048);
        }
    }
}

void SandwichRecipeNumberDetector::make_overlays(VideoOverlaySet& items) const{
    for(int i = 0; i < 6; i++){
        // items.add(m_color, m_arrow_boxes[i]);
        items.add(m_color, m_id_boxes[i]);
    }   
}

void SandwichRecipeNumberDetector::detect_recipes(const ImageViewRGB32& screen, size_t recipe_IDs[6]) const{
    for(int i = 0; i < 6; i++){
        auto cropped_image = extract_box_reference(screen, m_id_boxes[i]);

        const bool invert_blackwhite = true;
        ImageRGB32 filterd_image = to_blackwhite_rgb32_range(
            cropped_image,
            invert_blackwhite,
            combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)
        );
        
        // filterd_image.save("./tmp_fil_" + std::to_string(i) + ".png");

        ImageRGB32 dilated_image(filterd_image.width(), filterd_image.height());

        if (screen.width() >= 1280){
            const int dilation_type = cv::MORPH_ELLIPSE;
            const int dilation_size = 1;

            cv::Mat element = cv::getStructuringElement(dilation_type,
                cv::Size(2*dilation_size + 1, 2*dilation_size+1),
                cv::Point(dilation_size, dilation_size));

            // filterd_image.save("./tmp_" + std::to_string(i) + ".png");
            
            cv::Mat filtered_image_mat(static_cast<int>(filterd_image.height()), static_cast<int>(filterd_image.width()), CV_8UC4, (void*)filterd_image.data(), filterd_image.bytes_per_row());
            
            cv::Mat dilated_image_mat(static_cast<int>(dilated_image.height()), static_cast<int>(dilated_image.width()), CV_8UC4, (void*)dilated_image.data(), dilated_image.bytes_per_row());
            cv::dilate(filtered_image_mat, dilated_image_mat, element);
        }else{
            dilated_image = filterd_image.copy();
        }

        // dilated_image.save("./tmp_dil_" + std::to_string(i) + ".png");
        
        const int number = OCR::read_number(m_logger, dilated_image);
        if (number <= 0 || number > 151){
            recipe_IDs[i] = 0;
        }else{
            recipe_IDs[i] = number;
        }
    }

    // Fix any broken OCR reads:
    for (int i = 1; i < 5; i++){
        if (recipe_IDs[i+1] > 0 && recipe_IDs[i-1] > 0 && recipe_IDs[i-1] + 2 == recipe_IDs[i+1]){
            if (recipe_IDs[i] != recipe_IDs[i-1] + 1){
                recipe_IDs[i] = recipe_IDs[i-1] + 1;
                m_logger.log("Fix recipe number at cell " + std::to_string(i) + " to be " + std::to_string(recipe_IDs[i]));
            }
        }
    }
    if (recipe_IDs[1] == 2 && recipe_IDs[2] == 3){
        if (recipe_IDs[0] != 1){
            recipe_IDs[0] = 1;
            m_logger.log("Fix recipe number at cell 0 to be 1");
        }
    }
}


SandwichRecipeSelectionWatcher::SandwichRecipeSelectionWatcher(Color color)
: VisualInferenceCallback("SandwichRecipeSelectionWatcher"), m_arrow_watchers(6){
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 2; x++){
            ImageFloatBox box(x * 0.26 + 0.103, y * 0.275 + 0.074, 0.068, 0.085);
            m_arrow_watchers.emplace_back(color, GradientArrowType::DOWN, box);
        }
    }
}

void SandwichRecipeSelectionWatcher::make_overlays(VideoOverlaySet& items) const{
    for(int i = 0; i < 6; i++){
        m_arrow_watchers[i].make_overlays(items);
    }
}

bool SandwichRecipeSelectionWatcher::process_frame(const VideoSnapshot& frame){
    int num_arrows_found = 0;
    for(int i = 0; i < 6; i++){
        const bool found_arrow = m_arrow_watchers[i].process_frame(frame);
        if (found_arrow){
            m_selected_recipe = i;
            num_arrows_found++;
        }
    }
    return num_arrows_found == 1;
}

bool SandwichRecipeSelectionWatcher::detect(const ImageViewRGB32& frame){
    int num_arrows_found = 0;
    for(int i = 0; i < 6; i++){
        const bool found_arrow = m_arrow_watchers[i].detect(frame);
        if (found_arrow){
            m_selected_recipe = i;
            num_arrows_found++;
        }
    }
    return num_arrows_found == 1;
}

}
}
}
