/*  QtThreadPool Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_QtThreadPool_Tests_H
#define PokemonAutomation_QtThreadPool_Tests_H

namespace PokemonAutomation{

class ThreadPool;
class UnitTestDatabase;


// Stress tests for `QtEventThreadPool`: many threads concurrently adding and removing
// objects, checking that no thread's object is ever destroyed by another thread.
// `thread_pool` runs the concurrent workers. It must be a long-lived pool (e.g.
// `GlobalThreadPools::computation_normal()`), see the test for why.
void add_tests_QtEventThreadPool(UnitTestDatabase& database, ThreadPool& thread_pool);


}
#endif
