/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Compiler.h"
#include "PokemonLA_Tests.h"
#include "TestUtils.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleSpriteWatcher.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_TransparentDialogueDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleStartDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_BattleSpriteArrowDetector.h"
#include "PokemonLA/Inference/PokemonLA_BerryTreeDetector.h"
#include "PokemonLA/Inference/PokemonLA_BlackOutDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMarkerLocator.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapWeatherAndTimeReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"

#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch;
using namespace NintendoSwitch::PokemonLA;

int test_pokemonLA_BattleMenuDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    BattleMenuDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BattlePokemonSwitchDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    BattlePokemonSwitchDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_TransparentDialogueDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    TransparentDialogueDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_EventDialogDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    EventDialogDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_DialogueYellowArrowDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    DialogueYellowArrowDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BlackOutDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    BlackOutDetector detector(logger, overlay);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BattleStartDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    BattleStartDetector detector(logger, overlay);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_MMOQuestionMarkDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
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
        }else if (region_kw_found){
            // Found "Region" keyword, read a number as how many MMOs on the region map, and a region name.
            if (target_region_index < 0){
                for(size_t index = 0; index < 5; index++){
                    if (word == WILD_REGION_SHORT_NAMES[index]){
                        target_region_index = (int)index;
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
            if (result != target){
                cerr << "Error: " << __func__ << " result on region " << i << " is " << result << " but should be " << target << "." << endl;
                return 1;
            }
        }
    }else{ // Region keyword found
        if (target_region_index < 0 || target_num_MMOs_on_region_map < 0){
            cerr << "Error: need a region name and a number of MMOs in the filename (e.g. image-Fieldlands_5.png)." << endl; 
        }
        const auto results = detector.detect_MMOs_on_region_map(image);
        TEST_RESULT_EQUAL((int)results.size(), target_num_MMOs_on_region_map);
    }

    return 0;
}

int read_pokemon_info_from_words(const std::vector<std::string>& keywords, Language& language, PokemonDetails& details){
    // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>

    if (keywords.size() < 5){
        cerr << "Error: not enough number of keywords in the filename to generate PokemonDetails. Found only " << keywords.size() << "." << endl;
        return 1;
    }

    language = language_code_to_enum(keywords[keywords.size()-5]);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language keyword " << keywords[keywords.size()-5] << " is wrong." << endl;
        return 1;
    }

    const std::string& pokemon_slug = keywords[keywords.size()-4];
    details.name_candidates.insert(pokemon_slug);

    const std::string& shiny_word = keywords[keywords.size()-3];
    if (shiny_word == "Shiny"){
        details.is_shiny = true;
    }else if (shiny_word == "NotShiny"){
        details.is_shiny = false;
    }else{
        cerr << "Error: shiny keyword " << shiny_word << " is wrong. Must be \"Shiny\" or \"NotShiny\"." << endl;
        return 1;
    }

    const std::string& alpha_word = keywords[keywords.size()-2];
    if (alpha_word == "Alpha"){
        details.is_alpha = true;
    }else if (alpha_word == "NotAlpha"){
        details.is_alpha = false;
    }else{
        cerr << "Error: alpha keyword " << alpha_word << " is wrong. Must be \"Alpha\" or \"NotAlpha\"." << endl;
        return 1;
    }

    const std::string& gender_word = keywords[keywords.size()-1];
    if (gender_word == "Male"){
        details.gender = Gender::Male;
    }else if (gender_word == "Female"){
        details.gender = Gender::Female;
    }else if (gender_word == "Genderless"){
        details.gender = Gender::Genderless;
    }else{
        cerr << "Error: gender keyword " << gender_word << " is wrong. Must be \"Male\", \"Female\" or \"Genderless\"." << endl;
        return 1;
    }

    return 0;
}

int test_pokemon_details(const PokemonDetails& details, const PokemonDetails& target){
    TEST_RESULT_COMPONENT_EQUAL(details.is_shiny, target.is_shiny, "shiny");
    TEST_RESULT_COMPONENT_EQUAL(details.is_alpha, target.is_alpha, "alpha");
    TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC(details.gender, target.gender, "gender", get_gender_str);

    const std::string& pokemon_slug = *target.name_candidates.begin();
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

int test_pokemonLA_StatusInfoScreenDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
    Language language = Language::None;
    PokemonDetails target;

    int ret = read_pokemon_info_from_words(keywords, language, target);
    if (ret != 0){
        return ret;
    }

    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const PokemonDetails details = read_status_info(logger, overlay, image, language);

    return test_pokemon_details(details, target);
}

