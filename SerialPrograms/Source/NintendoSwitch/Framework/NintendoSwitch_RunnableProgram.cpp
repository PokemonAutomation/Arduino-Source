/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Qt/QtJsonTools.h"
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
    QString category, QString display_name,
    QString doc_link,
    QString description,
    FeedbackType feedback,
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
{}




void RunnableSwitchProgramInstance::from_json(const QJsonValue& json){
    const QJsonObject& obj = json.toObject();
    m_setup->load_json(json_get_value_nothrow(obj, "SwitchSetup"));
    RunnablePanelInstance::from_json(json);
}
QJsonValue RunnableSwitchProgramInstance::to_json() const{
    QJsonObject obj = RunnablePanelInstance::to_json().toObject();
    obj.insert("SwitchSetup", m_setup->to_json());
    return obj;
}






}
}





