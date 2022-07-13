/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "NintendoSwitch_SwitchSetup.h"
#include "NintendoSwitch_RunnableProgram.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



Color pick_color(FeedbackType feedback, PABotBaseLevel size){
    switch (size){
    case PABotBaseLevel::NOT_PABOTBASE:
        return Color();
    case PABotBaseLevel::PABOTBASE_12KB:
        return feedback == FeedbackType::REQUIRED ? COLOR_DARKGREEN : COLOR_BLUE;
    case PABotBaseLevel::PABOTBASE_31KB:
        return feedback == FeedbackType::REQUIRED ? COLOR_PURPLE : COLOR_RED;
    }
    return Color();
}
RunnableSwitchProgramDescriptor::RunnableSwitchProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    FeedbackType feedback, bool allow_commands_while_running,
    PABotBaseLevel min_pabotbase_level
)
    : RunnablePanelDescriptor(
        pick_color(feedback, min_pabotbase_level),
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
    , m_feedback(feedback)
    , m_min_pabotbase_level(min_pabotbase_level)
    , m_allow_commands_while_running(allow_commands_while_running)
{}




void RunnableSwitchProgramInstance::from_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value = obj->get_value("SwitchSetup");
    if (value){
        m_setup->load_json(*value);
    }
    RunnablePanelInstance::from_json(json);
}
JsonValue RunnableSwitchProgramInstance::to_json() const{
    JsonObject obj = std::move(*RunnablePanelInstance::to_json().get_object());
    obj["SwitchSetup"] = m_setup->to_json();
    return obj;
}






}
}





