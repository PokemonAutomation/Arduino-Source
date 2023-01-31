
#ifdef PA_SLEEPY

#include <deque>
#include <condition_variable>
#include <unordered_map>
#include <thread>
#include <QFile>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/StringTools.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "IntegrationsAPI.h"
#include "SleepyDiscordRunner.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace SleepyDiscordRunner{

using namespace SleepyDiscord;
const char* enum_str_callback[] = {
    "Fault",
    "API",
    "Disconnected",
    "Connected",
    "Settings Initialized",
    "Settings Updated",
    "Commands Initialized",
    "Callbacks Set",
    "Invalid Command",
    "Terminating",
    "Remove File",
};

const char* enum_str_command[] = {
    "Click",
    "Click DPad",

    "Left Stick",
    "Right Stick",

    "Screenshot JPG",
    "Screenshot PNG",
    "Start",
    "Stop",
    "Shutdown",
    "Get Connected Bots",
    "Reload Settings",
    "Reset Camera",
    "Reset Serial",
    "Terminate",

    "Hi",
    "Ping",
    "About",
    "Help",
};

const char* enum_str_state[] = {
    "STOPPED",
    "RUNNING",
    "FINISHED",
    "STOPPING",
};

std::unordered_map<uint16_t, std::string> str_buttons = {
    { 1, "Y" },
    { 2, "B" },
    { 4, "A" },
    { 8, "X" },
    { 16, "L" },
    { 32, "R" },
    { 64, "ZL" },
    { 128, "ZR" },
    { 256, "MINUS" },
    { 512, "PLUS" },
    { 1024, "LStick" },
    { 2048, "RStick" },
    { 4096, "HOME" },
    { 8192, "Capture" },
};

std::unordered_map<uint16_t, std::string> str_dpad = {
    { 0, "DUp" },
    { 2, "DRight" },
    { 4, "DDown" },
    { 6, "DLeft" },
};


Logger& sleepy_logger(){
    static TaggedLogger logger(global_logger_raw(), "SleepyDiscord");
    return logger;
}


struct SleepyDiscordRequest {
    SleepyDiscordRequest() = default;
    SleepyDiscordRequest(std::string embed, std::string channels, std::string messages, std::shared_ptr<PendingFileSend> file) :
        embed(std::move(embed)),
        channels(std::move(channels)),
        messages(std::move(messages)),
        file(std::move(file)) {}

    std::string embed;
    std::string channels;
    std::string messages;
    std::shared_ptr<PendingFileSend> file;
};

class SleepyDiscordSender {
private:
    SleepyDiscordSender()
//        : m_stopping(false)
//        , m_thread(run_with_catch, "SleepyDiscordSender::thread_loop()", [this]{ thread_loop(); })
        : m_dispatcher(nullptr, 1)
        , m_queue(m_dispatcher)
    {}
    ~SleepyDiscordSender() {
//        {
//            std::lock_guard<std::mutex> lg(m_lock);
//            m_stopping = true;
//            m_cv.notify_all();
//        }
//        m_thread.join();
    }

public:
    static SleepyDiscordSender& instance() {
        static SleepyDiscordSender sender;
        return sender;
    }

