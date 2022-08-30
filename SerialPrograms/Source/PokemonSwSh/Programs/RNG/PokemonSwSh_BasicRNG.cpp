/*  Basic RNG manipulation 
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/RNG/PokemonSwSh_OrbeetleAttackAnimationDetector.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"

namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonSwSh {

            Xoroshiro128PlusState find_rng_state(
                ConsoleHandle& console,
                BotBaseContext& context, 
                bool save_screenshots,
                bool log_image_values
            ) {
                OrbeetleAttackAnimationDetector detector(console, context);
                uint64_t last_bits0 = 0;
                uint64_t last_bits1 = 0;

                for (size_t i = 0; i < 128; i++) {
                    context.wait_for_all_requests();
                    console.log("RNG: Attack animation " + std::to_string(i + 1) + "/128");
                    OrbeetleAttackAnimationDetector::Detection detection = detector.run(save_screenshots, log_image_values);
                    uint64_t last_bit = 0;
                    switch (detection) {
                    case OrbeetleAttackAnimationDetector::NO_DETECTION:
                        throw OperationFailedException(console, "Attack animation could not be detected.");
                        break;
                    case OrbeetleAttackAnimationDetector::SPECIAL:
                        last_bit = 1;
                        break;
                    case OrbeetleAttackAnimationDetector::PHYSICAL:
                        last_bit = 0;
                        break;
                    }
                    pbf_wait(context, 180);

                    if (i < 64) {
                        last_bits0 += last_bit << (63 - i);
                    }
                    else {
                        last_bits1 += last_bit << (63 - (i - 64));
                    }
                }
                Xoroshiro128Plus rng = Xoroshiro128Plus::xoroshiro128plus_from_last_bits(std::pair(last_bits0, last_bits1));

                for (size_t j = 0; j < 128; j++) {
                    rng.next();
                }
                console.log("RNG: state[0] = " + std::format("{:x}", rng.get_state().s0));
                console.log("RNG: state[1] = " + std::format("{:x}", rng.get_state().s1));
                return rng.get_state();
            }


            Xoroshiro128PlusState refind_rng_state(
                ConsoleHandle& console,
                BotBaseContext& context,
                Xoroshiro128PlusState last_known_state,
                size_t min_advances,
                size_t max_advances,
                bool save_screenshots, 
                bool log_image_values)
            {
                Xoroshiro128Plus rng(last_known_state.s0, last_known_state.s1);
                for (size_t i = 0; i < min_advances; i++) {
                    rng.next();
                }
                OrbeetleAttackAnimationDetector detector(console, context);
                size_t possible_indices = SIZE_MAX;
                std::vector<bool> seq = {};
                std::vector<bool> last_bit_seq = rng.generate_last_bit_sequence(max_advances - min_advances);
                size_t distance = 0;

                while (possible_indices > 1) {
                    context.wait_for_all_requests();

                    OrbeetleAttackAnimationDetector::Detection detection = detector.run(save_screenshots, log_image_values);
                    switch (detection) {
                    case OrbeetleAttackAnimationDetector::NO_DETECTION:
                        throw OperationFailedException(console, "Attack animation could not be detected.");
                        break;
                    case OrbeetleAttackAnimationDetector::SPECIAL:
                        seq.emplace_back(true);
                        break;
                    case OrbeetleAttackAnimationDetector::PHYSICAL:
                        seq.emplace_back(false);
                        break;
                    }
                    pbf_wait(context, 180);

                    std::vector<bool>::iterator last_bit_start = std::search(last_bit_seq.begin(), last_bit_seq.end(), seq.begin(), seq.end());
                    possible_indices = 0;
                    distance = 0;
                    while (last_bit_start != last_bit_seq.end()) {
                        possible_indices++;
                        distance = std::distance(last_bit_seq.begin(), last_bit_start);

                        last_bit_start++;
                        last_bit_start = std::search(last_bit_start, last_bit_seq.end(), seq.begin(), seq.end());
                    }
                }
                if (possible_indices == 0) {
                    throw OperationFailedException(console, "Detected sequence of attack motions does not exist in expected range.");
                }

                distance += seq.size();
                console.log("RNG: needed " + std::to_string(seq.size()) + " animations.");
                console.log("RNG: new state is " + std::to_string(distance + min_advances) + " advances from last known state.");
                for (size_t advance = 0; advance < distance; advance++) {
                    rng.next();
                }
                console.log("RNG: state[0] = " + std::format("{:x}", rng.get_state().s0));
                console.log("RNG: state[1] = " + std::format("{:x}", rng.get_state().s1));

                return rng.get_state();
            }


            void do_rng_advances(ConsoleHandle& console, BotBaseContext& context, Xoroshiro128Plus& rng, size_t advances) {
                for (size_t i = 0; i < advances; i++) {
                    if ((i + 1) % 100 == 0) {
                        console.log("RNG advance: " + std::to_string(i + 1) + "/" + std::to_string(advances));
                    }
                    pbf_press_button(context, BUTTON_RCLICK, 10, 10);
                    rng.next();
                }
                pbf_wait(context, 1 * TICKS_PER_SECOND);
            }


        }
    }
}