int test_pokemonLA_WildPokemonFocusDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    // two keywords: <True/False> <True/False>
    if (keywords.size() < 2){
        cerr << "Error: not enough number of keywords in the filename. Found only " << keywords.size() << "." << endl;
        return 1;
    }

    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    WildPokemonFocusDetector detector(logger, overlay);
    bool result_has_focus = detector.process_frame(image, current_time());

    auto check_can_change_focus = [&](const std::string& word) -> int {
        bool result_can_change = detect_change_focus(logger, overlay, image);
        bool target_can_change = false;
        if (parse_bool(word, target_can_change) == false){
            cerr << "Error: True/False keyword " << word << " is wrong. Must be \"True\" or \"False\"." << endl;
            return 1;
        }
        TEST_RESULT_EQUAL(result_can_change, target_can_change);
        return 0;
    };

    bool target_has_focus = false;
    if (parse_bool(keywords[keywords.size()-2], target_has_focus) == true){
        TEST_RESULT_EQUAL(result_has_focus, target_has_focus);

        return check_can_change_focus(keywords[keywords.size()-1]);
    }

    // Or seven keywords:
    // the last seven keywords should be: <True/False> <True/False> <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
    if (keywords.size() < 7){
        cerr << "Error: not enough number of keywords in the filename. Found only " << keywords.size() << "." << endl;
        return 1;
    }

    if (parse_bool(keywords[keywords.size()-7], target_has_focus) == false){
        cerr << "Error: True/False keyword " << keywords[keywords.size()-7] << " is wrong. Must be \"True\" or \"False\"." << endl;
        return 1;
    }
    
    TEST_RESULT_EQUAL(result_has_focus, target_has_focus);

    if (result_has_focus){
        int ret = check_can_change_focus(keywords[keywords.size()-6]);
        if (ret != 0){
            return ret;
        }

        Language language = Language::None;
        PokemonDetails target_info;
        ret = read_pokemon_info_from_words(keywords, language, target_info);
        if (ret != 0){
            return ret;
        }

        const auto info = read_focused_wild_pokemon_info(logger, overlay, image, language);

        return test_pokemon_details(info, target_info);
    }

    return 0;
}


int test_pokemonLA_BattleSpriteWatcher(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    std::vector<bool> target;
    for(auto it = keywords.rbegin(); it != keywords.rend(); it++){
        const auto& word = *it;
        if (word == "0"){
            target.push_back(false);
        }else if (word == "1"){
            target.push_back(true);
        }else{
            break;
        }
    }
    target.resize(MAX_WILD_POKEMON_IN_MULTI_BATTLE, false);

    BattleSpriteWatcher watcher(logger, overlay);

    watcher.process_frame(image, current_time());

    auto result = watcher.sprites_appeared();

    for(size_t i = 0; i < MAX_WILD_POKEMON_IN_MULTI_BATTLE; i++){
        if (result[i] != target[i]){
            cerr << "Error: in test_pokemonLA_BattleSpriteWatcher sprite " << i << " has " << result[i] << ", but should be " << target[i] << endl;
            return 1;
        }
    }

    return 0;
}


int test_pokemonLA_MapMarkerLocator(const ImageViewRGB32& image, float target_angle, float threshold){
    float angle = get_orientation_on_map(image);
    TEST_RESULT_APPROXIMATE(angle, target_angle, threshold);
    return 0;
}

int test_pokemonLA_MapZoomLevelReader(const ImageViewRGB32& image, int target){
    int zoom = read_map_zoom_level(image);
    TEST_RESULT_EQUAL(zoom, target);
    return 0;
}

int test_pokemonLA_BattleSpriteArrowDetector(const ImageViewRGB32& image, int target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();

    const size_t target_index = target;
    for(size_t sprite_index = 0; sprite_index < MAX_WILD_POKEMON_IN_MULTI_BATTLE; sprite_index++){
        BattleSpriteArrowDetector detector(logger, overlay, sprite_index, std::chrono::milliseconds(0), true);
        bool result = detector.process_frame(image, current_time());
        if (sprite_index != target_index && result){
            cerr << "Error: in test_pokemonLA_BattleSpriteArrowDetector detected arrow at sprite index " << sprite_index
                 << " but should be at sprite index " << target << endl;
            return 1;
        }else if (sprite_index == target_index && result == false){
            cerr << "Error: in test_pokemonLA_BattleSpriteArrowDetector failed to detect arrow at sprite index " << sprite_index << endl;
            return 1;
        }
    }

    return 0;
}