    void send(
        std::string embed,
        std::string channels,
        std::chrono::milliseconds delay,
        std::string messages,
        std::shared_ptr<PendingFileSend> file
    ){
//        std::lock_guard<std::mutex> lg(m_lock);
        m_queue.add_event(
            delay > std::chrono::milliseconds(10000) ? std::chrono::milliseconds(0) : delay,
            [embed = std::move(embed), channels = std::move(channels), messages = std::move(messages), file = std::move(file)]() mutable {
                if (file == nullptr || file->filepath().empty()){
                    sendMessage(&channels[0], &messages[0], &embed[0], nullptr);
                }else{
                    std::string filepath = file->filepath();
                    sendMessage(&channels[0], &messages[0], &embed[0], &filepath[0]);
                }
            }
        );
        sleepy_logger().log("Sending notification... (queue = " + tostr_u_commas(m_queue.size()) + ")", COLOR_PURPLE);
//        m_queue.emplace_back(embed, channels, messages, std::move(file));
//        m_cv.notify_all();

    }

private:
#if 0
    void thread_loop() {
        while (true) {
            SleepyDiscordRequest item;
            {
                std::unique_lock<std::mutex> lg(m_lock);
                if (m_stopping) {
                    break;
                }
                if (m_queue.empty()) {
                    m_cv.wait(lg);
                    continue;
                }

                item = std::move(m_queue.front());
                m_queue.pop_front();
            }

            if (item.file != nullptr && !item.file->filepath().empty()) {
                std::string filepath = item.file->filepath();
                sendMessage(&item.channels[0], &item.messages[0], &item.embed[0], &filepath[0]);
            }
            else sendMessage(&item.channels[0], &item.messages[0], &item.embed[0], nullptr);
        }
    }
#endif

private:
//    bool m_stopping;
//    std::mutex m_lock;
//    std::condition_variable m_cv;
//    std::deque<SleepyDiscordRequest> m_queue;
//    std::thread m_thread;
    AsyncDispatcher m_dispatcher;
    ScheduledTaskRunner m_queue;
};

class SleepyDiscordClient;
std::unique_ptr<SleepyDiscordClient> m_sleepy_client;

std::mutex m_connect_lock;
std::mutex m_client_lock;





struct CommandArgs{
    uint64_t id;
    uint16_t button;
    uint16_t hold_ticks;
    uint8_t x;
    uint8_t y;
};


class SleepyDiscordClient {
public:
    bool m_connected = false;

public:
    void send(
        std::string embed,
        std::string channels,
        std::chrono::milliseconds delay,
        std::string messages,
        std::shared_ptr<PendingFileSend> file
    ) {
        if (m_sleepy_client != nullptr) {
            if (file){
//                cout << "Sending: " << file->filepath().toStdString() << endl;
                m_active_list.emplace(file->filepath(), file);
            }
            SleepyDiscordSender::instance().send(embed, channels, delay, messages, std::move(file));
        }else{
            sleepy_logger().log("SleepyDiscordClient::send(): Not connected.", COLOR_RED);
        }
    }

    void callback(int response, const char* message) {
        if (m_sleepy_client == nullptr) {
            return;
        }

        std::string msg = (std::string)message + " (Callback: " + (std::string)enum_str_callback[response] + ")";
        Color color = response == SleepyResponse::Disconnected || response == SleepyResponse::Fault ? COLOR_RED : COLOR_PURPLE;

        switch (response) {
        case SleepyResponse::Connected: m_connected = true; break;
        case SleepyResponse::Disconnected: m_connected = false; break;
        case SleepyResponse::RemoveFile:{
#if 1
            auto iter = m_active_list.find(message);
//            cout << "Removing: " << message << endl;
//            cout << "m_active_list = " << m_active_list.size() << endl;
            if (iter != m_active_list.end()){
                msg = "Marking sent file as done. (Callback: " + (std::string)enum_str_callback[response] + ")";
                m_active_list.erase(iter);
            }else{
                msg = "Unknown sent file. (Callback: " + (std::string)enum_str_callback[response] + ")";
//                color = COLOR_RED;
            }
            m_active_list.erase(message);
#else
            bool success = QFile(message).remove();
            if (success){
                msg = "Removed sent file. (Callback: " + (std::string)enum_str_callback[response] + ")";
            }else{
                msg = "Failed to remove sent file. (Callback: " + (std::string)enum_str_callback[response] + ")";
                color = COLOR_RED;
            }
#endif
            break;
        }
        }

        sleepy_logger().log(msg, color);
    }

