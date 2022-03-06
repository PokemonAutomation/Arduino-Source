/*  Process Priority Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProcessPriorityOption_H
#define PokemonAutomation_ProcessPriorityOption_H

#include "EnumDropdownOption.h"

namespace PokemonAutomation{


class ProcessPriorityOption : public EnumDropdownOption{
public:
    ProcessPriorityOption();
    void update_priority_to_option() const;

    virtual ConfigWidget* make_ui(QWidget& parent) override;
};



}
#endif
