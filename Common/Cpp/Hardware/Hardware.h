/*  Hardware
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Hardware_H
#define PokemonAutomation_Hardware_H

#include <string>


namespace PokemonAutomation{




std::string get_processor_name();
struct ProcessorSpecs{
    std::string name;
    size_t threads = 0;
    size_t cores = 0;
    size_t sockets = 0;
    size_t numa_nodes = 0;
    size_t base_frequency = 0;
};
ProcessorSpecs get_processor_specs();









}
#endif
