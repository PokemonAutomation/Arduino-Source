/*  Async Audio Inference
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "AudioInferenceSession.h"

#include <iostream>

namespace PokemonAutomation{



struct AudioInferenceSession::Callback{
    AudioInferenceCallback* callback;
    StatAccumulatorI32 stats;

    Callback(AudioInferenceCallback* p_callback)
        : callback(p_callback)
    {}
};



AudioInferenceSession::AudioInferenceSession(
    ProgramEnvironment& env, Logger& logger,
    AudioFeed& feed,
    std::chrono::milliseconds period
)
    : m_env(env)
    , m_logger(logger)
    , m_feed(feed)
    , m_period(period)
    , m_stop(false)
{}
AudioInferenceSession::~AudioInferenceSession(){
    stop();
}
void AudioInferenceSession::stop(){
    bool expected = false;
    if (!m_stop.compare_exchange_strong(expected, true)){
        return;
    }
    {
        std::unique_lock<std::mutex> lg(m_lock);
        m_cv.notify_all();
    }

    const double DIVIDER = std::chrono::milliseconds(1) / std::chrono::microseconds(1);
    const char* UNITS = " ms";

    for (Callback* callback : m_callback_list){
        callback->stats.log(m_logger, callback->callback->label(), UNITS, DIVIDER);
    }
}

void AudioInferenceSession::operator+=(AudioInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);

    auto iter = m_callback_map.find(&callback);
    if (iter != m_callback_map.end()){
        return;
    }

    Callback& entry = m_callback_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(&callback),
        std::forward_as_tuple(&callback)
    ).first->second;

    m_callback_list.emplace_back(&entry);
}
void AudioInferenceSession::operator-=(AudioInferenceCallback& callback){
    std::unique_lock<std::mutex> lg(m_lock);
    auto iter0 = m_callback_map.find(&callback);
    if (iter0 == m_callback_map.end()){
        return;
    }
    auto iter1 = std::find(m_callback_list.begin(), m_callback_list.end(), &iter0->second);
    m_callback_list.erase(iter1);
    m_callback_map.erase(iter0);
}

AudioInferenceCallback* AudioInferenceSession::run(std::chrono::milliseconds timeout){
    auto now = std::chrono::system_clock::now();
    auto wait_until = now + m_period;
    auto stop_time = timeout == std::chrono::milliseconds(0)
        ? std::chrono::system_clock::time_point::max()
        : wait_until + timeout;
    return run(stop_time);
}
AudioInferenceCallback* AudioInferenceSession::run(std::chrono::system_clock::time_point stop){
    using time_point = std::chrono::system_clock::time_point;

    auto now = std::chrono::system_clock::now();
    auto wait_until = now + m_period;

    size_t lastTimestamp = SIZE_MAX;
    // Stores new spectrums from audio feed. The newest spectrum (with largest timestamp) is at
    // the front of the vector.
    std::vector<std::shared_ptr<PokemonAutomation::AudioSpectrum>> spectrums;

    while (true){
        m_env.check_stopping();
        if (m_stop.load(std::memory_order_acquire)){
            return nullptr;
        }

        spectrums.clear();
        if (lastTimestamp == SIZE_MAX){
            m_feed.spectrums_latest(1, spectrums);
        } else{
            // Note: in this file we never consider the case that stamp may overflow.
            // It requires on the order of 1e10 years to overflow if we have about 25ms per stamp.
            m_feed.spectrums_since(lastTimestamp+1, spectrums);
        }
        if (spectrums.size() > 0){
            // spectrums[0] has the newest spectrum with the largest stamp:
            lastTimestamp = spectrums[0]->stamp;
        }

        std::unique_lock<std::mutex> lg(m_lock);
        for (Callback* callback : m_callback_list){
            std::cout << "Run callback on spectrums " << spectrums.size() << std::endl;
            time_point time0 = std::chrono::system_clock::now();
            bool done = callback->callback->process_spectrums(spectrums, m_feed);
            time_point time1 = std::chrono::system_clock::now();
            callback->stats += std::chrono::duration_cast<std::chrono::microseconds>(time1 - time0).count();
            if (done){
                return callback->callback;
            }
        }

        now = std::chrono::system_clock::now();
        if (now >= stop){
            return nullptr;
        }
        auto wait = wait_until - now;
        if (wait <= std::chrono::milliseconds(0)){
            wait_until = now + m_period;
        }else{
            m_cv.wait_for(
                lg, wait,
                [=]{
                    auto now = std::chrono::system_clock::now();
                    return
                        now >= stop ||
                        now >= wait_until ||
                        m_env.is_stopping() ||
                        m_stop.load(std::memory_order_acquire);
                }
            );
            wait_until += m_period;
        }
    }
}



AsyncAudioInferenceSession::AsyncAudioInferenceSession(
    ProgramEnvironment& env, Logger& logger,
    AudioFeed& feed,
    std::chrono::milliseconds period
)
    : AudioInferenceSession(env, logger, feed, period)
    , m_callback(nullptr)
    , m_task(env.dispatcher().dispatch([this]{ thread_body(); }))
{}
AsyncAudioInferenceSession::~AsyncAudioInferenceSession(){
    AudioInferenceSession::stop();
}
AudioInferenceCallback* AsyncAudioInferenceSession::stop(){
    AudioInferenceSession::stop();
    if (m_task){
        m_task->wait_and_rethrow_exceptions();
    }
    return m_callback;
}
void AsyncAudioInferenceSession::thread_body(){
    try{
        m_callback = run();
    }catch (CancelledException&){}
}






}
