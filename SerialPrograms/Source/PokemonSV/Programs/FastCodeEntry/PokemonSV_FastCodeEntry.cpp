/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_CodeEntry.h"
#include "PokemonSV_FastCodeEntry.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



FastCodeEntry_Descriptor::FastCodeEntry_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSV:FastCodeEntry",
        STRING_POKEMON + " SV", "Fast Code Entry (FCE)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/FastCodeEntry.md",
        "Quickly enter a 4, 6, or 8 digit link code.",
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
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
