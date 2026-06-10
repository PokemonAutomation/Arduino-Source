/* Core Affinity Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CoreAffinityOption_H
#define PokemonAutomation_CoreAffinityOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{


class CoreAffinityOption : public GroupOption{
public:
    CoreAffinityOption();
    ~CoreAffinityOption();

private:
    class Internal;
    Pimpl<Internal> m_internal;
};




}
#endif