    void cmd_callback(SleepyRequest request, char* channel, uint64_t id, uint16_t button, uint16_t hold_ticks, uint8_t x, uint8_t y) {
        if (m_sleepy_client == nullptr) {
            return;
        }

        cout << "cmd_callback(): " << request << endl;

        std::string cmd = "Received command: " + (std::string)enum_str_command[request] + ".";
        sleepy_logger().log(cmd, COLOR_PURPLE);

        switch (request) {
        case SleepyRequest::Click:
            run_Click(channel, id, hold_ticks, button);
            break;
        case SleepyRequest::DPad:
            run_DPad(channel, id, hold_ticks, button);
            break;
        case SleepyRequest::SetLStick:
            run_SetLStick(channel, id, hold_ticks, x, y);
            return;
        case SleepyRequest::SetRStick:
            run_SetRStick(channel, id, hold_ticks, x, y);
            return;
        case SleepyRequest::ScreenshotJpg:
            run_ScreenshotJpg(channel, id);
            return;
        case SleepyRequest::ScreenshotPng:
            run_ScreenshotPng(channel, id);
            return;
        case SleepyRequest::Start:
            run_start(channel, id);
            break;
        case SleepyRequest::Stop:
            run_stop(channel, id);
            break;
        case SleepyRequest::Shutdown:
            run_Shutdown();
            return;
        case SleepyRequest::GetConnectedBots:
            run_GetConnectedBots(channel);
            return;
        case SleepyRequest::ReloadSettings:
            run_ReloadSettings(channel);
            return;
        case SleepyRequest::ResetCamera:
            run_ResetCamera(channel, id);
            return;
        case SleepyRequest::ResetSerial:
            run_ResetSerial(channel, id);
            return;
        }
    }



private:
    void send_response(SleepyRequest request, char* channel, std::string message, std::shared_ptr<PendingFileSend> file = nullptr){
        std::string filename = file == nullptr ? "" : file->filepath();
        if ((int)request <= 11) {
            program_response(request, channel, &message[0], &filename[0]);
        }else{
            send("", channel, std::chrono::milliseconds(0), &message[0], std::move(file));
        }
    }

