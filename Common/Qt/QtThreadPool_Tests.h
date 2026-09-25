/*  QtThreadPool Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_QtThreadPool_Tests_H
#define PokemonAutomation_QtThreadPool_Tests_H

namespace PokemonAutomation{

class UnitTestDatabase;


// Stress tests for `QtEventThreadPool`: many threads concurrently adding and removing
// objects, checking that no thread's object is ever destroyed by another thread.
void add_tests_QtEventThreadPool(UnitTestDatabase& database);


}
#endif
