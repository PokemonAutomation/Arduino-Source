/*  Cancellation Exceptions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      These are exceptions that are throw for the purpose of
 *  unwinding the stack. They are not considered error conditions.
 *
 */

#ifndef PokemonAutomation_CancellationExceptions_H
#define PokemonAutomation_CancellationExceptions_H

namespace PokemonAutomation{


//  Definitions:
//      Catch:   To catch the exception using a try-catch.
//      Consume: To catch the exception and not rethrow it.



//  (Deprecated)
//  This is the universal "operation cancelled" exception that is used to break
//  out of routines in a clean manner when a users requests a cancellation.
struct CancelledException0{};


//  Thrown when the user stops the program.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
struct ProgramCancelledException : public CancelledException0{};


//  Thrown by BotBase connections when a command is issued while the connection
//  is in a state that isn't accepting commands.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
struct InvalidConnectionStateException : public CancelledException0{};


//  Thrown when a local operation is cancelled.
//  This can be caught by local handlers that do async-cancel.
//  If this propagates up to the infra, it is considered an error.
struct OperationCancelledException : public CancelledException0{};



}
#endif
