/*  Test Utils
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  
 */


#ifndef PokemonAutomation_Tests_TestUtils_H
#define PokemonAutomation_Tests_TestUtils_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "CommonFramework/AudioPipeline/AudioFeed.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace PokemonAutomation{

// Parse filename base into words.
// Words are separated by '_'.
std::vector<std::string> parse_words(const std::string& filename_base);

// Convert string to size_t. Return true if succeed.
bool parse_size_t(const std::string& word, size_t& number);

// Convert string to int. Return true if succeed.
bool parse_int(const std::string& word, int& number);

// Convert string to float. Return true if succeed.
bool parse_float(const std::string& word, float& number);

// Convert string of "True" or "False" to bool. Return true if succeed.
bool parse_bool(const std::string& word, bool& value);

// Load sprite counts from a text file.
// Each line is:
// <slug> (optional number of slug count, default 1)
bool load_sprite_count(const std::string& filepath, std::map<std::string, int>& sprites);

// Load a list of slugs from a text file.
// Each line is a slug.
bool load_slug_list(const std::string& filepath, std::vector<std::string>& sprites);


// Implement the dummy interface of BotBase so that we can run the test code
// that relies on a BotBase.
class DummyBotBase : public BotBaseController{
public:
    DummyBotBase(Logger& logger) : m_logger(logger) {}

    virtual void stop(std::string error_message) override{}
    
    virtual Logger& logger() override { return m_logger; }

    virtual State state() const override { return State::RUNNING; }
    virtual size_t queue_limit() const override { return PABB_DEVICE_MINIMUM_QUEUE_SIZE; }

    virtual void notify_all() override{}

    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) override {}
    virtual void stop_all_commands() override {}
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

    virtual void add_frame_listener(VideoFrameListener& listener) override{}
    virtual void remove_frame_listener(VideoFrameListener& listener) override{}

    //  Can call from anywhere.
    virtual void reset() override{}

    virtual VideoSnapshot snapshot_latest_blocking() override{ return VideoSnapshot(); }
    virtual VideoSnapshot snapshot_recent_nonblocking(WallClock min_time) override{ return VideoSnapshot(); }

    virtual double fps_source() const override{ return 0; }
    virtual double fps_display() const override{ return 0; }
};

// Implement the dummy interface of VideoOverlay so that we can test
// the video inference code that relies on a VideoOverlay.
class DummyVideoOverlay: public VideoOverlay{
public:
    DummyVideoOverlay() {}

    virtual void add_box(const OverlayBox& box) override{}
    virtual void remove_box(const OverlayBox& box) override{}

    virtual void add_text(const OverlayText& text) override{}
    virtual void remove_text(const OverlayText& text) override{}

    virtual void add_image(const OverlayImage& image) override{}
    virtual void remove_image(const OverlayImage& image) override{}

    virtual void add_log(std::string message, Color color) override{}
    virtual void clear_log() override{}

    virtual void add_stat(OverlayStat& stat) override{}
    virtual void remove_stat(OverlayStat& stat) override{}
};

// Implement the dummy interface of AudioFeed so that we can test
// the audio inference code that relies on an AudioFeed.
class DummyAudioFeed: public AudioFeed{
public:
    DummyAudioFeed() {}
    virtual void reset() override {}

    virtual std::vector<AudioSpectrum> spectrums_since(uint64_t starting_seqnum) override { return std::vector<AudioSpectrum>(); }

    virtual std::vector<AudioSpectrum> spectrums_latest(size_t num_last_spectrums) override { return std::vector<AudioSpectrum>(); }

    void add_overlay(uint64_t starting_seqnum, size_t end_seqnum, Color color) override {}
};




#define TEST_RESULT_EQUAL(result, target) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << ":" << __LINE__ << " result is " << (result) << " but should be " << (target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)


#define TEST_RESULT_COMPONENT_EQUAL(result, target, component_name) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << ":" << __LINE__ << " " << component_name << " result is " << (result) << " but should be " << \
                (target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)

#define TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC(result, target, component_name, print_func) \
    do { \
        if ((result) != (target)) {\
            std::cerr << "Error: " << __func__ << ":" << __LINE__ << " " << component_name << " result is " << print_func(result) << " but should be " << \
                print_func(target) << "." << std::endl; \
            return 1; \
        } \
    } while (0)

#define TEST_RESULT_APPROXIMATE(result, target, threshold) \
    do { \
        if (std::fabs((result) - (target)) > (threshold)) {\
            std::cerr << "Error: " << __func__ << ":" << __LINE__ << " result is " << (result) << " but should be close to " << (target) << \
                " with threshold: " << (threshold) << "." << std::endl; \
            return 1; \
        } \
    } while (0)



}

#endif
