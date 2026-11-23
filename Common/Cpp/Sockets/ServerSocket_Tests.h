/*  Server Socket Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ServerSocket_Tests_H
#define PokemonAutomation_ServerSocket_Tests_H

namespace PokemonAutomation{

class ThreadPool;
class UnitTestDatabase;


//  API-shape tests for AbstractServerSocket and
//  AbstractServerSocketConnection, run against an in-memory stub
//  backend. They pin the contracts the real backends must honor:
//  shared-pointer lifetime that outlives the server, inert-after-
//  close, on_disconnect() only for peer closes, and one connection at
//  a time.
void add_tests_ServerSocket(UnitTestDatabase& database, ThreadPool& thread_pool);


}
#endif
