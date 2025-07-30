/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TestProgram_H
#define PokemonAutomation_NintendoSwitch_TestProgram_H

#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/DateOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/BoxFloatOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOption.h"
#include "PokemonSV/Options/PokemonSV_PlayerList.h"
#include "PokemonSV/Options/PokemonSV_SinglesAIOption.h"
#include "Common/Cpp/Options/ColorOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerSettings.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

//using namespace PokemonSwSh;





class TestProgram_Descriptor : public MultiSwitchProgramDescriptor{
public:
    TestProgram_Descriptor();
};


class TestProgram : public MultiSwitchProgramInstance, public ButtonListener{
public:
    ~TestProgram();
    TestProgram();

//    std::unique_ptr<StatsTracker> make_stats() const override{
//        return std::unique_ptr<StatsTracker>(new StatsTracker());
//    }
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

    virtual void on_press() override;

private:
    struct Stats : public StatsTracker{
        Stats()
            : m_resets(m_stats["Resets"])
        {
            m_display_order.emplace_back("Resets");
        }
        std::atomic<uint64_t>& m_resets;
    };

private:
    ButtonCell BUTTON0;
    ButtonOption BUTTON1;

    OCR::LanguageOCROption LANGUAGE;

    StringOption IMAGE_PATH;

    StaticTextOption STATIC_TEXT;
    BoxFloatOption BOX;


//    PokemonSV::SinglesAIOption battle_AI;

    EventNotificationOption NOTIFICATION_TEST;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
#endif

