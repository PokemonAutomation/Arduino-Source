/*  Test Utils
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_TestUtils_H
#define PokemonAutomation_Tests_TestUtils_H

#include "CommonFramework/VideoPipeline/VideoOverlay.h"

#include <iostream>

namespace PokemonAutomation{


// Implement the dummy interface of VideoOverlay so that we can test
// the video inference code that relies on a VideoOverlay.
class DummyVideoOverlay: public VideoOverlay{
public:
    DummyVideoOverlay() {}

    virtual void add_box(const ImageFloatBox& box, Color color) override{}

    virtual void remove_box(const ImageFloatBox& box) override{}
};


#define TEST_RESULT_EQUAL(result, target) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " result is " << (result) << " but should be " << (target) << "." << std::endl; \
            return 1; \
        } \
        return 0; \
    } while (0)


#define TEST_RESULT_COMPONENT_EQUAL(result, target, component_name) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " " << component_name << " result is " << (result) << " but should be " << (target) << "." << std::endl; \
            return 1; \
        } \
        return 0; \
    } while (0)

#define TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC(result, target, component_name, print_func) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " " << component_name << " result is " << print_func(result) << " but should be " << print_func(target) << "." << std::endl; \
            return 1; \
        } \
        return 0; \
    } while (0)

}

#endif