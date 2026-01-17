/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Qt-dependent layer that adds GUI window support on top of the Qt-free FileLogger.
 *  The actual file I/O is handled by Common/Cpp/Logging/FileLogger.
 */

#ifndef PokemonAutomation_Logging_FileWindowLogger_H
#define PokemonAutomation_Logging_FileWindowLogger_H

#include <set>
#include <mutex>
#include <QTextEdit>
#include <QMainWindow>
#include "Common/Cpp/Logging/FileLogger.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{

class FileWindowLoggerWindow;


// A logger that writes to a file (via FileLogger) and can also display
// log messages in Qt GUI windows (FileWindowLoggerWindow).
//
// This class acts as a thin Qt wrapper around the Qt-free FileLogger,
// adding the ability to manage multiple Qt windows that display log output.
class FileWindowLogger : public Logger, private FileLogger::Listener{
public:
    ~FileWindowLogger();

    // Construct a FileWindowLogger that writes to the given file path.
    // The max_queue_size parameter controls how many log messages can be
    // queued before the log() call blocks.
    FileWindowLogger(const std::string& path, size_t max_queue_size);

    // Add/remove Qt windows that will display log messages.
    void operator+=(FileWindowLoggerWindow& widget);
    void operator-=(FileWindowLoggerWindow& widget);

    // Logger interface implementation - forwards to FileLogger.
    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string&& msg, Color color = Color()) override;
    virtual std::vector<std::string> get_last() const override;

private:
    // FileLogger::Listener implementation - called when a message is logged.
    // Formats the message for Qt display and sends to all registered windows.
    virtual void on_log(const std::string& msg, Color color) override;

    // Convert a log message to HTML for display in QTextEdit.
    static QString to_window_str(const std::string& msg, Color color);

private:
    FileLogger m_file_logger;

    std::mutex m_window_lock;
    std::set<FileWindowLoggerWindow*> m_windows;
};


// A Qt window that displays log output from a FileWindowLogger.
// Uses Qt signals/slots for thread-safe updates from the logger's background thread.
class FileWindowLoggerWindow : public QMainWindow, public ConfigOption::Listener{
    Q_OBJECT

public:
    FileWindowLoggerWindow(FileWindowLogger& logger, QWidget* parent = nullptr);
    virtual ~FileWindowLoggerWindow();

    // Called by FileWindowLogger to display a log message.
    // Thread-safe: emits a signal that is handled on the UI thread.
    void log(QString msg);

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void moveEvent(QMoveEvent* event) override;

signals:
    void signal_log(QString msg);

private:
    virtual void on_config_value_changed(void* object) override;

    FileWindowLogger& m_logger;
    QMenuBar* m_menubar;
    QTextEdit* m_text;
    bool m_pending_resize = false;
    bool m_pending_move = false;
};


}
#endif
