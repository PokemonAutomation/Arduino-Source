/*  Sockets Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Sockets_Tests_H
#define PokemonAutomation_Sockets_Tests_H

namespace PokemonAutomation{

class ThreadPool;
class UnitTestDatabase;


// Loopback tests that combine the generic ClientSocket and ServerSocket
// against each other on 127.0.0.1: echo round trip, zero-length sends, a
// large transfer, rapid reconnects, a send/close race, connection
// rejection, and a port-in-use start failure. `thread_pool` runs the
// sockets and the test workers; it must be a long-lived pool (e.g.
// `GlobalThreadPools::unlimited_normal()`).
void add_tests_Sockets(UnitTestDatabase& database, ThreadPool& thread_pool);


}
#endif
