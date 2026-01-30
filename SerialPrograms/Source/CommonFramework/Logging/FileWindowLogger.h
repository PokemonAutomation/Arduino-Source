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


// A Qt window that displays log output from a FileWindowLogger.
// Uses Qt signals/slots for thread-safe updates from the logger's background thread.
class FileWindowLoggerWindow : public QMainWindow, public ConfigOption::Listener, public FileLogger::Listener{
    Q_OBJECT

public:
    FileWindowLoggerWindow(QWidget* parent = nullptr);
    virtual ~FileWindowLoggerWindow();

    // Called by FileWindowLogger to display a log message.
    
    void log(QString msg);

    // Callback function registered to the global logger.
    // The global logger's background thread call it to display a log to the window.
    // Thread-safe: emits a signal that is handled on the UI thread.
    void on_log(const std::string& msg, Color color) override;

    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void moveEvent(QMoveEvent* event) override;

signals:
    void signal_log(QString msg);

private:
    virtual void on_config_value_changed(void* object) override;

    static QString to_window_str(const std::string& msg, Color color);

    FileLogger& m_logger;
    QMenuBar* m_menubar;
    QTextEdit* m_text;
    bool m_pending_resize = false;
    bool m_pending_move = false;
};


}
#endif
