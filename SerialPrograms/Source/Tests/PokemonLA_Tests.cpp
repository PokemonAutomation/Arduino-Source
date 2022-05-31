/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "PokemonLA_Tests.h"
#include "TestUtils.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_TransparentDialogueDetector.h"
#include "PokemonLA/Inference/PokemonLA_BerryTreeDetector.h"
#include "PokemonLA/Inference/PokemonLA_BlackOutDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMarkerLocator.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"

#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonLA;

int test_pokemonLA_BattleMenuDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    BattleMenuDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BattlePokemonSwitchDetector(const QImage& image, bool target) {
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    BattlePokemonSwitchDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_TransparentDialogueDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    TransparentDialogueDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_DialogueYellowArrowDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    DialogueYellowArrowDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BlackOutDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    BlackOutDetector detector(logger, overlay);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_MMOQuestionMarkDetector(const QImage& image, const std::vector<std::string>& keywords){
    bool hisui_kw_found = false;
    std::array<bool, 5> target_hisui_region_has_MMO = {false};

    bool region_kw_found = false;
    int target_num_MMOs_on_region_map = -1;
    int target_region_index = -1;

    for(size_t keyword_index = 0; keyword_index < keywords.size(); keyword_index++){
        const std::string& word = keywords[keyword_index];
        if (hisui_kw_found == false && word == "Hisui"){
            hisui_kw_found = true;
            continue;
        }
        if (region_kw_found == false && word == "Region"){
            region_kw_found = true;
            continue;
        }

        if (hisui_kw_found){
            // We have found the "Hisui" keyword. So the next words will be the index of the region on the Hisui map
            // that has MMO.
            try{
                int region = std::stoi(word);
                if (region < 0 || region > 4){
                    cerr << "Error: wrong region number, must be [0, 4] but got " << region << endl;
                    return 1;
                }
                target_hisui_region_has_MMO[region] = true;
            } catch(std::exception&){
                cerr << "Error: keyword must be a region number, ranging in [0, 4], but got " << word << endl;
                return 1;
            }
        }
        else if (region_kw_found){
            // Found "Region" keyword, read a number as how many MMOs on the region map, and a region name.
            if (target_region_index < 0){
                for(size_t index = 0; index < 5; index++){
                    if (word == WILD_REGION_SHORT_NAMES[index]){
                        target_region_index = index;
                        break;
                    }
                }
                if (target_region_index >= 0){
                    continue;
                }
            }
            if (target_num_MMOs_on_region_map < 0){
                try{
                    int num_MMOs = std::stoi(word);
                    if (num_MMOs < 0){
                        cerr << "Error: wrong number " << num_MMOs << ", must be non-negative" << endl;
                        return 1;
                    }
                    target_num_MMOs_on_region_map = num_MMOs;
                } catch(std::exception&){}
            }
        }
    }

    if (hisui_kw_found == false && region_kw_found == false){
        cerr << "Error: need keyword \"Hisui\" or \"Region\" in filename/" << endl;
        return 1;
    }

    auto& logger = global_logger_command_line();
    MMOQuestionMarkDetector detector(logger);

    if (hisui_kw_found){
        const auto region_has_MMO = detector.detect_MMO_on_hisui_map(image);

        for(size_t i = 0; i < 5; i++){
            const bool result = region_has_MMO[i];
            const bool target = target_hisui_region_has_MMO[i];
            if (result != target) {
                cerr << "Error: " << __func__ << " result on region " << i << " is " << result << " but should be " << target << "." << endl;
                return 1;
            }
        }
    } else { // Region keyword found
        if (target_region_index < 0 || target_num_MMOs_on_region_map < 0){
            cerr << "Error: need a region name and a number of MMOs in the filename (e.g. image-Fieldlands-5.png)." << endl; 
        }
        const auto results = detector.detect_MMOs_on_region_map(image);
        TEST_RESULT_EQUAL(results.size(), target_num_MMOs_on_region_map);
    }

    return 0;
}

int test_pokemonLA_StatusInfoScreenDetector(const QImage& image, const std::vector<std::string>& keywords){
    // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
    if (keywords.size() < 5){
        cerr << "Error: not enough number of keywords in the filename, found only " << keywords.size() << "." << endl;
        return 1;
    }

    const Language language = language_code_to_enum(keywords[keywords.size()-5]);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language keyword " << keywords[keywords.size()-5] << " is wrong." << endl;
        return 1;
    }

    const std::string& pokemon_slug = keywords[keywords.size()-4];

    const std::string& shiny_word = keywords[keywords.size()-3];
    bool is_shiny = true;
    if (shiny_word == "Shiny"){
        is_shiny = true;
    } else if (shiny_word == "NotShiny"){
        is_shiny = false;
    } else{
        cerr << "Error: shiny keyword " << shiny_word << " is wrong. Must be \"Shiny\" or \"NotShiny\"." << endl;
        return 1;
    }

    const std::string& alpha_word = keywords[keywords.size()-2];
    bool is_alpha = true;
    if (alpha_word == "Alpha"){
        is_alpha = true;
    } else if (alpha_word == "NotAlpha"){
        is_alpha = false;
    } else{
        cerr << "Error: alpha keyword " << alpha_word << " is wrong. Must be \"Alpha\" or \"NotAlpha\"." << endl;
        return 1;
    }

    const std::string& gender_word = keywords[keywords.size()-1];

    Gender gender = Gender::Genderless;
    if (gender_word == "Male"){
        gender = Gender::Male;
    } else if (gender_word == "Female"){
        gender = Gender::Female;
    } else if (gender_word == "Genderless"){
        gender = Gender::Genderless;
    } else {
        cerr << "Error: gender keyword " << gender_word << " is wrong. Must be \"Male\", \"Female\" or \"Genderless\"." << endl;
        return 1;
    }

    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    const PokemonDetails details = read_status_info(logger, overlay, image, language);

    TEST_RESULT_COMPONENT_EQUAL(details.is_shiny, is_shiny, "shiny");
    TEST_RESULT_COMPONENT_EQUAL(details.is_alpha, is_alpha, "alpha");
    TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC(details.gender, gender, "gender", get_gender_str);

    bool found_name = false;
    for(const auto& slg : details.name_candidates){
        if (slg == pokemon_slug){
            found_name = true;
            break;
        }
    }
    if (found_name == false){
        cerr << "Error: " << __func__ << " name result is ";
        for(const auto& slg : details.name_candidates){
            cerr << slg << ", ";
        }
        cerr << "but should be " << pokemon_slug << "." << endl;
        return 1;
    }

    return 0;
}

