/*  Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnitTest_H
#define PokemonAutomation_UnitTest_H

#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"

namespace PokemonAutomation{

class CancellableScope;



struct UnitTestResult{
    enum Result{
        NOT_RUN,
        PASSED,
        FAILED,
        SKIPPED,
        OOM,
    };
    Result result;
    std::string message;

    UnitTestResult()
        : result(NOT_RUN)
    {}
    UnitTestResult(bool pass)
        : result(pass ? PASSED : FAILED)
    {}
    UnitTestResult(const char* p_message)
        : result(FAILED)
        , message(p_message)
    {}
    UnitTestResult(std::string p_message)
        : result(FAILED)
        , message(std::move(p_message))
    {}
    UnitTestResult(Result p_result, std::string p_message = "")
        : result(p_result)
        , message(std::move(p_message))
    {}
};



class UnitTest{
public:
    virtual ~UnitTest() = default;
    UnitTest(std::string name)
        : m_name(std::move(name))
    {}


    template <typename TestType, class... Args>
    static std::unique_ptr<UnitTest> make(Args&&... args){
        return std::make_unique<TestType>(std::forward<Args>(args)...);
    }

    // The test name, e.g.
    // "NintendoSwitch::UpdatePopupDetector - NintendoSwitch/UpdateMenuDetector/English_1_True.png".
    const std::string& name() const{
        return m_name;
    }
    uint64_t memory() const{
        return m_memory;
    }
    size_t threads() const{
        return m_threads;
    }

    //  Relative paths of the test images this test loads, relative to the test repo folder
    //  `UNIT_TEST_RESOURCE_PATH()`, e.g. "NintendoSwitch/UpdateMenuDetector/English_1_True.png".
    //  This is useful for tracking and managing image assets at runtime along with applying the
    //  correct image distortion to augment test coverage.
    virtual std::vector<std::string> image_paths() const{
        return {};
    }

    //  The label of the image distortion to apply to the test images in the current run.
    // Distortion label names are defined in the test folder.
    // TestImageDistortionManager loads the color distortion setting including label names
    // to the test framework.
    // Empty means to use the images as they are.
    const std::string& distortion_label() const{
        return m_distortion_label;
    }
    //  Call this before the main test function `run()` to set up the image distortion label.
    //  UnitTest instances should use this label to apply the correct distortion inside `run()`.
    //
    //  Current implementation assumes the runs with different distortions are executed serially
    //  on the same UnitTest instance, so we don't have thread-safe implementation regarding
    //  the distortion label here. But in future if we want more parallelism we may need to change
    //  it.
    void set_distortion_label(std::string label) const{
        m_distortion_label = std::move(label);
    }

    //  The test name, followed by the current distortion label in brackets if one is set, e.g.
    //  "NintendoSwitch::UpdatePopupDetector - NintendoSwitch/UpdateMenuDetector/English_1_True.png [Win-MYPIN]".
    //  Logs and test results use this name so that a failure shows which distortion broke the test.
    std::string name_with_distortion() const{
        return m_distortion_label.empty() ? m_name : m_name + " [" + m_distortion_label + "]";
    }

    // Called by the test framework to run this test.
    // Make sure it can be run in parallel with other UnitTest instances.
    //
    // If `distortion_label()` is called beforehand, need to apply the distortion augmentation inside `run()`
    // via `SerialPrograms/Source/Tests/TestImageDistortion.h:test_image_distortion_manager()`.
    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const = 0;


protected:
    const std::string m_name;
    uint64_t m_memory = 0;
    size_t m_threads = 1;

private:
    mutable std::string m_distortion_label;
};





//  Forward Declaration
class UnitTestDatabase;







}
#endif
