/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_LifetimeSanitizer_H
#define PokemonAutomation_LifetimeSanitizer_H

#define PA_SANITIZER_ENABLE

namespace PokemonAutomation{



class LifetimeSanitizer{
public:
#ifdef PA_SANITIZER_ENABLE
    LifetimeSanitizer();
    ~LifetimeSanitizer();
    void check_usage() const;
#else
    void check_usage() const{}
#endif
};




}
#endif
