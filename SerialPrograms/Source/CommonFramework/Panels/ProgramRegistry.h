/*  Program Registry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramRegistry_H
#define PokemonAutomation_ProgramRegistry_H

#include <vector>
#include <memory>
#include <string>
#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{

class ProgramRegistry{
public:
    static ProgramRegistry& instance();

    void add_category(std::unique_ptr<PanelListDescriptor> category);
    const std::vector<std::unique_ptr<PanelListDescriptor>>& categories() const { return m_categories; }

private:
    ProgramRegistry() = default;

private:
    std::vector<std::unique_ptr<PanelListDescriptor>> m_categories;
};

}

#endif