    void run_Click(char* channel, uint64_t id, uint16_t hold_ticks, uint16_t button){
        std::string message;
        auto it = str_buttons.find(button);
        if (it == str_buttons.end()) {
            message = "Unrecognized button input";
        }else{
            message = Integration::press_button(id, button, hold_ticks);
        }
        if (message.empty()){
            message = "Console ID " + std::to_string(id) + " clicked " + it->second + ".";
        }
        send_response(SleepyRequest::Click, channel, message);
    }
    void run_DPad(char* channel, uint64_t id, uint16_t hold_ticks, uint16_t button){
        std::string message;
        auto it = str_dpad.find(button);
        if (it == str_dpad.end()) {
            message = "Unrecognized button input";
        }else{
            message = Integration::press_dpad(id, button, hold_ticks);
        }
        if (message.empty()){
            message = "Console ID " + std::to_string(id) + " clicked " + it->second + ".";
        }
        send_response(SleepyRequest::DPad, channel, message);
    }
    void run_SetLStick(char* channel, uint64_t id, uint16_t hold_ticks, uint8_t x, uint8_t y){
        std::string message = Integration::press_left_joystick(id, x, y, hold_ticks);
        if (message.empty()){
            message = "Console ID " + std::to_string(id) + " moved the left joystick.";
        }
        send_response(SleepyRequest::SetLStick, channel, message);
    }
    void run_SetRStick(char* channel, uint64_t id, uint16_t hold_ticks, uint8_t x, uint8_t y){
        std::string message = Integration::press_right_joystick(id, x, y, hold_ticks);
        if (message.empty()){
            message = "Console ID " + std::to_string(id) + " moved the right joystick.";
        }
        send_response(SleepyRequest::SetRStick, channel, message);
    }
    void run_ScreenshotPng(char* channel, uint64_t id){
        std::string filepath = "capture.png";
        std::string message = Integration::screenshot(id, filepath.c_str());
        if (!message.empty()){
            send_response(SleepyRequest::ScreenshotPng, channel, message);
            return;
        }else{
            message = "Captured image from console ID " + std::to_string(id) + ".";
        }

        std::shared_ptr<PendingFileSend> file(new PendingFileSend(filepath, true));
        send_response(SleepyRequest::ScreenshotPng, channel, message, std::move(file));
    }
    void run_ScreenshotJpg(char* channel, uint64_t id){
        std::string filepath = "capture.jpg";
        std::string message = Integration::screenshot(id, filepath.c_str());
        if (!message.empty()){
            send_response(SleepyRequest::ScreenshotJpg, channel, message);
            return;
        }else{
            message = "Captured image from console ID " + std::to_string(id) + ".";
        }

        std::shared_ptr<PendingFileSend> file(new PendingFileSend(filepath, true));
        send_response(SleepyRequest::ScreenshotJpg, channel, message, std::move(file));
    }
    void run_start(char* channel, uint64_t id){
        std::string message = Integration::start_program(id);
        if (!message.empty()){
            send_response(SleepyRequest::Start, channel, message);
            return;
        }else{
            message = "Console ID " + std::to_string(id) + " started the program.";
        }
        send_response(SleepyRequest::Start, channel, message);
    }
    void run_stop(char* channel, uint64_t id){
        std::string message = Integration::stop_program(id);
        if (!message.empty()){
            send_response(SleepyRequest::Stop, channel, message);
            return;
        }else{
            message = "Console ID " + std::to_string(id) + " stopped the program.";
        }
        send_response(SleepyRequest::Stop, channel, message);
    }
    void run_Shutdown(){
        program_response(SleepyRequest::Terminate);
        m_sleepy_client.reset();

        //  TODO: Can't do this since it skips all the shutdown sequence from all the other threads.
        exit(0);
    }
    void run_GetConnectedBots(char* channel){
        std::string message = Integration::status();
        send_response(SleepyRequest::GetConnectedBots, channel, message);
    }
    void run_ReloadSettings(char* channel){
        std::string message = initialize_sleepy_settings()
            ? "Successfully reloaded Discord settings."
            : "Failed to reload Discord settings.";
        send_response(SleepyRequest::ReloadSettings, channel, message);
    }
    void run_ResetCamera(char* channel, uint64_t id){
        std::string message = Integration::reset_camera(id);
        send_response(SleepyRequest::ResetCamera, channel, message.empty() ? "Camera was reset." : message);
    }
    void run_ResetSerial(char* channel, uint64_t id){
        std::string message = Integration::reset_serial(id);
        send_response(SleepyRequest::ResetSerial, channel, message);
    }

    std::map<std::string, std::shared_ptr<PendingFileSend>> m_active_list;
};







bool is_running(){
    std::lock_guard<std::mutex> lg(m_connect_lock);
    return m_sleepy_client != nullptr;
}

bool is_connected(){
    std::lock_guard<std::mutex> lg(m_connect_lock);
    return m_sleepy_client != nullptr && m_sleepy_client->m_connected;
}

void sleepy_connect(){
    std::lock_guard<std::mutex> lg(m_connect_lock);
    {
//        std::lock_guard<std::mutex> lg(m_client_lock);
//        if (!PreloadSettings::instance().DEVELOPER_MODE){
//            return;
//        }
        if (m_sleepy_client != nullptr){
            sleepy_logger().log("sleepy_connect(): Already initialized!", COLOR_PURPLE);
            return;
        }
        if (!initialize_sleepy_settings()){
            sleepy_logger().log("sleepy_connect(): initialize_sleepy_settings() failed.", COLOR_RED);
            return;
        }
        sleepy_logger().log("Connecting...", COLOR_PURPLE);
    }
    client_connect();
    sleepy_logger().log("Finished Connecting...", COLOR_PURPLE);
}

void sleepy_terminate() {
    std::lock_guard<std::mutex> lg(m_client_lock);
    program_response(SleepyRequest::Terminate);
    if (m_sleepy_client != nullptr) {
        m_sleepy_client.reset();
    }
}

