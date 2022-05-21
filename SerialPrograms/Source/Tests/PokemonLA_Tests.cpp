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
#include "PokemonLA/Inference/Battles/PokemonLA_TransparentDialogueDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"
#include "PokemonLA/Inference/PokemonLA_BerryTreeDetector.h"
#include "PokemonLA/Inference/PokemonLA_BlackOutDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"

#include <QImage>
#include <iostream>

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

int test_pokemonLA_StatusInfoScreenDetector(const QImage& image, const std::vector<std::string>& keywords){
    // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
    if (keywords.size() < 5){
        std::cout << "Error: not enough number of keywords in the filename, found only " << keywords.size() << "." << std::endl;
        return 1;
    }

    const Language language = language_code_to_enum(keywords[keywords.size()-5]);
    if (language == Language::None || language == Language::EndOfList){
        std::cout << "Error: language keyword " << keywords[keywords.size()-5] << " is wrong." << std::endl;
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
        std::cout << "Error: shiny keyword " << shiny_word << " is wrong. Must be \"Shiny\" or \"NotShiny\"." << std::endl;
        return 1;
    }

    const std::string& alpha_word = keywords[keywords.size()-2];
    bool is_alpha = true;
    if (alpha_word == "Alpha"){
        is_alpha = true;
    } else if (alpha_word == "NotAlpha"){
        is_alpha = false;
    } else{
        std::cout << "Error: alpha keyword " << alpha_word << " is wrong. Must be \"Alpha\" or \"NotAlpha\"." << std::endl;
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
        std:: cout << "Error: gender keyword " << gender_word << " is wrong. Must be \"Male\", \"Female\" or \"Genderless\"." << std::endl;
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
        std::cout << "Error: " << __func__ << " name result is ";
        for(const auto& slg : details.name_candidates){
            std::cout << slg << ", ";
        }
        std::cout << "but should be " << pokemon_slug << "." << std::endl;
        return 1;
    }

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