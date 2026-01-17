/*  File Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  A Qt-free file logger that writes log messages to a file with optional
 *  log rotation. Supports listeners for UI integration (e.g., displaying
 *  logs in a Qt window without coupling the core logger to Qt).
 */

#ifndef PokemonAutomation_Logging_FileLogger_H
#define PokemonAutomation_Logging_FileLogger_H

#include <deque>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include "AbstractLogger.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/Cpp/ListenerSet.h"
#include "LastLogTracker.h"

namespace PokemonAutomation{


// Configuration for the FileLogger.
struct FileLoggerConfig{
    std::string file_path;                          // Path to the log file, assuming UTF-8
    size_t max_queue_size = 10000;                  // Max pending log entries before blocking
    size_t max_file_size_bytes = 50 * 1024 * 1024;  // Max file size before rotation (50MB default)
    size_t last_log_max_lines = 10000;              // Max lines to keep in memory for get_last()
};


// A Qt-free file logger that:
// 1. Writes log messages to a file asynchronously via a background thread
// 2. Supports log rotation when the file exceeds a configured size
// 3. Notifies registered listeners when a log message is written (for UI integration)
// 4. Keeps track of recent log lines for retrieval via get_last()
//
// The Listener interface allows Qt GUI components to receive log messages
// without the core logger depending on Qt.
class FileLogger : public Logger{
public:
    // Listener interface for receiving log messages.
    // Implement this to integrate with UI components (e.g., QTextEdit).
    struct Listener{
        // Called when a log message is written. This is called from the
        // logger's background thread, so implementations must handle
        // thread safety (e.g., using Qt signals/slots for cross-thread updates).
        //
        // Parameters:
        //   msg: The log message with newlines normalized (no \r\n, just \n)
        //   color: Optional color for the message
        virtual void on_log(const std::string& msg, Color color) = 0;
    };

public:
    ~FileLogger();

    // Construct a FileLogger with the given configuration.
    // The log file is created if it doesn't exist, or appended to if it does.
    // A UTF-8 BOM is written to new files.
    FileLogger(FileLoggerConfig config);

    // Add a listener to receive log messages. Thread-safe.
    void add_listener(Listener& listener);

    // Remove a listener. Thread-safe.
    void remove_listener(Listener& listener);

    // Logger interface implementation
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string&& msg, Color color = Color()) override;
    virtual std::vector<std::string> get_last() const override;

private:
    // Normalize newlines: convert \r\n to \n, remove trailing newline.
    static std::string normalize_newlines(const std::string& msg);

    // Convert message to file format (with \r\n line endings for Windows compatibility).
    static std::string to_file_str(const std::string& msg);

    // Internal log processing (called from background thread).
    void internal_log(const std::string& msg, Color color);

    // Background thread loop that processes the log queue.
    void thread_loop();

    // Rotate the log file if it exceeds the configured max size.
    // Renames the current file to "<basename>-<timestamp>.log" and creates a new file.
    void rotate_log_file();

private:
    FileLoggerConfig m_config;
    std::ofstream m_file;
    std::string m_current_file_size_approx;  // Approximate file size for rotation check

    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    LastLogTracker m_last_log_tracker;
    bool m_stopping;
    std::deque<std::pair<std::string, Color>> m_queue;

    ListenerSet<Listener> m_listeners;
    Thread m_thread;
};


}
#endif