void sleepy_response(int response, char* message) {
    //std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_sleepy_client != nullptr) {
        return m_sleepy_client->callback(response, message);
    }
}

void sleepy_cmd_response(int request, char* channel, uint64_t console_id, uint16_t button, uint16_t hold_ticks, uint8_t x, uint8_t y) {
    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_sleepy_client != nullptr) {
        return m_sleepy_client->cmd_callback((SleepyRequest)request, channel, console_id, button, hold_ticks, x, y);
    }
}

void send_embed_sleepy(
    bool should_ping,
    const std::vector<std::string>& tags,
    const JsonObject& embed,
    std::shared_ptr<PendingFileSend> file
){
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!settings.integration.enabled()){
        return;
    }

    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_sleepy_client == nullptr){
        return;
    }

    MessageBuilder builder(tags);
    const DiscordIntegrationTable& channels = settings.integration.channels;

    std::vector<std::unique_ptr<DiscordIntegrationChannel>> table = channels.copy_snapshot();
    for (size_t i = 0; i < table.size(); i++){
        const Integration::DiscordIntegrationChannel& channel = *table[i];
        if (((std::string)channel.tags_text).empty() || !channel.enabled){
            continue;
        }
        if (!builder.should_send(EventNotificationOption::parse_tags(channel.tags_text))){
            continue;
        }

        sleepy_logger().log("send_message_sleepy(): Sending...", COLOR_PURPLE);
        m_sleepy_client->send(
            embed.dump(),
            channel.channel_id,
            std::chrono::seconds(channel.delay),
            builder.build_message(
                should_ping && channel.ping,
                settings.message.user_id,
                settings.message.message
            ),
            file == nullptr ? nullptr : file
        );
    }
}


bool initialize_sleepy_settings() {
    //  Must be called inside lock.
//    std::lock_guard<std::mutex> lg(m_client_lock);
    DiscordSettingsOption& settings = GlobalSettings::instance().DISCORD;
    if (!check_if_empty(settings)) {
        return false;
    }

    bool suffix = settings.integration.use_suffix;
    std::string param_string;
    param_string += StringTools::replace(settings.integration.token, " ", "") + "|";
    param_string += (std::string)settings.integration.command_prefix + "|";
    param_string += StringTools::replace(settings.integration.owner, " ", "") + "|";
    param_string += (std::string)settings.integration.game_status + "|";
    param_string += StringTools::replace(settings.integration.hello_message, "@", "") + "|";
    param_string += PROGRAM_VERSION + "|";
    param_string += PROJECT_SOURCE_URL;

    std::string sudo = StringTools::replace(settings.integration.sudo, " ", "");

//    std::string w_channels = StringTools::replace(settings.integration.channels_whitelist.get(), " ", "");
    std::string w_channels;
    for (const std::string& channel_id : settings.integration.channels.command_channels()){
        if (!w_channels.empty()){
            w_channels += ",";
        }
        w_channels += channel_id;
    }

    m_sleepy_client = std::unique_ptr<SleepyDiscordClient>(new SleepyDiscordClient());
    apply_settings(sleepy_response, sleepy_cmd_response, &w_channels[0], &sudo[0], &param_string[0], suffix);
    return true;
}

bool check_if_empty(const DiscordSettingsOption& settings) {

    if (!settings.integration.enabled()){
        return false;
    }
    if (((std::string)settings.integration.token).empty()) {
        return false;
    }
    else if (((std::string)settings.integration.token).find(",") != std::string::npos) {
        sleepy_logger().log("\"Token\" must only contain one token. Stopping...", COLOR_RED);
        return false;
    }
    else if (((std::string)settings.integration.owner).find(",") != std::string::npos) {
        sleepy_logger().log("\"Owner\" must only contain one Discord ID (yours). Stopping...", COLOR_RED);
        return false;
    }
    else if (((std::string)settings.integration.command_prefix).empty()) {
        sleepy_logger().log("Please enter a Discord command prefix. Stopping...", COLOR_RED);
        return false;
    }
    return true;
}





}
}
}
#endif
