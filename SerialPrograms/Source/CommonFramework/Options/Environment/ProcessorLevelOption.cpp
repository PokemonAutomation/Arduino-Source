/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Options/EnumDropdownWidget.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{



std::vector<EnumDropdownOption::Option> make_processor_labels(){
    std::vector<EnumDropdownOption::Option> ret;
    for (const CpuCapabilityOption& option : AVAILABLE_CAPABILITIES()){
        ret.emplace_back(option.label, option.available);
    }
    return ret;
}
size_t get_default_ProcessorLevel_index(){
    const std::vector<CpuCapabilityOption>& LEVELS = AVAILABLE_CAPABILITIES();
    size_t best = 0;
    for (size_t c = 0; c < LEVELS.size(); c++){
        if (LEVELS[c].available){
            best = c;
        }
    }
    return best;
}



ProcessorLevelOption::ProcessorLevelOption()
    : EnumDropdownOption(
        "<b>Processor Specific Optimization:</b><br>"
        "Note that this only applies to this binary. External dependencies may ignore this and use higher instructions anyway.",
        make_processor_labels(),
        get_default_ProcessorLevel_index()
    )
{
    set_global();
}
void ProcessorLevelOption::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QString processor_string = QString::fromStdString(get_processor_name());
    global_logger_tagged().log("Processor String: " + processor_string);
    if (processor_string == json_get_string_nothrow(obj, "ProcessorString")){
        global_logger_tagged().log("Processor string matches. Using stored processor level.", COLOR_BLUE);
        EnumDropdownOption::load_json(json_get_value_nothrow(obj, "Level"));
        set_global();
    }else{
        global_logger_tagged().log("Mismatched processor string. Will not load saved processor level.", COLOR_RED);
    }
}
QJsonValue ProcessorLevelOption::to_json() const{
    QJsonObject obj;
    obj.insert("Level", EnumDropdownOption::to_json());
    obj.insert("ProcessorString", QString::fromStdString(get_processor_name()));
    return obj;
}
void ProcessorLevelOption::set_global(){
    set_global((int)(size_t)*this);
}
void ProcessorLevelOption::set_global(int index){
    const auto& LIST = AVAILABLE_CAPABILITIES();
    if (index < 0 || (size_t)index >= LIST.size()){
        return;
    }
    CPU_CAPABILITY_CURRENT = LIST[index].features;
    global_logger_tagged().log(std::string("Processor capability set to: ") + LIST[index].label, COLOR_BLUE);
}

class ProcessPriorityWidget : public EnumDropdownWidget{
public:
    ProcessPriorityWidget(QWidget& parent, ProcessorLevelOption& value)
         : EnumDropdownWidget(parent, value)
    {
        connect(
            this, &EnumDropdownWidget::on_changed,
            this, [=](){
                static_cast<ProcessorLevelOption&>(m_value).set_global();
            }
        );
    }
};
ConfigWidget* ProcessorLevelOption::make_ui(QWidget& parent){
    return new ProcessPriorityWidget(parent, *this);
}





}
