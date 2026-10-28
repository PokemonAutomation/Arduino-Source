/*  Listener Set
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "ListenerSet.h"
#include "Concurrency/Mutex.h"
#include "TestRunners/UnitTestDatabase.h"

namespace PokemonAutomation{

namespace{



//
//  Records the order in which listeners were called, so tests can compare it
//  against the expected call order.
//
class CallLog{
public:
    void record(const std::string& name){
        LockGuard<Mutex> lg(m_lock);
        m_log += name;
    }
    std::string take(){
        LockGuard<Mutex> lg(m_lock);
        std::string ret = std::move(m_log);
        m_log.clear();
        return ret;
    }

private:
    Mutex m_lock;
    std::string m_log;
};



//
//  A listener that records its name into the shared call log, optionally
//  throwing. The map-based storage in the tests keeps these objects (and so
//  their addresses, which the ListenerSet keys on) stable.
//
struct TestListener{
    std::string name;
    CallLog& log;
    bool throws = false;

    TestListener(std::string p_name, CallLog& p_log)
        : name(std::move(p_name))
        , log(p_log)
    {}

    void ping(){
        log.record(name);
        if (throws){
            throw std::runtime_error("boom");
        }
    }
};



//
//  A listener whose callback can be made to block until released, for testing
//  removal of a listener that is currently running.
//
struct BlockingListener{
    std::atomic<bool> entered{false};
    std::atomic<bool> finished{false};
    std::atomic<bool> release{false};

    void ping(){
        entered.store(true, std::memory_order_release);
        while (!release.load(std::memory_order_acquire)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        //  Set inside the callback, before the node's lock is released:
        //  any remove() that returns has waited on that lock, so it is
        //  guaranteed to observe a finished callback.
        finished.store(true, std::memory_order_release);
    }
};



//
//  Collects the first failure of a test so the test can report one message.
//
class Checker{
public:
    bool ok() const{
        return m_error.empty();
    }
    const std::string& error() const{
        return m_error;
    }
    void expect(bool cond, std::string message){
        if (!cond && m_error.empty()){
            m_error = std::move(message);
        }
    }

private:
    std::string m_error;
};



//
//  A model of the call order that tests keep in sync with a ListenerSet and
//  compare against after every operation.
//
class Model{
public:
    void add(Checker& c, const char* tag, const std::string& name, bool front){
        auto iter = m_listeners.find(name);
        if (iter == m_listeners.end()){
            m_listeners.emplace(name, TestListener(name, m_log));
            if (front){
                m_order.insert(m_order.begin(), name);
            }else{
                m_order.push_back(name);
            }
        }
        TestListener& listener = m_listeners.at(name);
        if (front){
            set.push_front(listener);
        }else{
            set.push_back(listener);
        }
        check(c, tag);
    }
    void remove(Checker& c, const char* tag, const std::string& name){
        auto iter = m_listeners.find(name);
        if (iter != m_listeners.end()){
            TestListener& listener = iter->second;
            set.remove(listener);
            m_listeners.erase(iter);
            m_order.erase(std::find(m_order.begin(), m_order.end(), name));
        }
        check(c, tag);
    }
    void check(Checker& c, const char* tag){
        set.run_method(&TestListener::ping);
        std::string actual = m_log.take();
        std::string expected;
        for (auto& n : m_order){
            expected += n;
        }
        c.expect(
            actual == expected,
            std::string(tag) + ": expected call order '" + expected + "', got '" + actual + "'"
        );
        c.expect(
            set.count_unique() == m_order.size(),
            std::string(tag) + ": count_unique() is " + std::to_string(set.count_unique())
                + ", expected " + std::to_string(m_order.size())
        );
        c.expect(
            set.empty() == m_order.empty(),
            std::string(tag) + ": empty() disagrees with the expected call order"
        );
    }

private:
    CallLog m_log;
    std::vector<std::string> m_order;
    std::map<std::string, TestListener> m_listeners;
    ListenerSet<TestListener> set;
};



//
//  Insertions and removals at every position of the linked list: the first
//  node, the last node, the middle, and the only node, plus dedup semantics
//  and re-adding listeners after they were removed (fresh nodes).
//
class Test_ListenerSet_Ordering : public UnitTest{
public:
    Test_ListenerSet_Ordering()
        : UnitTest("Common::ListenerSet - Ordering and dedup")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Checker c;
        Model m;

        //  Insertion into an empty set: the new node becomes head and tail.
        m.add(c, "add() to empty", "a", false);
        m.add(c, "push_front() to empty", "b", true);
        m.add(c, "push_back() after two", "c", false);

        //  Dedup: re-adding a present listener is a no-op and does not change
        //  the call order.
        m.add(c, "add() present at back", "a", false);
        m.add(c, "push_front() present at front", "b", true);
        m.add(c, "push_back() present at back", "c", false);

        //  Grow at both ends, then remove from the front, back, and middle.
        m.add(c, "push_front()", "d", true);
        m.add(c, "push_back()", "e", false);
        m.remove(c, "remove() front of 5", "d");
        m.remove(c, "remove() back of 4", "e");
        m.remove(c, "remove() middle of 3", "a");
        m.remove(c, "remove() new front", "b");
        m.remove(c, "remove() only node", "c");

        //  Removing a listener that was never added is a no-op.
        m.remove(c, "remove() never-added", "x");

        //  Re-adding after removal creates fresh nodes: make sure the list
        //  links up cleanly from scratch.
        m.add(c, "add() after emptying", "a", false);
        m.add(c, "push_front() after emptying", "b", true);
        m.add(c, "dedup front", "a", true);
        m.add(c, "dedup back", "b", false);
        m.remove(c, "remove() back", "a");
        m.remove(c, "remove() front", "b");

        //  Repeatedly grow and shrink at both ends: the head and tail
        //  pointers must stay consistent across many fresh allocations.
        for (int round = 0; round < 3; round++){
            m.add(c, "grow front", "f", true);
            m.add(c, "grow back", "g", false);
            m.remove(c, "shrink front", "f");
            m.remove(c, "shrink back", "g");
        }

        if (!c.ok()){
            return UnitTestResult(c.error());
        }
        return true;
    }
};



//
//  try_remove() is non-blocking: it succeeds when the listener is absent or
//  idle, and fails when the listener is currently inside a callback.
//
class Test_ListenerSet_TryRemove : public UnitTest{
public:
    Test_ListenerSet_TryRemove()
        : UnitTest("Common::ListenerSet - try_remove()")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Checker c;
        CallLog log;
        ListenerSet<TestListener> set(true);
        std::map<std::string, TestListener> storage;
        auto listener = [&](const char* name) -> TestListener&{
            auto iter = storage.find(name);
            if (iter == storage.end()){
                iter = storage.emplace(name, TestListener(name, log)).first;
            }
            return iter->second;
        };

        //  Absent listener: nothing to remove, returns true.
        c.expect(
            set.try_remove(listener("a")),
            "try_remove() of an absent listener should return true"
        );

        set.add(listener("a"));
        set.add(listener("b"));
        c.expect(
            set.try_remove(listener("a")),
            "try_remove() of an idle listener should return true"
        );
        c.expect(
            set.count_unique() == 1,
            "count_unique() should be 1 after a successful try_remove()"
        );
        c.expect(
            set.try_remove(listener("a")),
            "try_remove() of an already-removed listener should return true"
        );

        //  While a listener is inside its callback, its node lock is held, so
        //  try_remove() must fail without blocking.
        BlockingListener b;
        ListenerSet<BlockingListener> set2(true);
        set2.add(b);
        std::thread runner([&]{
            set2.run_method(&BlockingListener::ping);
        });
        while (!b.entered.load(std::memory_order_acquire)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        c.expect(
            !set2.try_remove(b),
            "try_remove() while the listener is inside its callback should return false"
        );
        c.expect(
            set2.count_unique() == 1,
            "the listener should still be registered while its callback is running"
        );

        b.release.store(true, std::memory_order_release);
        runner.join();
        c.expect(
            set2.try_remove(b),
            "try_remove() after the callback finished should return true"
        );
        c.expect(set2.empty(), "the set should be empty after the final try_remove()");

        if (!c.ok()){
            return UnitTestResult(c.error());
        }
        return true;
    }
};



//
//  remove() of a listener that is currently running: it must block until the
//  callback finishes, only then remove the listener, and the callback must
//  run to completion.
//
class Test_ListenerSet_RemoveRunningListener : public UnitTest{
public:
    Test_ListenerSet_RemoveRunningListener()
        : UnitTest("Common::ListenerSet - remove() of a running listener")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Checker c;
        BlockingListener b;
        ListenerSet<BlockingListener> set(true);
        set.add(b);

        std::thread runner([&]{
            set.run_method(&BlockingListener::ping);
        });
        while (!b.entered.load(std::memory_order_acquire)){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        //  remove() must block until the running callback finishes.
        std::thread remover([&]{
            set.remove(b);
        });

        //  Give the remover time to start and block on the node's lock.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.expect(
            set.count_unique() == 1,
            "the listener should still be registered while remove() waits for its callback"
        );
        c.expect(
            !b.finished.load(std::memory_order_acquire),
            "the callback should still be running"
        );

        b.release.store(true, std::memory_order_release);
        runner.join();
        remover.join();

        //  remove() can only acquire the node's lock after the callback
        //  released it, and "finished" is stored inside the callback,
        //  so by the time remove() returns the callback has completed.
        c.expect(
            b.finished.load(std::memory_order_acquire),
            "remove() should only return after the callback finished"
        );
        c.expect(set.empty(), "the set should be empty after remove()");
        c.expect(
            set.count_unique() == 0,
            "count_unique() should be 0 after remove()"
        );

        //  A second remove() of the now-absent listener is a no-op.
        set.remove(b);
        c.expect(set.empty(), "the set should still be empty after a second remove()");

        if (!c.ok()){
            return UnitTestResult(c.error());
        }
        return true;
    }
};



//
//  Exception semantics: a listener that throws does not stop the rest of the
//  listeners from running, and the first exception is rethrown.
//
class Test_ListenerSet_Exceptions : public UnitTest{
public:
    Test_ListenerSet_Exceptions()
        : UnitTest("Common::ListenerSet - Exception propagation")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Checker c;
        CallLog log;
        std::map<std::string, TestListener> storage;
        auto listener = [&](const char* name, bool throws) -> TestListener&{
            auto iter = storage.emplace(name, TestListener(name, log)).first;
            iter->second.throws = throws;
            return iter->second;
        };

        //  run_method(): the middle listener throws; the others still run.
        {
            ListenerSet<TestListener> set(true);
            set.add(listener("a", false));
            set.add(listener("b", true));
            set.add(listener("c", false));
            std::string err;
            try{
                set.run_method(&TestListener::ping);
            }catch (std::runtime_error& e){
                err = e.what();
            }
            c.expect(
                err == "boom",
                "run_method() should rethrow the exception, got '" + err + "'"
            );
            c.expect(
                log.take() == "abc",
                "all listeners should run even though one threw"
            );
        }

        //  run_on_all(): the first listener throws via the lambda; the rest
        //  still run.
        {
            ListenerSet<TestListener> set(true);
            set.add(listener("d", false));
            set.add(listener("e", false));
            std::string err;
            try{
                set.run_on_all([&](TestListener& l){
                    log.record(l.name);
                    if (l.name == "d"){
                        throw std::runtime_error("first");
                    }
                    if (l.name == "e"){
                        throw std::runtime_error("second");
                    }
                    return false;
                });
            }catch (std::runtime_error& e){
                err = e.what();
            }
            c.expect(
                err == "first",
                "run_on_all() should rethrow the first exception, got '" + err + "'"
            );
            c.expect(
                log.take() == "de",
                "all listeners should run even though two threw"
            );
        }

        //  Early stop still applies when the exception happens later.
        {
            ListenerSet<TestListener> set(true);
            set.add(listener("f", false));
            set.add(listener("g", false));
            set.run_on_all([&](TestListener& l){
                log.record(l.name);
                return true;
            });
            c.expect(
                log.take() == "f",
                "run_on_all() should stop after a lambda returns true"
            );
        }

        if (!c.ok()){
            return UnitTestResult(c.error());
        }
        return true;
    }
};



//
//  Stress: many threads concurrently add and remove shared listeners while
//  callbacks are running, to shake out the intrusive pointer bookkeeping.
//
class Test_ListenerSet_ConcurrentChurn : public UnitTest{
public:
    static constexpr int LISTENER_COUNT = 16;
    static constexpr int WORKER_COUNT = 4;
    static constexpr int RUN_ITERATIONS = 300;

    Test_ListenerSet_ConcurrentChurn()
        : UnitTest("Common::ListenerSet - Concurrent add/remove")
    {
        m_threads = WORKER_COUNT;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Checker c;
        CallLog log;
        ListenerSet<TestListener> set(true);
        std::map<std::string, TestListener> storage;
        for (int i = 0; i < LISTENER_COUNT; i++){
            std::string name = "L" + std::to_string(i);
            storage.emplace(name, TestListener(name, log));
        }

        std::atomic<bool> stop{false};
        auto worker = [&](int id){
            int i = 0;
            while (!stop.load(std::memory_order_relaxed)){
                auto iter = storage.find("L" + std::to_string(i % LISTENER_COUNT));
                TestListener& listener = iter->second;
                if ((id + i) % 2 == 0){
                    set.add(listener);
                }else{
                    set.remove(listener);
                }
                i++;
            }
        };
        std::vector<std::thread> workers;
        for (int id = 0; id < WORKER_COUNT; id++){
            workers.emplace_back(worker, id);
        }
        for (int i = 0; i < RUN_ITERATIONS; i++){
            set.run_on_all([&](TestListener& l){
                log.record(l.name);
                return false;
            });
        }
        stop.store(true, std::memory_order_relaxed);
        for (auto& w : workers){
            w.join();
        }
        for (auto& item : storage){
            set.remove(item.second);
        }
        c.expect(set.empty(), "all listeners should have been removed");
        c.expect(
            set.count_unique() == 0,
            "count_unique() should be 0 after all listeners were removed"
        );

        if (!c.ok()){
            return UnitTestResult(c.error());
        }
        return true;
    }
};



} // anonymous namespace



void add_tests_ListenerSet(UnitTestDatabase& database){
    database.add<Test_ListenerSet_Ordering>();
    database.add<Test_ListenerSet_TryRemove>();
    database.add<Test_ListenerSet_RemoveRunningListener>();
    database.add<Test_ListenerSet_Exceptions>();
    database.add<Test_ListenerSet_ConcurrentChurn>();
}



}
