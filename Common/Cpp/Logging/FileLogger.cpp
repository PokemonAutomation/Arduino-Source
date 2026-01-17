/*  File Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <filesystem>
#include <iostream>
#include "Common/Cpp/PrettyPrint.h"
#include "FileLogger.h"

namespace PokemonAutomation{


FileLogger::~FileLogger(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
        m_cv.notify_all();
    }
    m_thread.join();
}

FileLogger::FileLogger(FileLoggerConfig config)
    : m_config(std::move(config))
    , m_last_log_tracker(m_config.last_log_max_lines)
    , m_stopping(false)
{
    bool exists = std::filesystem::exists(m_config.file_path);
    m_file.open(m_config.file_path, std::ios::out | std::ios::app | std::ios::binary);

    if (!exists && m_file.is_open()){
        // Write UTF-8 BOM to new files
        const char bom[] = "\xef\xbb\xbf";
        m_file.write(bom, 3);
        std::cout << "Write log to new file " << m_config.file_path << std::endl;
    }else{
        std::cout << "Write log to existing file " << m_config.file_path << std::endl;
    }

    m_thread = Thread([this]{
        thread_loop();
    });
}

void FileLogger::add_listener(Listener& listener){
    m_listeners.add(listener);
}

void FileLogger::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}

void FileLogger::log(const std::string& msg, Color color){
    std::unique_lock<std::mutex> lg(m_lock);
    m_last_log_tracker += msg;
    m_cv.wait(lg, [this]{ return m_queue.size() < m_config.max_queue_size; });
    m_queue.emplace_back(msg, color);
    m_cv.notify_all();
}

void FileLogger::log(std::string&& msg, Color color){
    std::unique_lock<std::mutex> lg(m_lock);
    m_last_log_tracker += msg;
    m_cv.wait(lg, [this]{ return m_queue.size() < m_config.max_queue_size; });
    m_queue.emplace_back(std::move(msg), color);
    m_cv.notify_all();
}

std::vector<std::string> FileLogger::get_last() const{
    std::unique_lock<std::mutex> lg(m_lock);
    return m_last_log_tracker.snapshot();
}

std::string FileLogger::normalize_newlines(const std::string& msg){
    std::string str;
    size_t index = 0;

    while (true){
        auto pos = msg.find("\r\n", index);
        if (pos == std::string::npos){
            str += msg.substr(index, pos);
            break;
        }else{
            str += msg.substr(index, pos - index);
            str += "\n";
            index = pos + 2;
        }
    }

    if (!str.empty() && str.back() == '\n'){
        str.pop_back();
    }

    return str;
}

std::string FileLogger::to_file_str(const std::string& msg){
    // Replace all newlines with \r\n for the log file (Windows compatibility).
    std::string str;
    for (char ch : msg){
        if (ch == '\n'){
            str += "\r\n";
            continue;
        }
        str += ch;
    }
    str += "\r\n";
    return str;
}

void FileLogger::internal_log(const std::string& msg, Color color){
    std::string line = normalize_newlines(msg);

    // Notify all listeners (e.g., UI windows)
    m_listeners.run_method(&Listener::on_log, line, color);

    // Write to file
    if (m_file.is_open()){
        std::string file_str = to_file_str(line);
        m_file.write(file_str.c_str(), file_str.size());
        m_file.flush();
    }
}

void FileLogger::thread_loop(){
    std::unique_lock<std::mutex> lg(m_lock);
    while (true){
        m_cv.wait(lg, [&]{
            return m_stopping || !m_queue.empty();
        });
        if (m_stopping){
            break;
        }
        auto& item = m_queue.front();
        std::string msg = std::move(item.first);
        Color color = item.second;
        m_queue.pop_front();

        lg.unlock();
        rotate_log_file();
        internal_log(msg, color);
        lg.lock();

        if (m_queue.size() <= m_config.max_queue_size / 2){
            m_cv.notify_all();
        }
    }
}

void FileLogger::rotate_log_file(){
    if (!m_file.is_open()){
        return;
    }

    // Check file size
    std::error_code ec;
    auto file_size = std::filesystem::file_size(m_config.file_path, ec);
    if (ec || file_size < m_config.max_file_size_bytes){
        return;
    }

    // Close current file
    m_file.close();

    // Generate backup filename: <basename>-<timestamp>.log
    std::filesystem::path original_path(m_config.file_path);
    std::string stem = original_path.stem().string();
    std::string extension = original_path.extension().string();
    std::filesystem::path parent = original_path.parent_path();

    std::string backup_filename = stem + "-" + now_to_filestring() + extension;
    std::filesystem::path backup_path = parent / backup_filename;

    // Rename current file to backup
    std::filesystem::rename(m_config.file_path, backup_path, ec);
    if (ec){
        std::cerr << "Failed to rename log file: " << ec.message() << std::endl;
    }

    // Re-open the file (creates a new, empty file)
    bool exists = std::filesystem::exists(m_config.file_path);
    m_file.open(m_config.file_path, std::ios::out | std::ios::app | std::ios::binary);
    if (!exists && m_file.is_open()){
        const char bom[] = "\xef\xbb\xbf";
        m_file.write(bom, 3);
    }
}


}
