/*  Basic RNG manipulation 
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/RNG/PokemonSwSh_OrbeetleAttackAnimationDetector.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

Xoroshiro128PlusState find_rng_state(
    VideoStream& stream,
    ProControllerContext& context,
    bool save_screenshots,
    bool log_image_values
){
    OrbeetleAttackAnimationDetector detector(stream, context);
    uint64_t last_bits0 = 0;
    uint64_t last_bits1 = 0;

    for (size_t i = 0; i < 128; i++){
        context.wait_for_all_requests();
        std::string text = std::to_string(i + 1) + "/128";
        stream.log("RNG: Attack animation " + text);
        OrbeetleAttackAnimationDetector::Detection detection = detector.run(save_screenshots, log_image_values);
        uint64_t last_bit = 0;
        switch (detection){
        case OrbeetleAttackAnimationDetector::NO_DETECTION:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Attack animation could not be detected.",
                stream
            );
        case OrbeetleAttackAnimationDetector::SPECIAL:
            text += " : Special";
            last_bit = 1;
            break;
        case OrbeetleAttackAnimationDetector::PHYSICAL:
            text += " : Physical";
            last_bit = 0;
            break;
        }
        stream.overlay().add_log(text, COLOR_BLUE);
        pbf_wait(context, 180);

        if (i < 64){
            last_bits0 += last_bit << (63 - i);
        }else{
            last_bits1 += last_bit << (63 - (i - 64));
        }
    }
    Xoroshiro128Plus rng = Xoroshiro128Plus::xoroshiro128plus_from_last_bits(std::pair(last_bits0, last_bits1));

    for (size_t j = 0; j < 128; j++){
        rng.next();
    }
    stream.log("RNG: state[0] = " + tostr_hex(rng.get_state().s0));
    stream.log("RNG: state[1] = " + tostr_hex(rng.get_state().s1));
    return rng.get_state();
}

Xoroshiro128PlusState refind_rng_state(
    VideoStream& stream,
    ProControllerContext& context,
    Xoroshiro128PlusState last_known_state,
    size_t min_advances,
    size_t max_advances,
    bool save_screenshots,
    bool log_image_values
) {
    return refind_rng_state_and_animations(stream, context, last_known_state, min_advances, max_advances, save_screenshots, log_image_values).first;
}

std::pair<Xoroshiro128PlusState, uint64_t> refind_rng_state_and_animations(
    VideoStream& stream,
    ProControllerContext& context,
    Xoroshiro128PlusState last_known_state,
    size_t min_advances,
    size_t max_advances,
    bool save_screenshots,
    bool log_image_values
)
{
    Xoroshiro128Plus rng(last_known_state.s0, last_known_state.s1);
    for (size_t i = 0; i < min_advances; i++){
        rng.next();
    }
    OrbeetleAttackAnimationDetector detector(stream, context);
    size_t possible_indices = SIZE_MAX;
    std::vector<bool> sequence = {};
    std::vector<bool> last_bit_sequence = rng.generate_last_bit_sequence(max_advances - min_advances);
    size_t distance = 0;

    size_t i = 0;
    while (possible_indices > 1){
        context.wait_for_all_requests();

        std::string text = std::to_string(++i) + "/?";
        OrbeetleAttackAnimationDetector::Detection detection = detector.run(save_screenshots, log_image_values);
        switch (detection){
        case OrbeetleAttackAnimationDetector::NO_DETECTION:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Attack animation could not be detected.",
                stream
            );
        case OrbeetleAttackAnimationDetector::SPECIAL:
            text += " : Special";
            sequence.emplace_back(true);
            break;
        case OrbeetleAttackAnimationDetector::PHYSICAL:
            text += " : Physical";
            sequence.emplace_back(false);
            break;
        }
        stream.overlay().add_log(text, COLOR_BLUE);
        pbf_wait(context, 180);

        std::vector<bool>::iterator last_bit_start = std::search(last_bit_sequence.begin(), last_bit_sequence.end(), sequence.begin(), sequence.end());
        possible_indices = 0;
        distance = 0;
        while (last_bit_start != last_bit_sequence.end()){
            possible_indices++;
            distance = std::distance(last_bit_sequence.begin(), last_bit_start);

            last_bit_start++;
            last_bit_start = std::search(last_bit_start, last_bit_sequence.end(), sequence.begin(), sequence.end());
        }
    }
    if (possible_indices == 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Detected sequence of attack motions does not exist in expected range.",
            stream
        );
    }

    distance += sequence.size();
    stream.log("RNG: needed " + std::to_string(sequence.size()) + " animations.");
    stream.log("RNG: new state is " + std::to_string(distance + min_advances) + " advances from last known state.");
    for (size_t advance = 0; advance < distance; advance++){
        rng.next();
    }
    stream.log("RNG: state[0] = " + tostr_hex(rng.get_state().s0));
    stream.log("RNG: state[1] = " + tostr_hex(rng.get_state().s1));

    return { rng.get_state(), sequence.size() };
}


void do_rng_advances(
    VideoStream& stream, ProControllerContext& context,
    Xoroshiro128Plus& rng,
    size_t advances,
    Milliseconds press_duration,
    Milliseconds release_duration
){
    Milliseconds tv = context->timing_variation();
    for (size_t i = 0; i < advances; i++){
        if ((i + 1) % 10 == 0){
            std::string text = std::to_string(i + 1) + "/" + std::to_string(advances);
            stream.log("RNG advance: " + text);
            stream.overlay().add_log("Advancing: " + text, COLOR_GREEN);
        }
        pbf_press_button(
            context,
            BUTTON_RCLICK,
            press_duration + tv,
            release_duration + tv
        );
        rng.next();
    }
    pbf_wait(context, 1000ms);
}

Xoroshiro128PlusState predict_state_after_menu_close(Xoroshiro128PlusState current_state, uint8_t num_npcs) {
    Xoroshiro128Plus rng(current_state);

    for (size_t i = 0; i < num_npcs; i++) {
        rng.nextInt(91);
    }
    rng.next();
    rng.nextInt(61);

    return rng.get_state();
}


}
}
}
