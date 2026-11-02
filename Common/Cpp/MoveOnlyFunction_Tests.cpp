/*  Move-Only Function Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include "TestRunners/UnitTestDatabase.h"
#include "MoveOnlyFunction.h"
#include "MoveOnlyFunction_Tests.h"

namespace PokemonAutomation{
namespace{



//  Copying would fork a stateful callable, so it must not compile.
static_assert(!std::is_copy_constructible_v<MoveOnlyFunction<void()>>);
static_assert(!std::is_copy_assignable_v<MoveOnlyFunction<void()>>);
static_assert(std::is_nothrow_move_constructible_v<MoveOnlyFunction<void()>>);



std::string expected_int(const char* what, int expected, int actual){
    return std::format("{}: expected {}, got {}", what, expected, actual);
}



class Test_MoveOnlyFunction_MoveOnlyCapture : public UnitTest{
public:
    Test_MoveOnlyFunction_MoveOnlyCapture()
        : UnitTest("Common::MoveOnlyFunction - Move-only capture")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        auto value = std::make_unique<int>(42);
        MoveOnlyFunction<int(int)> function = [value = std::move(value)](int add){
            return *value + add;
        };
        int actual = function(1);
        if (actual != 43){
            return UnitTestResult(expected_int("call with a unique_ptr capture", 43, actual));
        }

        MoveOnlyFunction<int(int)> moved = std::move(function);
        actual = moved(2);
        if (actual != 44){
            return UnitTestResult(expected_int("call after a move", 44, actual));
        }
        return true;
    }
};



class Test_MoveOnlyFunction_StateNotForked : public UnitTest{
public:
    Test_MoveOnlyFunction_StateNotForked()
        : UnitTest("Common::MoveOnlyFunction - Moving keeps one state")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        MoveOnlyFunction<int()> counter = [count = 0]() mutable{
            return ++count;
        };
        int actual = counter();
        if (actual != 1){
            return UnitTestResult(expected_int("first call", 1, actual));
        }

        //  The moved-to wrapper continues the same count.
        MoveOnlyFunction<int()> constructed = std::move(counter);
        actual = constructed();
        if (actual != 2){
            return UnitTestResult(expected_int("call after a move construction", 2, actual));
        }

        MoveOnlyFunction<int()> assigned;
        assigned = std::move(constructed);
        actual = assigned();
        if (actual != 3){
            return UnitTestResult(expected_int("call after a move assignment", 3, actual));
        }
        return true;
    }
};



int return_seven(){
    return 7;
}

class Test_MoveOnlyFunction_Empty : public UnitTest{
public:
    Test_MoveOnlyFunction_Empty()
        : UnitTest("Common::MoveOnlyFunction - Empty is false")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        MoveOnlyFunction<int()> defaulted;
        if (defaulted){
            return UnitTestResult("default constructed: expected empty, got non-empty");
        }
        MoveOnlyFunction<int()> from_nullptr = nullptr;
        if (from_nullptr){
            return UnitTestResult("constructed from nullptr: expected empty, got non-empty");
        }
        int (*null_pointer)() = nullptr;
        MoveOnlyFunction<int()> from_null_pointer = null_pointer;
        if (from_null_pointer){
            return UnitTestResult("constructed from a null function pointer: expected empty, got non-empty");
        }

        MoveOnlyFunction<int()> from_pointer = &return_seven;
        if (!from_pointer){
            return UnitTestResult("constructed from a function pointer: expected non-empty, got empty");
        }
        int actual = from_pointer();
        if (actual != 7){
            return UnitTestResult(expected_int("call through a function pointer", 7, actual));
        }
        return true;
    }
};



class Test_MoveOnlyFunction_Reassignment : public UnitTest{
public:
    Test_MoveOnlyFunction_Reassignment()
        : UnitTest("Common::MoveOnlyFunction - Reassignment")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        //  The use count shows whether the wrapper still holds the first
        //  callable.
        auto token = std::make_shared<int>(1);
        MoveOnlyFunction<int()> function = [token]{
            return *token;
        };
        int actual = function();
        if (actual != 1){
            return UnitTestResult(expected_int("first callable", 1, actual));
        }

        function = []{
            return 2;
        };
        actual = function();
        if (actual != 2){
            return UnitTestResult(expected_int("call after reassignment", 2, actual));
        }
        actual = (int)token.use_count();
        if (actual != 1){
            return UnitTestResult(expected_int("token owners after reassignment", 1, actual));
        }

        function = nullptr;
        if (function){
            return UnitTestResult("assigned nullptr: expected empty, got non-empty");
        }
        if (function != nullptr){
            return UnitTestResult("assigned nullptr: expected == nullptr, got != nullptr");
        }
        return true;
    }
};



} // anonymous namespace



void add_tests_MoveOnlyFunction(UnitTestDatabase& database){
    database.add<Test_MoveOnlyFunction_MoveOnlyCapture>();
    database.add<Test_MoveOnlyFunction_StateNotForked>();
    database.add<Test_MoveOnlyFunction_Empty>();
    database.add<Test_MoveOnlyFunction_Reassignment>();
}



}
