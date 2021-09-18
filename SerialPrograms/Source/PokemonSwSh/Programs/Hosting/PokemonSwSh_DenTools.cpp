/*  Den Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShMisc.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "NintendoSwitch/FixedInterval.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh_DenTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void enter_den(const BotBaseContext& context, uint16_t ENTER_ONLINE_DEN_DELAY, bool watts, bool online){
    if (!online){
        if (!watts){
            ssf_press_button2(context, BUTTON_A, ENTER_OFFLINE_DEN_DELAY, 10);
        }else{
            //  This is the critical den-rolling path. It needs to be fast.
            mash_A(context, COLLECT_WATTS_OFFLINE_DELAY);
            pbf_wait(context, ENTER_OFFLINE_DEN_DELAY);
        }
    }else{
        if (!watts){
            ssf_press_button2(context, BUTTON_A, ENTER_ONLINE_DEN_DELAY, 50);
        }else{
            ssf_press_button2(context, BUTTON_A, COLLECT_WATTS_ONLINE_DELAY, 50);
            ssf_press_button2(context, BUTTON_B, 100, 50);
            ssf_press_button2(context, BUTTON_B, ENTER_ONLINE_DEN_DELAY, 50);
        }
    }
}


void enter_lobby(const BotBaseContext& context, uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY, bool online, Catchability catchability){
    if (online){
        switch (catchability){
        case ALWAYS_CATCHABLE:
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        case MAYBE_UNCATCHABLE:
        case ALWAYS_UNCATCHABLE:
            ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
            ssf_press_button1(context, BUTTON_A, OPEN_ONLINE_DEN_LOBBY_DELAY);
            return;
        }
    }

    switch (catchability){
    case ALWAYS_CATCHABLE:
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    case MAYBE_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);

        if (!DODGE_UNCATCHABLE_PROMPT_FAST){
            //  lobby-switch        switch-box
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  lobby-switch        switch-party-red
            ssf_press_button1(context, BUTTON_A, ENTER_SWITCH_POKEMON);
            //  switch-box          switch-confirm
            ssf_press_button1(context, BUTTON_Y, 10);
            ssf_press_dpad1(context, DPAD_LEFT, 10);
            //  switch-party-blue   switch-confirm
            ssf_press_button1(context, BUTTON_A, EXIT_SWITCH_POKEMON);
            //  lobby-switch        lobby-switch
        }
        return;
    case ALWAYS_UNCATCHABLE:
        ssf_press_button1(context, BUTTON_A, UNCATCHABLE_PROMPT_DELAY);
        ssf_press_button1(context, BUTTON_A, OPEN_LOCAL_DEN_LOBBY_DELAY);
        return;
    }
}


void roll_den(
    const BotBaseContext& context,
    uint16_t ENTER_ONLINE_DEN_DELAY,
    uint16_t OPEN_ONLINE_DEN_LOBBY_DELAY,
    uint8_t skips, Catchability catchability
){
    if (skips > 60){
        skips = 60;
    }
    for (uint8_t c = 0; c < skips; c++){
        enter_den(context, ENTER_ONLINE_DEN_DELAY, c != 0, false);
        enter_lobby(context, OPEN_ONLINE_DEN_LOBBY_DELAY, false, catchability);

        //  Skip forward.
        ssf_press_button2(context, BUTTON_HOME, GAME_TO_HOME_DELAY_FAST, 10);
        home_to_date_time(context, true, false);
        roll_date_forward_1(context, false);

        //  Enter game
        settings_to_enter_game_den_lobby(
            context,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW, true,
            ENTER_SWITCH_POKEMON, EXIT_SWITCH_POKEMON
        );

        //  Exit Raid
        ssf_press_button2(context, BUTTON_B, 120, 50);
        ssf_press_button2(context, BUTTON_A, REENTER_DEN_DELAY, 50);
    }
}
void rollback_date_from_home(const BotBaseContext& context, uint8_t skips){
    if (skips == 0){
        return;
    }
    if (skips > 60){
        skips = 60;
    }
    home_to_date_time(context, true, false);
    roll_date_backward_N(context, skips, false);
//    pbf_wait(5);

    //  Note that it is possible for this return animation to run longer than
    //  "SETTINGS_TO_HOME_DELAY" and swallow a subsequent button press.
    //  Therefore the caller needs to be able to tolerate this.
    ssf_press_button2(context, BUTTON_HOME, SETTINGS_TO_HOME_DELAY, 10);
}


void send_raid_notification(
    const QString& program_name,
    ConsoleHandle& console,
    const AutoHostNotificationOption& settings,
    bool has_code, uint8_t code[8],
    const DenMonReader& reader,
    const StatsTracker& stats_tracker
){
    if (!settings.ENABLE){
        return;
    }

    QImage screen = console.video().snapshot();
    DenMonReadResults results = reader.read(screen);

    std::vector<std::pair<QString, QString>> embeds;

    QString description = settings.DESCRIPTION;
    if (!description.isEmpty()){
        embeds.emplace_back("Description", description);
    }

    QString slugs;
    if (results.slugs.slugs.empty()){
        slugs += "Unable to detect.";
    }else if (results.slugs.slugs.size() == 1){
        slugs += QString::fromStdString(results.slugs.slugs.begin()->second);
    }else{
        slugs += "Ambiguous: ";
        size_t c = 0;
        for (const auto& item : results.slugs.slugs){
            if (c > 0){
                slugs += ", ";
            }
            slugs += QString::fromStdString(item.second);
            c++;
            if (c >= 5){
                break;
            }
        }
        if (c < results.slugs.slugs.size()){
            slugs += ", (";
            slugs += QString::number(results.slugs.slugs.size() - c);
            slugs += " more...)";
        }
    }
    embeds.emplace_back("Current " + STRING_POKEMON, slugs);

    if (settings.SCREENSHOT == ScreenshotMode::NO_SCREENSHOT){
        std::string code_str;
        if (has_code){
            size_t c = 0;
            for (; c < 4; c++){
                code_str += code[c] + '0';
            }
            code_str += " ";
            for (; c < 8; c++){
                code_str += code[c] + '0';
            }
        }else{
            code_str += "None";
        }
        embeds.emplace_back("Raid Code", QString::fromStdString(code_str));

#if 0
        QString ign = settings.IGN;
        if (!ign.isEmpty()){
            embeds.emplace_back("IGN(s)", ign);
        }
#endif
    }

#if 0
    std::vector<std::string> friend_codes = settings.FRIEND_CODES.list();
    std::string fcs_str;
    if (!friend_codes.empty()){
        bool first = true;
        for (const auto& item : friend_codes){
            if (!first){
                fcs_str += "\n";
            }
            first = false;
            fcs_str += item;
        }
        embeds.emplace_back("Friend Code(s)", QString::fromStdString(fcs_str));
    }
#endif

    embeds.emplace_back("Session Stats", QString::fromStdString(stats_tracker.to_str()));

    send_program_notification(
        console,
        false, QColor(),
        program_name,
        "Raid Notification",
        embeds
    );
    DiscordWebHook::send_screenshot(console, screen, settings.SCREENSHOT, false);

}





}
}
}
