
#ifdef PA_SLEEPY

#include <deque>
#include <condition_variable>
#include <unordered_map>
#include <thread>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/Logger.h"
#include "IntegrationsAPI.h"
#include "SleepyDiscordRunner.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Integration{
namespace SleepyDiscordRunner{

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
    SleepyDiscordRequest(std::string json, std::string channel, std::string message, std::shared_ptr<PendingFileSend> file) :
        json(std::move(json)),
        channel(std::move(channel)),
        message(std::move(message)),
        file(std::move(file)) {}

    std::string json;
    std::string channel;
    std::string message;
    std::shared_ptr<PendingFileSend> file;
};

class SleepyDiscordSender {
private:
    SleepyDiscordSender() : m_stopping(false), m_thread(&SleepyDiscordSender::thread_loop, this) {}
    ~SleepyDiscordSender() {
        {
            std::lock_guard<std::mutex> lg(m_lock);
            m_stopping = true;
            m_cv.notify_all();
        }
        m_thread.join();
    }

public:
    static SleepyDiscordSender& instance() {
        static SleepyDiscordSender sender;
        return sender;
    }

    void send(std::string json, std::string channel, std::string message, std::shared_ptr<PendingFileSend> file) {
        std::lock_guard<std::mutex> lg(m_lock);
        sleepy_logger().log("Sending notification... (queue = " + tostr_u_commas(m_queue.size()) + ")", "purple");
        m_queue.emplace_back(json, channel, message, std::move(file));
        m_cv.notify_all();
    }

private:
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

            if (item.file != nullptr && !item.file->filepath().isEmpty()) {
                std::string filepath = item.file->filepath().toStdString();
                sendFile(&filepath[0], &item.channel[0], &item.message[0], &item.json[0]);
            }
            else if (item.json != "") {
                sendEmbed(&item.message[0], &item.json[0]);
            }
            else sendLog(&item.message[0]);
        }
    }

private:
    bool m_stopping;
    std::mutex m_lock;
    std::condition_variable m_cv;
    std::deque<SleepyDiscordRequest> m_queue;
    std::thread m_thread;
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
    void send(std::string json, std::string channel, std::string message, std::shared_ptr<PendingFileSend> file) {
        //  TODO: Once file cleanup works, remove this.
        if (file){
            file->extend_lifetime();
        }

        if (m_connected) {
            SleepyDiscordSender::instance().send(json, channel, message, std::move(file));
//            sleepy_logger().log("SleepyDiscordClient::send(): Destroying file.", Qt::red);
        }else{
            sleepy_logger().log("SleepyDiscordClient::send(): Not connected.", Qt::red);
        }
    }

    void callback(int response, const char* message) {
        if (m_sleepy_client == nullptr) {
            return;
        }

        std::string msg = (std::string)message + " (Callback: " + (std::string)enum_str_callback[response] + ")";
        const char* color = response == SleepyResponse::Disconnected || response == SleepyResponse::Fault ? "red" : "purple";

        switch (response) {
        case SleepyResponse::Connected: m_connected = true; break;
        case SleepyResponse::Disconnected: m_connected = false; break;
        }

        if (response != (int)SleepyResponse::API){
            send_log_sleepy(msg);
        }
        sleepy_logger().log(msg, color);
    }

    void cmd_callback(SleepyRequest request, char* channel, uint64_t id, uint16_t button, uint16_t hold_ticks, uint8_t x, uint8_t y) {
        if (m_sleepy_client == nullptr) {
            return;
        }

        cout << "cmd_callback(): " << request << endl;

        std::string cmd = "Received command: " + (std::string)enum_str_command[request] + ".";
        send_log_sleepy(cmd);
        sleepy_logger().log(cmd, "purple");

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
        }
    }



