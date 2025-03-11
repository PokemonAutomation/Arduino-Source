/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Environment/Environment.h"
#include "CommonFramework/Logging/Logger.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{



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
    : IntegerEnumDropdownOption(
        "<b>Processor Specific Optimization:</b><br>"
        "Note that this only applies to this binary. External dependencies may ignore this and use higher instructions anyway.",
        CAPABILITIES_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        get_default_ProcessorLevel_index()
    )
{
    set_global();
}
bool ProcessorLevelOption::set_value(size_t value){
    if (!IntegerEnumDropdownOption::set_value(value)){
        return false;
    }
    set_global(value);
    return true;
}
void ProcessorLevelOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
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
            IntegerEnumDropdownOption::load_json(*value);
        }
        set_global();
    }else{
        global_logger_tagged().log("Mismatched processor string. Will not load saved processor level.", COLOR_RED);
    }
}
JsonValue ProcessorLevelOption::to_json() const{
    JsonObject obj;
    obj["Level"] = IntegerEnumDropdownOption::to_json();
    obj["ProcessorString"] = get_processor_name();
    return obj;
}
void ProcessorLevelOption::set_global(){
    set_global(current_value());
}
void ProcessorLevelOption::set_global(size_t index){
    const auto& LIST = AVAILABLE_CAPABILITIES();
    if ((size_t)index >= LIST.size()){
        return;
    }
    CPU_CAPABILITY_CURRENT = LIST[index].features;
    global_logger_tagged().log(std::string("Processor capability set to: ") + LIST[index].display, COLOR_BLUE);
}






}
