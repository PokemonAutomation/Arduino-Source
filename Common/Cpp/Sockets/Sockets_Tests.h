/*  ClientSocket Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Sockets_Tests_H
#define PokemonAutomation_Sockets_Tests_H

namespace PokemonAutomation{

class ThreadPool;
class UnitTestDatabase;


//  Loopback tests that drive the "ClientSocket" backends against a
//  "ServerSocket" peer on 127.0.0.1: the client-side behaviors of the
//  legacy ClientSocket/ServerSocket pair that the "ServerSocket" test
//  suite does not cover (a send with a deadline against a stalled peer,
//  a pre-close response while the client is still sending, close() from
//  the client's own receive callback, and the Qt client's
//  close()-cancels-sends guarantee). Each scenario runs for every client
//  backend that compiles on the current platform (Qt: always, POSIX:
//  non-Windows, WinSocket: Windows). `thread_pool` runs the sockets and
//  the test workers; it must be a long-lived pool (e.g.
//  `GlobalThreadPools::unlimited_normal()`).
void add_tests_Sockets(UnitTestDatabase& database, ThreadPool& thread_pool);


}
#endif