private:
    void send_response(SleepyRequest request, char* channel, std::string message, std::shared_ptr<PendingFileSend> file = nullptr){
        //  TODO: Once file cleanup works, remove this.
        if (file){
            file->extend_lifetime();
        }

        std::string filename = file == nullptr ? "" : file->filepath().toStdString();
        if ((int)request <= 11) {
            program_response(request, channel, &message[0], &filename[0]);
        }else{
            send("", channel, &message[0], std::move(file));
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
            send_log_sleepy(message);
            send_response(SleepyRequest::ScreenshotPng, channel, message);
            return;
        }else{
            message = "Captured image from console ID " + std::to_string(id) + ".";
        }

        std::shared_ptr<PendingFileSend> file(new PendingFileSend(QString::fromStdString(filepath), true));
        send_response(SleepyRequest::ScreenshotPng, channel, message, std::move(file));
    }
    void run_ScreenshotJpg(char* channel, uint64_t id){
        std::string filepath = "capture.jpg";
        std::string message = Integration::screenshot(id, filepath.c_str());
        if (!message.empty()){
            send_log_sleepy(message);
            send_response(SleepyRequest::ScreenshotJpg, channel, message);
            return;
        }else{
            message = "Captured image from console ID " + std::to_string(id) + ".";
        }

        std::shared_ptr<PendingFileSend> file(new PendingFileSend(QString::fromStdString(filepath), true));
        send_response(SleepyRequest::ScreenshotJpg, channel, message, std::move(file));
    }
    void run_start(char* channel, uint64_t id){
        std::string message = Integration::start_program(id);
        if (!message.empty()){
            send_log_sleepy(message);
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
            send_log_sleepy(message);
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



private:
    bool m_connected = false;
};







bool is_running(){
    std::lock_guard<std::mutex> lg(m_connect_lock);
    return m_sleepy_client != nullptr;
}
void sleepy_connect(){
    std::lock_guard<std::mutex> lg(m_connect_lock);
    {
//        std::lock_guard<std::mutex> lg(m_client_lock);
        if (!GlobalSettings::instance().DEVELOPER_MODE){
            return;
        }
        if (m_sleepy_client != nullptr){
            sleepy_logger().log("sleepy_connect(): Already initialized!", "purple");
            return;
        }
        if (!initialize_sleepy_settings()){
            sleepy_logger().log("sleepy_connect(): initialize_sleepy_settings() failed.", Qt::red);
            return;
        }
        sleepy_logger().log("Connecting...", "purple");
    }
    client_connect();
    sleepy_logger().log("Finished Connecting...", "purple");
}

void sleepy_terminate() {
//    std::lock_guard<std::mutex> lg(m_client_lock);
    program_response(SleepyRequest::Terminate);
    if (m_sleepy_client != nullptr) {
        m_sleepy_client.reset();
    }
}

void sleepy_response(int response, char* message) {
    std::lock_guard<std::mutex> lg(m_client_lock);
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

void send_message_sleepy(bool should_ping, const QString& message, const QJsonObject& embed) {
    std::lock_guard<std::mutex> lg(m_client_lock);
    sleepy_logger().log("send_message_sleepy()", "purple");
    if (m_sleepy_client != nullptr) {
        sleepy_logger().log("send_message_sleepy(): Sending...", "purple");
        std::string content = ((should_ping ? "<@" + GlobalSettings::instance().DISCORD.message.user_id + "> " : "") + message).toStdString();
        std::string json = QJsonDocument(embed).toJson().toStdString();
        m_sleepy_client->send(json, "", content, nullptr);
    }
}

void send_screenshot_sleepy(bool should_ping, const QString& message, QJsonObject& embed, std::shared_ptr<PendingFileSend> file){
    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_sleepy_client == nullptr) {
        return;
    }

    std::string content = ((should_ping ? "<@" + GlobalSettings::instance().DISCORD.message.user_id + "> " : "") + message).toStdString();
    std::string json = QJsonDocument(embed).toJson().toStdString();
    m_sleepy_client->send(json, "", content, std::move(file));
}

void send_log_sleepy(std::string logText) {
    //  Must be called inside lock.
//    std::lock_guard<std::mutex> lg(m_client_lock);
    if (m_sleepy_client != nullptr) {
        std::string instance = GlobalSettings::instance().DISCORD.message.instance_name.get().toStdString();
        if (!instance.empty()) {
            instance = " - [" + instance + "]";
        }
        else instance = " - [SleepyDiscord]";

        std::string msg = "> [" + current_time() + "]" + instance + ": " + logText;
        m_sleepy_client->send("", "", msg, nullptr);
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
    param_string += settings.integration.token.get().replace(" ", "").toStdString() + "|";
    param_string += settings.integration.command_prefix.get().toStdString() + "|";
    param_string += settings.integration.owner.get().replace(" ", "").toStdString() + "|";
    param_string += settings.integration.game_status.get().toStdString() + "|";
    param_string += settings.integration.hello_message.get().replace("@&", "").toStdString() + "|";
    param_string += PROGRAM_VERSION.toStdString() + "|";
    param_string += PROJECT_GITHUB_URL.toStdString();

    std::string sudo = settings.integration.sudo.get().replace(" ", "").toStdString();

//    std::string w_channels = settings.integration.channels_whitelist.get().replace(" ", "").toStdString();
    std::string w_channels;
    for (const QString& channel_id : settings.integration.channels.command_channels()){
        if (!w_channels.empty()){
            w_channels += ",";
        }
        w_channels += channel_id.toStdString();
    }

//    std::string l_channels = settings.integration.channels_log.get().replace(" ", "").toStdString();
    std::string l_channels;
    for (const QString& channel_id : settings.integration.channels.logging_channels()){
        if (!l_channels.empty()){
            l_channels += ",";
        }
        l_channels += channel_id.toStdString();
    }

//    std::string e_channels = settings.integration.channels_echo.get().replace(" ", "").toStdString();
    std::string e_channels;
    for (const QString& channel_id : settings.integration.channels.echo_channels()){
        if (!e_channels.empty()){
            e_channels += ",";
        }
        e_channels += channel_id.toStdString();
    }

    m_sleepy_client = std::unique_ptr<SleepyDiscordClient>(new SleepyDiscordClient());
    apply_settings(sleepy_response, sleepy_cmd_response, &w_channels[0], &e_channels[0], &l_channels[0], &sudo[0], &param_string[0], suffix);
    return true;
}

bool check_if_empty(const DiscordSettingsOption& settings) {
    if (!settings.integration.enabled()){
        return false;
    }
    if (settings.integration.token.get().isEmpty()) {
        return false;
    }
    else if (settings.integration.token.get().contains(",")) {
        sleepy_logger().log("\"Token\" must only contain one token. Stopping...", "red");
        return false;
    }
    else if (settings.integration.owner.get().contains(",")) {
        sleepy_logger().log("\"Owner\" must only contain one Discord ID (yours). Stopping...", "red");
        return false;
    }
    else if (settings.integration.command_prefix.get().isEmpty()) {
        sleepy_logger().log("Please enter a Discord command prefix. Stopping...", "red");
        return false;
    }
//    else if (settings.integration.channels_echo.get().isEmpty()) {
//        sleepy_logger().log(" Please enter at least one Discord channel for your bot to post notifications to. Stopping...", "red");
//        return false;
//    }
    return true;
}





}
}
}
#endif
