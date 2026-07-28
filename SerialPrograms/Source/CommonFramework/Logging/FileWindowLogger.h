/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Qt-dependent layer that adds GUI window support on top of the Qt-free FileLogger.
 *  The actual file I/O is handled by Common/Cpp/Logging/FileLogger.
 */

#ifndef PokemonAutomation_Logging_FileWindowLogger_H
#define PokemonAutomation_Logging_FileWindowLogger_H

#include <QTextEdit>
#include <QMainWindow>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Containers/CircularBuffer.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


// A Qt window that displays log output from a FileWindowLogger.
// Uses Qt signals/slots for thread-safe updates from the logger's background thread.
class FileWindowLoggerWindow final : public QMainWindow, public ConfigOption::Listener, public Logger{
    Q_OBJECT

public:
    //  Don't display more than this many log lines. Too many will lag the window.
    static constexpr size_t MAX_LINES = 1000;

    //  Throttling: If there are more than MAX_LOGS_PER_WINDOW logs with in a
    //  WINDOW_SIZE window, delay the window updates.
    static constexpr size_t MAX_LOGS_PER_WINDOW = 10;
    static constexpr Milliseconds WINDOW_SIZE = Milliseconds(50);

public:
    FileWindowLoggerWindow(QWidget* parent);
    virtual ~FileWindowLoggerWindow();

    // Called by FileWindowLogger to display a log message.

    // Callback function registered to the global logger.
    // The global logger's background thread call it to display a log to the window.
    // Thread-safe: emits a signal that is handled on the UI thread.
    void log(const std::string& msg, Color color = Color()) override;

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void moveEvent(QMoveEvent* event) override;

signals:
    void signal_log();

private:
    virtual void on_config_value_changed(void* object) override;
    void update();
    QString to_window_str(const std::string& msg, Color color);

    QMenuBar* m_menubar;
    QTextEdit* m_text;
    bool m_pending_resize = false;
    bool m_pending_move = false;

    SpinLock m_lock;
    bool m_pending_update = false;
    CircularBuffer<WallClock> m_timestamps;

    CircularBuffer<QString> m_pending;
};


}
#endif
