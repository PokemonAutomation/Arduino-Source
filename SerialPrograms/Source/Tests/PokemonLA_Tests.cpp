/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "PokemonLA_Tests.h"
#include "TestUtils.h"
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
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"

#include <QImage>
#include <iostream>
#include <cmath>
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
    
    size_t keyword_index = 0;
    bool hisui_found = false;
    std::array<bool, 5> target_region_has_MMO = {false};
    for(; keyword_index < keywords.size(); keyword_index++){
        const std::string& word = keywords[keyword_index];
        if (hisui_found == false){
            if (word == "Hisui"){
                hisui_found = true;
            }
            continue;
        }

        // We have found the "Hisui" keyword. So the next words will be the index of the region on the Hisui map
        // that has MMO.
        try{
            int region = std::stoi(word);
            if (region < 0 || region > 4){
                cerr << "Error: wrong region number, must be [0, 4] but got " << region << endl;
                return 1;
            }
            target_region_has_MMO[region] = true;
        } catch(std::exception&){
            cerr << "Error: keyword must be a region number, ranging in [0, 4], but got " << word << endl;
            return 1;
        }
    }

    if (hisui_found == false){
        cerr << "Error: need keyword \"Hisui\" in filename/" << endl;
        return 1;
    }

    auto& logger = global_logger_command_line();

    MMOQuestionMarkDetector detector(logger);

    const auto region_has_MMO = detector.detect_MMO_on_hisui_map(image);

    for(size_t i = 0; i < 5; i++){
        const bool result = region_has_MMO[i];
        const bool target = target_region_has_MMO[i];
        if (result != target) {
            cerr << "Error: " << __func__ << " result on region " << i << " is " << result << " but should be " << target << "." << endl;
            return 1;
        }
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



}
