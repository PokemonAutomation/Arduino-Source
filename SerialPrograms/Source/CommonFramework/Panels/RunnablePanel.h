/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_RunnablePanel_H
#define PokemonAutomation_CommonFramework_RunnablePanel_H

#include "Panel.h"

namespace PokemonAutomation{

class StatsTracker;


class RunnablePanelDescriptor : public PanelDescriptor{
public:
    using PanelDescriptor::PanelDescriptor;

    virtual std::unique_ptr<StatsTracker> make_stats() const;
};







}
#endif
