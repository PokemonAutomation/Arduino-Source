/*  QtThreadPool Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <string>
#include <thread>
#include <vector>
#include <QObject>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "QtThreadPool.h"
#include "QtThreadPool_Tests.h"

namespace PokemonAutomation{


namespace{

// Registry of the currently alive `TrackedObject`s and which worker owns each one.
// Objects register themselves on construction and unregister on destruction, so the
// test can tell whether an object it still owns was destroyed behind its back without
// ever touching freed memory.
struct LiveObjectRegistry{
    Mutex lock;
    std::map<const QObject*, size_t> owners;

    bool is_owned_by(const QObject* object, size_t owner){
        std::lock_guard<Mutex> lg(lock);
        auto iter = owners.find(object);
        return iter != owners.end() && iter->second == owner;
    }
};

class TrackedObject : public QObject{
public:
    TrackedObject(LiveObjectRegistry& registry, size_t owner)
        : m_registry(registry)
    {
        std::lock_guard<Mutex> lg(m_registry.lock);
        m_registry.owners[this] = owner;
    }
    ~TrackedObject(){
        std::lock_guard<Mutex> lg(m_registry.lock);
        m_registry.owners.erase(this);
    }

private:
    LiveObjectRegistry& m_registry;
};

} // anonymous namespace



// Many workers concurrently add an object to a `QtEventThreadPool`, check that it's
// still alive and still theirs, then remove it. This is the access pattern of many
// console sessions (e.g. parallel unit tests) each creating and destroying their
// `AudioPassthroughPairQtThread`, which lives on the pool.
//
// It catches races where the pool hands a busy event thread to a new owner. The new
// owner's `add_object()` then replaces, and so destroys, the object still in use by
// the previous owner, which shows up here as an object disappearing from the registry
// while its worker still owns it.
class Test_QtEventThreadPool_ConcurrentAddRemove : public UnitTest{
public:
    static constexpr size_t WORKERS = 8;
    static constexpr size_t ITERATIONS = 300;

    Test_QtEventThreadPool_ConcurrentAddRemove()
        : UnitTest("Common::QtEventThreadPool - Concurrent add/remove")
    {
        m_threads = WORKERS;
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        LiveObjectRegistry registry;
        QtEventThreadPool pool;

        Mutex error_lock;
        std::string first_error;
        auto report_error = [&](std::string error){
            std::lock_guard<Mutex> lg(error_lock);
            if (first_error.empty()){
                first_error = std::move(error);
            }
        };

        auto worker = [&](size_t owner){
            for (size_t c = 0; c < ITERATIONS; c++){
                QObject* object = pool.add_object([&]{
                    return std::make_unique<TrackedObject>(registry, owner);
                });
                if (!registry.is_owned_by(object, owner)){
                    report_error("Worker " + std::to_string(owner) + ": object not alive right after add_object().");
                    return;
                }

                //  Give other workers a chance to add and remove their objects.
                std::this_thread::yield();

                if (!registry.is_owned_by(object, owner)){
                    report_error("Worker " + std::to_string(owner) + ": object destroyed by another worker.");
                    return;
                }
                pool.remove_object(object);
            }
        };

        {
            std::vector<Thread> threads;
            for (size_t c = 0; c < WORKERS; c++){
                threads.emplace_back([&worker, c]{ worker(c); });
            }
        }   //  Thread destructors join.

        if (!first_error.empty()){
            return UnitTestResult(first_error);
        }
        std::lock_guard<Mutex> lg(registry.lock);
        if (!registry.owners.empty()){
            return UnitTestResult(std::to_string(registry.owners.size()) + " objects were never destroyed.");
        }
        return true;
    }
};



void add_tests_QtEventThreadPool(UnitTestDatabase& database){
    database.add<Test_QtEventThreadPool_ConcurrentAddRemove>();
}


}
