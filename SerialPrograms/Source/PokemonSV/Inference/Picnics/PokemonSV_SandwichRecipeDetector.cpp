/*  Sandwich Recipe Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "Tests/TestUtils.h"
#include "PokemonSV_SandwichRecipeDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class FixedLimitVector<NintendoSwitch::PokemonSV::GradientArrowWatcher>;

namespace NintendoSwitch{
namespace PokemonSV{

SandwichRecipeNumberDetector::SandwichRecipeNumberDetector(Logger& logger, Color color): m_logger(logger), m_color(color){
    for (int y = 0; y < 3; y++){
        for (int x = 0; x < 2; x++){
            // m_arrow_boxes[y*2+x] = ImageFloatBox(x * 0.26 + 0.103, y * 0.275 + 0.074, 0.068, 0.085);
            m_id_boxes[y*2+x] = ImageFloatBox(x * 0.26 + 0.015, y * 0.277 + 0.240, 0.042, 0.048);
        }
    }
}

void SandwichRecipeNumberDetector::make_overlays(VideoOverlaySet& items) const{
    for (int i = 0; i < 6; i++){
        // items.add(m_color, m_arrow_boxes[i]);
        items.add(m_color, m_id_boxes[i]);
    }   
}

void SandwichRecipeNumberDetector::detect_recipes(const ImageViewRGB32& screen, size_t recipe_IDs[6]) const{
    for (int i = 0; i < 6; i++){
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
    for (int y = 0; y < 3; y++){
        for (int x = 0; x < 2; x++){
            ImageFloatBox box(x * 0.26 + 0.103, y * 0.275 + 0.074, 0.068, 0.085);
            m_arrow_watchers.emplace_back(color, GradientArrowType::DOWN, box);
        }
    }
}

void SandwichRecipeSelectionWatcher::make_overlays(VideoOverlaySet& items) const{
    for (int i = 0; i < 6; i++){
        m_arrow_watchers[i].make_overlays(items);
    }
}

bool SandwichRecipeSelectionWatcher::process_frame(const VideoSnapshot& frame){
    int num_arrows_found = 0;
    for (int i = 0; i < 6; i++){
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
    for (int i = 0; i < 6; i++){
        const bool found_arrow = m_arrow_watchers[i].detect(frame);
        if (found_arrow){
            m_selected_recipe = i;
            num_arrows_found++;
        }
    }
    return num_arrows_found == 1;
}







class Test_SandwichRecipeDetector : public UnitTest{
public:
    Test_SandwichRecipeDetector(
        const std::string& image,
        std::vector<std::string> words
    )
        : UnitTest("PokemonSV::Test_SandwichRecipeDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // seven words: the sandwich recipe IDs on the screen. Order:
        // --------------------------------------
        // recipe_IDs[0]  |   recipe_IDs[1]
        // recipe_IDs[2]  |   recipe_IDs[3]
        // recipe_IDs[4]  |   recipe_IDs[5]
        // --------------------------------------
        // plus the current selected cell ID (range [0, 5]).
        if (m_words.size() < 7){
            std::stringstream ss;
            ss << "Error: not enough number of words in the filename. Found only " << m_words.size() << "." << std::endl;
            return ss.str();
        }

        int target_IDs[6] = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < 6; i++){
            const auto& word = m_words[m_words.size() + i - 7];
            if (parse_int(word, target_IDs[i]) == false || target_IDs[i] > 151){
                std::stringstream ss;
                ss << "Error: word " << m_words[m_words.size() + i - 7] << " is wrong. Must be an integer < 151. " << std::endl;
                return ss.str();
            }
        }

        SandwichRecipeNumberDetector detector(logger);

        size_t detected_IDs[6] = {0, 0, 0, 0, 0, 0};
        ImageRGB32 image(m_image);
        detector.detect_recipes(image, detected_IDs);

        for (int i = 0; i < 6; i++){
            if (target_IDs[i] < 0){
                continue;
            }
            TEST_RESULT_COMPONENT_EQUAL_STR(detected_IDs[i], (size_t)target_IDs[i], "recipe at cell " + std::to_string(i));
        }

        int target_selection = 0;
        if (parse_int(m_words[m_words.size()-1], target_selection) == false || target_selection < 0 || target_selection >= 6){
            std::stringstream ss;
            ss << "Error: word " << m_words[m_words.size()-1] << " is wrong. Must be an integer in range [0, 6). " << std::endl;
            return ss.str();
        }
        SandwichRecipeSelectionWatcher selection_watcher;
        bool result = selection_watcher.detect(image);

        TEST_RESULT_COMPONENT_EQUAL_STR(result, true, "SandwichRecipeSelectionWatcher::process_frame() result");

        int selected_cell = selection_watcher.selected_recipe_cell();

        TEST_RESULT_COMPONENT_EQUAL_STR(selected_cell, target_selection, "selected cell");

        return true;
    };

private:
    std::string m_image;
    std::vector<std::string> m_words;
};



void add_tests_SandwichRecipeDetector(UnitTestDatabase& database){
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/Korean_NewAccount_1_0_0_0_0_0_0.png",
        std::vector<std::string>{"1", "0", "0", "0", "0", "0", "0"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/NewAccount_1_12_20_0_84_0_0.png",
        std::vector<std::string>{"1", "12", "20", "0", "84", "0", "0"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/Nix_Sandwich_15_16_17_18_19_20_2.png",
        std::vector<std::string>{"15", "16", "17", "18", "19", "20", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/Sandwich_1_4_8_12_13_14_0.png",
        std::vector<std::string>{"1", "4", "8", "12", "13", "14", "0"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/Sandwich_-1_16_17_-1_-1_20_2.png",
        std::vector<std::string>{"-1", "16", "17", "-1", "-1", "20", "2"}
    );
//    database.add<Test_SandwichRecipeDetector>(
//        "PokemonSV/SandwichRecipeDetector/macOS/_Recipe_No_Ingredient_1_2_3_4_5_6_0.png",
//        std::vector<std::string>{"1", "2", "3", "4", "5", "6", "0"}
//    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_1_2_3_4_5_6_0.png",
        std::vector<std::string>{"1", "2", "3", "4", "5", "6", "0"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_1_2_3_4_5_6_1.png",
        std::vector<std::string>{"1", "2", "3", "4", "5", "6", "1"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_1_2_3_4_5_6_2.png",
        std::vector<std::string>{"1", "2", "3", "4", "5", "6", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_1_2_3_4_5_6_3.png",
        std::vector<std::string>{"1", "2", "3", "4", "5", "6", "3"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_3_4_5_6_7_8_2.png",
        std::vector<std::string>{"3", "4", "5", "6", "7", "8", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_7_8_9_10_11_12_2.png",
        std::vector<std::string>{"7", "8", "9", "10", "11", "12", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_15_16_17_18_19_20_2.png",
        std::vector<std::string>{"15", "16", "17", "18", "19", "20", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_41_42_43_44_45_46_2.png",
        std::vector<std::string>{"41", "42", "43", "44", "45", "46", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_75_76_77_78_79_80_2.png",
        std::vector<std::string>{"75", "76", "77", "78", "79", "80", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_147_148_149_150_151_0_0.png",
        std::vector<std::string>{"147", "148", "149", "150", "151", "0", "0"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_147_148_149_150_151_0_2.png",
        std::vector<std::string>{"147", "148", "149", "150", "151", "0", "2"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_147_148_149_150_151_0_3.png",
        std::vector<std::string>{"147", "148", "149", "150", "151", "0", "3"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_149_150_151_0_0_0_1.png",
        std::vector<std::string>{"149", "150", "151", "0", "0", "0", "1"}
    );
    database.add<Test_SandwichRecipeDetector>(
        "PokemonSV/SandwichRecipeDetector/macOS/Recipe_149_150_151_0_0_0_2.png",
        std::vector<std::string>{"149", "150", "151", "0", "0", "0", "2"}
    );

}




}
}
}
