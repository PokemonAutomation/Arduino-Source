/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "EnumDropdownWidget.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{


struct CpuCapabilityOption{
    const char* label;
    const CPU_x86_Features& features;

    CpuCapabilityOption(const char* p_label, const CPU_x86_Features& p_features)
        : label(p_label)
        , features(p_features)
    {}
};


std::vector<CpuCapabilityOption> make_AVAILABLE_CAPABILITIES(){
    std::vector<CpuCapabilityOption> list;
    list.emplace_back("Nothing (C++ Only)", CPU_CAPABILITY_NOTHING);
    if (CPU_CAPABILITY_NATIVE.OK_08_Nehalem){
        list.emplace_back("Intel Nehalem (x64 SSE4.2)", CPU_CAPABILITY_09_NEHALEM);
    }
    if (CPU_CAPABILITY_NATIVE.OK_13_Haswell){
        list.emplace_back("Intel Haswell (x64 AVX2)", CPU_CAPABILITY_13_Haswell);
    }
    if (CPU_CAPABILITY_NATIVE.OK_17_Skylake){
        list.emplace_back("Intel Skylake (x64 AVX512)", CPU_CAPABILITY_17_Skylake);
    }
    if (CPU_CAPABILITY_NATIVE.OK_19_IceLake){
        list.emplace_back("Intel Ice Lake (x64 AVX512-GF)", CPU_CAPABILITY_19_IceLake);
    }
    return list;
}

const std::vector<CpuCapabilityOption> AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST = make_AVAILABLE_CAPABILITIES();
    return LIST;
}


std::vector<QString> make_processor_labels(){
    std::vector<QString> ret;
    for (const CpuCapabilityOption& option : AVAILABLE_CAPABILITIES()){
        ret.emplace_back(option.label);
    }
    return ret;
}



ProcessorLevelOption::ProcessorLevelOption()
    : EnumDropdownOption(
        "<b>Processor Specific Optimization:</b>",
        make_processor_labels(),
        AVAILABLE_CAPABILITIES().size() - 1
    )
{
    set_global();
}
void ProcessorLevelOption::load_json(const QJsonValue& json){
    EnumDropdownOption::load_json(json);
    set_global();
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
