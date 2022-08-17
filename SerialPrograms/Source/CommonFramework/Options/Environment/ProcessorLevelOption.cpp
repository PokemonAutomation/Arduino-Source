/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{



std::vector<DropdownEntry> make_processor_labels(){
    std::vector<DropdownEntry> ret;
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
    : DropdownOption(
        "<b>Processor Specific Optimization:</b><br>"
        "Note that this only applies to this binary. External dependencies may ignore this and use higher instructions anyway.",
        make_processor_labels(),
        get_default_ProcessorLevel_index()
    )
{
    set_global();
}
bool ProcessorLevelOption::set_index(size_t index){
    if (!DropdownOption::set_index(index)){
        return false;
    }
    set_global(index);
    return true;
}
void ProcessorLevelOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    std::string processor_string = get_processor_name();
    global_logger_tagged().log("Processor String: " + processor_string);

    const std::string* saved_string = obj->get_string("ProcessorString");
    if (saved_string == nullptr){
        global_logger_tagged().log("No processor string saved.", COLOR_RED);
        return;
    }

    if (processor_string == *saved_string){
        global_logger_tagged().log("Processor string matches. Using stored processor level.", COLOR_BLUE);
        const JsonValue* value = obj->get_value("Level");
        if (value){
            DropdownOption::load_json(*value);
        }
        set_global();
    }else{
        global_logger_tagged().log("Mismatched processor string. Will not load saved processor level.", COLOR_RED);
    }
}
JsonValue ProcessorLevelOption::to_json() const{
    JsonObject obj;
    obj["Level"] = DropdownOption::to_json();
    obj["ProcessorString"] = get_processor_name();
    return obj;
}
void ProcessorLevelOption::set_global(){
    set_global((size_t)*this);
}
void ProcessorLevelOption::set_global(size_t index){
    const auto& LIST = AVAILABLE_CAPABILITIES();
    if ((size_t)index >= LIST.size()){
        return;
    }
    CPU_CAPABILITY_CURRENT = LIST[index].features;
    global_logger_tagged().log(std::string("Processor capability set to: ") + LIST[index].label, COLOR_BLUE);
}






}
