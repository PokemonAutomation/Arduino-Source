/*  Processor Level Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Logging/LoggerQt.h"
#include "EnumDropdownWidget.h"
#include "ProcessorLevelOption.h"

namespace PokemonAutomation{


struct CpuCapabilityOption{
    const char* label;
    const CPU_x86_Features& features;
};

const std::vector<CpuCapabilityOption> AVAILABLE_CAPABILITIES{
#ifdef PA_AutoDispatch_08_Nehalem
    {"Intel Nehalem (x64 SSE4.2)",      CPU_CAPABILITY_09_NEHALEM},
#endif
#ifdef PA_AutoDispatch_13_Haswell
    {"Intel Haswell (x64 AVX2)",        CPU_CAPABILITY_13_Haswell},
#endif
#ifdef PA_AutoDispatch_17_Skylake
    {"Intel Skylake (x64 AVX512)",      CPU_CAPABILITY_17_Skylake},
#endif
#ifdef PA_AutoDispatch_19_IceLake
    {"Intel Ice Lake (x64 AVX512-GF)",  CPU_CAPABILITY_19_IceLake},
#endif
};


std::vector<QString> make_processor_labels(){
    std::vector<QString> ret;
    for (const CpuCapabilityOption& option : AVAILABLE_CAPABILITIES){
        ret.emplace_back(option.label);
    }
    return ret;
}



ProcessorLevelOption::ProcessorLevelOption()
    : EnumDropdownOption(
        "<b>Processor Specific Optimization:</b><br>"
        "Don't set this higher than what your CPU supports or the program may crash!",
        make_processor_labels(),
        0
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
    if (index < 0 || (size_t)index >= AVAILABLE_CAPABILITIES.size()){
        return;
    }
    CPU_CAPABILITY_CURRENT = AVAILABLE_CAPABILITIES[index].features;
    global_logger_tagged().log(std::string("Processor capability set to: ") + AVAILABLE_CAPABILITIES[index].label, COLOR_BLUE);
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
