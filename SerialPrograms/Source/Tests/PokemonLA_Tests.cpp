/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "PokemonLA_Tests.h"
#include "TestUtils.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_BerryTreeDetector.h"

#include <QImage>


namespace PokemonAutomation{

int test_pokemonLA_BattleMenuDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    NintendoSwitch::PokemonLA::BattleMenuDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_DETECTOR(result, target);
}

int test_pokemonLA_BattlePokemonSwitchDetector(const QImage& image, bool target) {
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    NintendoSwitch::PokemonLA::BattlePokemonSwitchDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_DETECTOR(result, target);
}

int test_pokemonLA_DialogueEllipseDetector(const QImage& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    NintendoSwitch::PokemonLA::DialogueEllipseDetector detector(logger, overlay, std::chrono::milliseconds(0), stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_DETECTOR(result, target);
}

void test_pokemonLA_BerryTreeDetector(const QImage& image){
    NintendoSwitch::PokemonLA::BerryTreeDetector detector;

    detector.process_frame(image, current_time());
}



}