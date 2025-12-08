/*  Weather Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_MenuNavigation.h"
#include "PokemonLZA_WeatherFinder.h"
#include <PokemonLZA/Inference/PokemonLZA_WeatherDetector.h>
#include <cstdint>
#include <string>


namespace PokemonAutomation {
    namespace NintendoSwitch {
        namespace PokemonLZA {

            using namespace Pokemon;

            WeatherFinder_Descriptor::WeatherFinder_Descriptor()
                : SingleSwitchProgramDescriptor(
                    "PokemonLZA:WeatherFinder",
                    STRING_POKEMON + " LZA", "Weather Finder",
                    "Programs/PokemonLZA/WeatherFinder.html",
                    "Reset the weather by siting on a bench until reach the desired value",
                    ProgramControllerClass::StandardController_NoRestrictions,
                    FeedbackType::REQUIRED,
                    AllowCommandsWhenRunning::DISABLE_COMMANDS,
                    {}
                )
            {
            }

            class WeatherFinder_Descriptor::Stats : public StatsTracker {
            public:
                Stats()
                    : loops(m_stats["Loops"])
                    , errors(m_stats["Errors"])
                {
                    m_display_order.emplace_back("Loops");
                    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
                }

                std::atomic<uint64_t>& loops;
                std::atomic<uint64_t>& errors;
            };

            std::unique_ptr<StatsTracker> WeatherFinder_Descriptor::make_stats() const {
                return std::unique_ptr<StatsTracker>(new Stats());
            }


            WeatherFinder::WeatherFinder()
                : START_POSITION(
                    "<b>Starting from day or night:</b>",
                    {
                        {0, "day", "DAY"},
                        {1, "night", "NIGHT"},
                    },
                    LockMode::LOCK_WHILE_RUNNING,
                    0
                    )
                , DESIRED_WEATHER(
                    "<b>Stop on:</b>",
                    {
                        {0, "day-clear", "Clear Day"},
                        {1, "day-cloudy", "Cloudy Day"},
                        {2, "day-rainy", "Rainy Day"},
                        {3, "day-sunny", "Sunny Day"},
                        {4, "day-foggy", "Foggy Day"},
                        {5, "day-rainbow", "Rainbow Day"},
                    },
                    LockMode::LOCK_WHILE_RUNNING,
                    0
                    )
                , GO_HOME_WHEN_DONE(true)
                , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
                , NOTIFICATIONS({
                    &NOTIFICATION_STATUS,
                    &NOTIFICATION_PROGRAM_FINISH,
                    &NOTIFICATION_ERROR_RECOVERABLE,
                    &NOTIFICATION_ERROR_FATAL,
                    })
            {
                PA_ADD_OPTION(START_POSITION);
                PA_ADD_OPTION(DESIRED_WEATHER);
                PA_ADD_OPTION(GO_HOME_WHEN_DONE);
                PA_ADD_OPTION(NOTIFICATIONS);
            }

            // //Test Method
            // void WeatherFinder::validate_weather(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
            //     VideoSnapshot screen = env.console.video().snapshot();
                
            //     WeatherIconDetector clearDetector(
            //         WeatherIconType::Clear,
            //         &env.console.overlay()
            //     );
            //     bool clearResult = clearDetector.detect(screen);
            //     env.log("Clear is " + std::to_string(clearResult));
            //     // context.wait_for_all_requests();

            //     WeatherIconDetector sunnyDetector(
            //         WeatherIconType::Sunny,
            //         &env.console.overlay()
            //     );
            //     bool sunnyResult = sunnyDetector.detect(screen);
            //     env.log("Sunny is " + std::to_string(sunnyResult));

            //     WeatherIconDetector rainyDetector(
            //         WeatherIconType::Rain,
            //         &env.console.overlay()
            //     );
            //     bool rainyResult = rainyDetector.detect(screen);
            //     env.log("Rainy is " + std::to_string(rainyResult));

            //     WeatherIconDetector cloudyDetector(
            //         WeatherIconType::Cloudy,
            //         &env.console.overlay()
            //     );
            //     bool cloudyResult = cloudyDetector.detect(screen);
            //     env.log("Cloudy is " + std::to_string(cloudyResult));
               
            //     WeatherIconDetector foggyDetector(
            //         WeatherIconType::Foggy,
            //         &env.console.overlay()
            //     );
            //     bool foggyResult = foggyDetector.detect(screen);
            //     env.log("Foggy is " + std::to_string(foggyResult));

            //     WeatherIconDetector rainbowDetector(
            //         WeatherIconType::Rainbow,
            //         &env.console.overlay()
            //     );
            //     bool rainbowResult = rainbowDetector.detect(screen);
            //     env.log("Rainbow is " + std::to_string(rainbowResult));
            // }

            WeatherIconType get_weather_type(size_t option) {
                switch (option) {
                    case 0:
                        return WeatherIconType::Clear;
                    case 1:
                        return WeatherIconType::Cloudy;
                    case 2:
                        return WeatherIconType::Rain;
                    case 3:
                        return WeatherIconType::Sunny;
                    case 4:
                        return WeatherIconType::Foggy;
                    case 5:
                        return WeatherIconType::Rainbow;
                    default:
                        return WeatherIconType::Clear;
                }
            }

            std::string get_weather_name(size_t option) {
                switch (option) {
                    case 0:
                        return "Clear";
                    case 1:
                        return "Cloudy";
                    case 2:
                        return "Rainy";
                    case 3:
                        return "Sunny";
                    case 4:
                        return "Foggy";
                    case 5:
                        return "Rainbow";
                    default:
                        return "Clear";
                }
            }

            void bench_it(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
                sit_on_bench(env.console, context);
                pbf_move_left_joystick(context, 128, 255, 500ms, 500ms);
                context.wait_for_all_requests();
            }

            void WeatherFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
                WeatherFinder_Descriptor::Stats& stats = env.current_stats<WeatherFinder_Descriptor::Stats>();
                assert_16_9_720p_min(env.logger(), env.console);
                
                bool isDay = !START_POSITION.current_value();

                //Reset to day time
                if (!isDay) {
                    env.log("Resetting to Day");
                    bench_it(env, context);
                    isDay = true;
                }

                WeatherIconType weatherType = get_weather_type(DESIRED_WEATHER.current_value());
                std::string weatherName = get_weather_name(DESIRED_WEATHER.current_value());

                WeatherIconDetector detector(
                        weatherType,
                        &env.console.overlay()
                    );

                while (true) {
                    env.log("Looking for " + weatherName + " " + std::string(isDay ? "day" : "night"));
                    
                    open_map(env.console, context);
                    VideoSnapshot screen = env.console.video().snapshot();
                    
                    bool result = detector.detect(screen);
                    context.wait_for_all_requests();
                    
                    if (result == 1) {
                        env.log("Desired weather found.");
                        GO_HOME_WHEN_DONE.run_end_of_program(context);
                        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
                        break;
                    }
                    else {
                        env.log("Desired weather not found. Continuing");
                        pbf_press_button(context, BUTTON_PLUS, 500ms, 500ms);
                        for (uint8_t c = 0; c < 2; c++) {
                            bench_it(env, context);
                            isDay = !isDay;
                            env.log("Current time of day: " + std::string(isDay ? "DAY" : "NIGHT"));
                        }
                        stats.loops++;
                        context.wait_for_all_requests();
                    }
                    env.update_stats();
                    context.wait_for_all_requests();
                    send_program_status_notification(env, NOTIFICATION_STATUS);
                }
            }
}
}
}
