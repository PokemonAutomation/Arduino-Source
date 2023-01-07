/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
//#include <QApplication>
//#include <QClipboard>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_FastCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings, const std::string& code,
    bool connect_controller_press
){
    static const std::map<char, char> MAP{
        {'1', '1'}, {'I', '1'}, {'i', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},
        {'0', '0'}, {'O', '1'}, {'o', '1'},

        {'Q', 'Q'}, {'q', 'Q'},
        {'W', 'W'}, {'w', 'W'},
        {'E', 'E'}, {'e', 'E'},
        {'R', 'R'}, {'r', 'R'},
        {'T', 'T'}, {'t', 'T'},
        {'Y', 'Y'}, {'y', 'Y'},
        {'U', 'U'}, {'u', 'U'},
        {'P', 'P'}, {'p', 'P'},

        {'A', 'A'}, {'a', 'A'},
        {'S', 'S'}, {'s', 'S'}, {'Z', 'S'}, {'z', 'S'},
        {'D', 'D'}, {'d', 'D'},
        {'F', 'F'}, {'f', 'F'},
        {'G', 'G'}, {'g', 'G'},
        {'H', 'H'}, {'h', 'H'},
        {'J', 'J'}, {'j', 'J'},
        {'K', 'K'}, {'k', 'K'},
        {'L', 'L'}, {'l', 'L'},

        {'X', 'X'}, {'x', 'X'},
        {'C', 'C'}, {'c', 'C'},
        {'V', 'V'}, {'v', 'V'},
        {'B', 'B'}, {'b', 'B'},
        {'N', 'N'}, {'n', 'N'},
        {'M', 'M'}, {'m', 'M'},
    };

    std::string normalized_code;

    //  Prune invalid characters.
    bool digits_only = true;
    for (char ch : code){
        auto iter = MAP.find(ch);
        if (iter == MAP.end()){
            continue;
        }
        ch = iter->second;
        digits_only &= '0' <= ch && ch <= '9';
        normalized_code += ch;
    }

    switch (normalized_code.size()){
    case 4:
        if (!digits_only){
            return "4-digit codes must be only digits.";
        }
        break;
    case 6:
        break;
    case 8:
        if (!digits_only){
            return "8-digit codes must be only digits.";
        }
        break;
    default:
        return "Invalid code length. Must be 4, 6, or 8 characters long.";
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        if (connect_controller_press){
            //  Connect the controller.
            pbf_press_button(context, BUTTON_R | BUTTON_L, 5, 3);
        }

        switch (normalized_code.size()){
        case 4:
            enter_digits_str(context, 4, normalized_code.c_str());
            break;
        case 6:
            enter_alphanumeric_code(
                console.logger(), context,
                settings,
                normalized_code
            );
            break;
        case 8:
            enter_digits_str(context, 8, normalized_code.c_str());
            break;
        }

    });

    return nullptr;
}



FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:FastCodeEntry",
        STRING_POKEMON + " SV", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FastCodeEntry.md",
        "Quickly enter a 4, 6, or 8 digit link code.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}

FastCodeEntry::FastCodeEntry()
    : MODE(
        "<b>Mode:</b>",
        {
            {Mode::NORMAL, "normal", "Enter Code when clicking Start Program."},
            {Mode::ENTER_ON_PASTE, "on-paste", "Start the program first. Code is entered when you paste into the code box."},
        },
        LockWhileRunning::LOCKED,
        Mode::NORMAL
    )
    , CODE(
        "<b>Link Code:</b><br>Must be 4-digit numeric or 6-digit alphanumeric. (not case sensitive)<br>"
        "(Box is big so it's easy to land your mouse on.)",
        LockWhileRunning::UNLOCKED,
        "0123", "0123",
        true
    )
{
    PA_ADD_OPTION(MODE);
    PA_ADD_OPTION(CODE);
    PA_ADD_OPTION(SETTINGS);
}




class FceCodeListener : public ConfigOption::Listener, public TextEditOption::FocusListener{
public:
    FceCodeListener(TextEditOption& code_box)
        : m_code_box(code_box)
    {
        code_box.add_listener(*this);
    }
    ~FceCodeListener(){
        m_code_box.remove_listener(*this);
    }

    virtual void value_changed() override{
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }
//    virtual void focus_in() override{
//        std::lock_guard<std::mutex> lg(m_lock);
//        m_cv.notify_all();
//    }

    void run(
        MultiSwitchProgramEnvironment& env, CancellableScope& scope,
        const FastCodeEntrySettings& settings
    ){
//        const QClipboard* clipboard = QApplication::clipboard();

        std::unique_lock<std::mutex> lg(m_lock);
        while (true){
            std::string code = m_code_box;
//            if (code.empty()){
//                code = clipboard->text().toStdString();
//            }
            if (!code.empty()){
                const char* error = enter_code(env, scope, settings, code, false);
                if (error == nullptr){
                    return;
                }
            }
            scope.throw_if_cancelled();
            m_cv.wait_for(lg, std::chrono::milliseconds(1));
        }
    }

private:
    TextEditOption& m_code_box;
    std::mutex m_lock;
    std::condition_variable m_cv;
};


void FastCodeEntry::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    FastCodeEntrySettings settings(SETTINGS);

    if (MODE == Mode::NORMAL){
        const char* error = enter_code(env, scope, settings, CODE, true);
        if (error){
            throw UserSetupError(env.logger(), error);
        }
        return;
    }

    //  Connect the controller.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        pbf_press_button(context, BUTTON_R | BUTTON_L, 5, 3);
    });

    FceCodeListener listener(CODE);
    listener.run(env, scope, settings);

}



}
}
}