int test_pokemonLA_MapMissionTabReader(const ImageViewRGB32& image, bool target){
    bool result = is_map_mission_tab_raised(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLA_BerryTreeDetector(const ImageViewRGB32& image){
    BerryTreeDetector detector;

    detector.process_frame(image, current_time());

    return 0;
}

int test_pokemonLA_SaveScreenDetector(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    // two keywords: <True/False> <True/False>
    if (keywords.size() < 2){
        cerr << "Error: not enough number of keywords in the filename. Found only " << keywords.size() << "." << endl;
        return 1;
    }

    bool target_save_tab = false;
    if (parse_bool(keywords[keywords.size()-2], target_save_tab) == false){
        cerr << "Error: True/False keyword " << keywords[keywords.size()-2] << " is wrong. Must be \"True\" or \"False\"." << endl;
        return 1;
    }
    bool target_save_disabled = false;
    if (parse_bool(keywords[keywords.size()-1], target_save_disabled) == false){
        cerr << "Error: True/False keyword " << keywords[keywords.size()-1] << " is wrong. Must be \"True\" or \"False\"." << endl;
        return 1;
    }

    bool save_tab = save_tab_selected(image);
    bool save_disabled = save_tab_disabled(image);
    
    TEST_RESULT_COMPONENT_EQUAL(save_tab, target_save_tab, "save tab");
    TEST_RESULT_COMPONENT_EQUAL(save_disabled, target_save_disabled, "save disabled");

    return 0;
}


int test_pokemonLA_shinySoundDetector(const std::vector<AudioSpectrum>& spectrums, bool target){
    auto& logger = global_logger_command_line();
    DummyBotBase botbase(logger);
    SerialPABotBase::SerialPABotBase_Connection connection(logger, nullptr, false);
    SerialPABotBase_WiredController controller(
        logger, connection,
        ControllerType::NintendoSwitch_WiredController,
        ControllerResetMode::DO_NOT_RESET
    );
    DummyVideoFeed video_feed;
    DummyVideoOverlay video_overlay;
    DummyAudioFeed audio_feed;
    StreamHistorySession history(logger);

    ConsoleHandle console(0, logger, controller, video_feed, video_overlay, audio_feed, history);
    ShinySoundDetector detector(console, [&](float error_coefficient) -> bool{
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
    auto& logger = global_logger_command_line();
    const QString full_path(QString::fromStdString(filepath));
    const QFileInfo fileinfo(full_path);
    const QString filename = fileinfo.fileName();
    const QDir parent_dir = fileinfo.dir();

    const std::string base_name = fileinfo.baseName().toStdString();

    const std::vector<std::string> filename_words = parse_words(base_name);
    MapRegion region = MapRegion::NONE;
    for(const std::string& word : filename_words){
        if (word == "Fieldlands"){
            region = MapRegion::FIELDLANDS;
            break;
        }else if (word == "Mirelands"){
            region = MapRegion::MIRELANDS;
            break;
        }else if (word == "Coastlands"){
            region = MapRegion::COASTLANDS;
            break;
        }else if (word == "Highlands"){
            region = MapRegion::HIGHLANDS;
            break;
        }else if (word == "Icelands"){
            region = MapRegion::ICELANDS;
            break;
        }
    }
    if (region == MapRegion::NONE){
        cout << "Error: filename should contain a region name (e.g. \"Fieldlands\")." << endl;
        return 1;
    } 

    const QString mmo_revealed_image_path = parent_dir.filePath("_" + filename);
    const QString mmo_revealed_txt_path = parent_dir.filePath("_" + fileinfo.baseName() + ".txt");

    ImageRGB32 question_mark_image(filepath);
    ImageRGB32 sprite_image(mmo_revealed_image_path.toStdString());
    
    if (!question_mark_image){
        cerr << "Error: cannot load MMO question mark image file " << filepath << endl;
        return 1;
    }
    if (!sprite_image){
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

    MMOQuestionMarkDetector detector(logger);

    std::vector<ImagePixelBox> quest_results = detector.detect_MMOs_on_region_map(question_mark_image);
    std::sort(quest_results.begin(), quest_results.end(), [](const ImagePixelBox& a, const ImagePixelBox& b) -> bool {
        if (a.center_y() < b.center_y()){
            return true;
        }
        if (a.center_y() > b.center_y()){
            return false;
        }
        return a.center_x() < b.center_x();
    });

    cout << "Detect MMO question marks:" << endl;
    for(const auto& box : quest_results){
        cout << "- " << box.center_x() << ", " << box.center_y() << " " << box.width() << " x " << box.height() << endl;
    }

    if (quest_results.size() != target_sprites.size()){
        cerr << "Error: the number of MMO question marks detected is not correct: " << quest_results.size() << " should be " << 
            target_sprites.size() << endl;
        return 1;
    }

    // static int count = 0;
    ImageRGB32 output_sprite = sprite_image.copy();
    ImageRGB32 output_quest = question_mark_image.copy();
    std::vector<ImagePixelBox> new_boxes;
    for (size_t i = 0; i < quest_results.size(); i++){
        auto box = quest_results[i];
        draw_box(output_quest, box, combine_rgb(255, 0, 0));
        draw_box(output_sprite, box, combine_rgb(255, 0, 0));

        pxint_t radius = (pxint_t)((box.width() + box.height()) / 4 + 0.5);
        pxint_t center_x = (pxint_t)box.center_x();
        pxint_t center_y = (pxint_t)box.center_y();
        auto new_box = ImagePixelBox(center_x - radius, center_y - radius, center_x + radius, center_y + radius);
        new_boxes.push_back(new_box);
        
        // std::ostringstream os;
        // os << "test_sprite_" << count << "_" << std::setfill('0') << std::setw(2) << i << ".png";
        // std::string sprite_filename = os.str();
        // extract_box_reference(sprite_image, new_box).save(sprite_filename);
    }
    // output_quest.save("test_MMO_question_mark_detection_" + std::to_string(count) + ".png");
    // output_sprite.save("test_sprite_detection_" + std::to_string(count) + ".png");

    size_t success_count = 0;
    for (size_t i = 0; i < quest_results.size(); i++){
        // XXX
        // if (i != 9){
            // continue;
        // }
        cout << "--------------------------------------------------------------------" << endl;
        cout << i << ": Target slug: " << target_sprites[i] << endl;

        bool debug_mode = false;
        auto result = match_sprite_on_map(logger, sprite_image, new_boxes[i], region, debug_mode);
        if (result.slug == target_sprites[i]){
            success_count++;
            cout << "Match SUCCESS" << endl;
        }else{
            cout << "Match FAILURE" << endl;
        }
    }

    if (success_count == target_sprites.size()){
        cout << "ALL SUCCESS" << endl;
    }else{
        cout << "FAILURE: " << target_sprites.size() - success_count << "/" << target_sprites.size() << endl;
        return 1;
    }
    // count++;
    
    return 0;
}

int test_pokemonLA_MapWeatherAndTimeReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    // two keywords: <Weather name> <Time of day name>
    if (keywords.size() < 2){
        cerr << "Error: not enough number of keywords in the filename. Found only " << keywords.size() << "." << endl;
        return 1;
    }

    const auto& target_weather = keywords[keywords.size()-2];
    const auto& target_time = keywords[keywords.size()-1];

    auto& logger = global_logger_command_line();
    const std::string weather_result = WEATHER_NAMES[(int)detect_weather_on_map(logger, image)];
    const std::string time_result = TIME_OF_DAY_NAMES[(int)detect_time_of_day_on_map(logger, image)];
    
    TEST_RESULT_COMPONENT_EQUAL(weather_result, target_weather, "weather");
    TEST_RESULT_COMPONENT_EQUAL(time_result, target_time, "time of day");

    
    return 0;
}

int test_pokemonLA_FlagTracker_performance(const ImageViewRGB32& image, int num_iterations){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    FlagTracker tracker(logger, overlay);

    auto time_start = current_time();
    for(int i = 0; i < num_iterations; i++){
        tracker.process_frame(image, current_time());
    }
    auto time_end = current_time();
    const double ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count() / (double)num_iterations;
    cout << "Time: " << ms << " ms, " << ms / 1000. << " s" << endl;

    return 0;
}

}
