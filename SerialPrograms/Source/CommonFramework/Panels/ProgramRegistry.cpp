/*  Program Registry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ProgramRegistry.h"

namespace PokemonAutomation{

ProgramRegistry& ProgramRegistry::instance(){
    static ProgramRegistry instance;
    return instance;
}

void ProgramRegistry::add_category(std::unique_ptr<PanelListDescriptor> category){
    m_categories.emplace_back(std::move(category));
}

}
