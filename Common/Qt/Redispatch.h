/*  Redispatch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Qt_Redispatch_H
#define PokemonAutomation_Qt_Redispatch_H

#include <functional>

class QObject;

namespace PokemonAutomation{


//  Be careful with these due to re-entrancy and object lifetime.
void dispatch_to_main_thread(std::function<void()> lambda);
void queue_on_main_thread(std::function<void()> lambda);


void run_on_main_thread_and_wait(std::function<void()> lambda);
void run_on_object_thread_and_wait(QObject* object, std::function<void()> lambda);



}
#endif