int test_pokemonLA_MapMarkerLocator(const QImage& image, float target_angle, float threshold){
    float angle = get_orientation_on_map(image);
    TEST_RESULT_APPROXIMATE(angle, target_angle, threshold);
    return 0;
}

int test_pokemonLA_MapZoomLevelReader(const QImage& image, int target){
    int zoom = read_map_zoom_level(image);
    TEST_RESULT_EQUAL(zoom, target);
    return 0;
}

void test_pokemonLA_BerryTreeDetector(const QImage& image){
    BerryTreeDetector detector;

    detector.process_frame(image, current_time());
}


int test_pokemonLA_shinySoundDetector(const std::vector<AudioSpectrum>& spectrums, bool target){
    auto& logger = global_logger_command_line();
    DummyBotBase botbase(logger);
    DummyVideoFeed video_feed;
    DummyVideoOverlay video_overlay;
    DummyAudioFeed audio_feed;

    ConsoleHandle console(0, logger, botbase, video_feed, video_overlay, audio_feed);
    ShinySoundDetector detector(logger, console, [&](float error_coefficient) -> bool{
        return true;
    });

    bool result = detector.process_spectrums(spectrums, audio_feed);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

// Load an image with MMO question marks from an MMO event, with filename <XXX.png>
// Load an image with MMO question marks revealed by Munchlax to show each pokemon sprite from the same MMO event, with filename <_XXX.png>
// Load a text file with each line the pokemon in the MMO event, with filename <_XXX.txt>. If more than one pokemon of the same species appears,
// add a number as the number of appearance at end of that line.
int test_pokemonLA_MMOSpriteMatcher(const std::string& filepath){
    const QString full_path(filepath.c_str());
    const QFileInfo fileinfo(full_path);
    const QString filename = fileinfo.fileName();
    const QDir parent_dir = fileinfo.dir();

    const QString mmo_revealed_image_path = parent_dir.filePath("_" + filename);
    const QString mmo_revealed_txt_path = parent_dir.filePath("_" + fileinfo.baseName() + ".txt");

    QImage question_mark_image(full_path);
    QImage sprite_image(mmo_revealed_image_path);
    
    if (question_mark_image.isNull()){
        cerr << "Error: cannot load MMO quesiton mark image file " << filepath << endl;
        return 1;
    }
    if (sprite_image.isNull()){
        cerr << "Error: cannot load MMO revealed sprites image file " << mmo_revealed_image_path.toStdString() << endl;
        return 1;
    }

    std::vector<std::string> target_sprites;
    if (load_slug_list(mmo_revealed_txt_path.toStdString(), target_sprites) == false){
        return 1;
    }

    cout << "Target sprites: " << target_sprites.size() << " total" << endl;
    for(const auto& slug : target_sprites){
        cout << "- " << slug << endl;
    }

    auto& logger = global_logger_command_line();
    MMOQuestionMarkDetector detector(logger);

    const auto quest_results = detector.detect_MMOs_on_region_map(question_mark_image);
    cout << "Detect MMO question marks:" << endl;
    for(const auto& box : quest_results){
        cout << "- " << box.center_x() << ", " << box.center_y() << " " << box.width() << " x " << box.height() << endl;
    }

    if (quest_results.size() != target_sprites.size()){
        cerr << "Error: the number of MMO question marks detected is not correct: " << quest_results.size() << " should be " << 
            target_sprites.size() << endl;
        return 1;
    }
    
    static int count = 0;
    QImage output_sprite = sprite_image;
    QImage output_quest = question_mark_image;
    std::vector<ImagePixelBox> new_boxes;
    for (size_t i = 0; i < quest_results.size(); i++){
        auto box = quest_results[i];
        draw_box(output_quest, box, combine_rgb(255, 0, 0));
        draw_box(output_sprite, box, combine_rgb(255, 0, 0));

        int radius = int((box.width() + box.height()) / 4 + 0.5);
        int center_x = box.center_x();
        int center_y = box.center_y();
        auto new_box = ImagePixelBox(center_x - radius, center_y - radius, center_x + radius, center_y + radius);
        new_boxes.push_back(new_box);
        
        std::ostringstream os;
        os << "test_sprite_" << count << "_" << std::setfill('0') << std::setw(2) << i << ".png";
        std::string sprite_filename = os.str();
        extract_box_reference(sprite_image, new_box).save(QString::fromStdString(sprite_filename));
    }
    output_quest.save("test_MMO_question_mark_detection_" + QString::number(count) + ".png");
    output_sprite.save("test_sprite_detection_" + QString::number(count) + ".png");

    size_t success_count = 0;
    for (size_t i = 0; i < quest_results.size(); i++){
        // XXX
        // if (i != 9){
            // continue;
        // }
        cout << "--------------------------------------------------------------------" << endl;
        cout << i << ": Target slug: " << target_sprites[i] << endl;

        auto result = match_sprite_on_map(sprite_image, new_boxes[i]);
        if (result.slug == target_sprites[i]){
            success_count++;
            cout << "Match SUCCESS" << endl;
        } else{
            cout << "Match FAILURE" << endl;
        }
    }

    if (success_count == target_sprites.size()){
        cout << "ALL SUCCESS" << endl;
    }
    count++;
    
    return 0;
}

}
