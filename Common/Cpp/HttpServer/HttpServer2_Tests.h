/*  HTTP Server 2 Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_HttpServer2_Tests_H
#define PokemonAutomation_HttpServer2_Tests_H

namespace PokemonAutomation{

class ThreadPool;
class UnitTestDatabase;


//  Loopback tests that run a real "HttpServer2" (the platform's default
//  server socket 2 backend) on 127.0.0.1 and drive it with raw
//  "ClientSocket" clients: request/response parsing, keep-alive,
//  pipelining, "Connection: close", "Expect: 100-continue", error
//  responses, and the lifecycle (disconnect during a handler, "stop()"
//  during a handler, restart, second-client rejection). `thread_pool`
//  runs the server and the test workers; it must be a long-lived pool
//  (e.g. `GlobalThreadPools::unlimited_normal()`).
void add_tests_HttpServer2(UnitTestDatabase& database, ThreadPool& thread_pool);


}
#endif
