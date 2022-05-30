/*  Test Utils
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_TestUtils_H
#define PokemonAutomation_Tests_TestUtils_H

#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace PokemonAutomation{

// Parse filename base into words.
// Words are separated by '_'.
std::vector<std::string> parse_words(const std::string& filename_base);

// Convert string to int. Return true if succeed.
bool parse_int(const std::string& word, int& number);

// Convert string to float. Return true if succeed.
bool parse_float(const std::string& word, float& number);

// Load sprite counts from a text file.
// Each line is:
// <slug> (optional number of slug count, default 1)
bool load_sprite_count(const std::string& filepath, std::map<std::string, int>& sprites);

// Load a list of slugs from a text file.
// Each line is a slug.
bool load_slug_list(const std::string& filepath, std::vector<std::string>& sprites);


// Implement the dummy interface of BotBase so that we can run the test code
// that relies on a BotBase.
class DummyBotBase: public BotBase{
public:
    DummyBotBase(Logger& logger) : m_logger(logger) {}
    
    virtual Logger& logger() override { return m_logger; }

    virtual State state() const override { return State::RUNNING; }

    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) override {}

    virtual bool try_stop_all_commands() override { return true; }
    virtual void stop_all_commands() override {}

    virtual bool try_next_command_interrupt() override { return true; }
    virtual void next_command_interrupt() override {};

    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override { return true; }
    virtual void issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override {}
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) override { return BotBaseMessage(0, ""); }

    Logger& m_logger;
};

// Implement the dummy interface of VideoFeed so that we can test
// the video inference code that relies on a VideoFeed.
class DummyVideoFeed: public VideoFeed{
public:
    DummyVideoFeed() {}

    //  Can call from anywhere.
    virtual void async_reset_video() override {}

    virtual VideoSnapshot snapshot() override { return VideoSnapshot(); }
};

// Implement the dummy interface of VideoOverlay so that we can test
// the video inference code that relies on a VideoOverlay.
class DummyVideoOverlay: public VideoOverlay{
public:
    DummyVideoOverlay() {}

    virtual void add_box(const ImageFloatBox& box, Color color) override{}

    virtual void remove_box(const ImageFloatBox& box) override{}
};

// Implement the dummy interface of AudioFeed so that we can test
// the audio inference code that relies on an AudioFeed.
class DummyAudioFeed: public AudioFeed{
public:
    DummyAudioFeed() {}
    virtual void async_reset_audio() override {}

    virtual std::vector<AudioSpectrum> spectrums_since(size_t startingStamp) override { return std::vector<AudioSpectrum>(); }

    virtual std::vector<AudioSpectrum> spectrums_latest(size_t numLatestSpectrums) override { return std::vector<AudioSpectrum>(); }

    void add_overlay(size_t startingStamp, size_t endStamp, Color color) override {}
};




#define TEST_RESULT_EQUAL(result, target) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " result is " << (result) << " but should be " << (target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)


#define TEST_RESULT_COMPONENT_EQUAL(result, target, component_name) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " " << component_name << " result is " << (result) << " but should be " << \
                (target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)

#define TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC(result, target, component_name, print_func) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << " " << component_name << " result is " << print_func(result) << " but should be " << \
                print_func(target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)

#define TEST_RESULT_APPROXIMATE(result, target, threshold) \
    do { \
        if (std::fabs((result) - (target)) > (threshold)) {\
            std::cerr << "Error: " << __func__ << " result is " << (result) << " but should be close to " << (target) << \
                " with threshold: " << (threshold) << "." << std::endl; \
            return 1; \
        } \
    } while (0)



}

#endif