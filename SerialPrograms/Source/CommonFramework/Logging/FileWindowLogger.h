/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Logging_FileWindowLogger_H
#define PokemonAutomation_Logging_FileWindowLogger_H

#include <deque>
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QFile>
#include <QTextEdit>
#include <QMainWindow>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Options/ConfigOption.h"
//#include "Common/Cpp/LifetimeSanitizer.h"

namespace PokemonAutomation{

class FileWindowLoggerWindow;


class LastLogTracker{
public:
    LastLogTracker(size_t max_lines = 10000)
        : m_max_lines(max_lines)
    {}
    void operator+=(std::string line);
    std::vector<std::string> snapshot() const;

private:
    size_t m_max_lines;
    std::deque<std::string> m_lines;
};


class FileWindowLogger : public Logger{
public:
    ~FileWindowLogger();
    FileWindowLogger(const std::string& path);

    void operator+=(FileWindowLoggerWindow& widget);
    void operator-=(FileWindowLoggerWindow& widget);

    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string&& msg, Color color = Color()) override;
    virtual std::vector<std::string> get_last() const override;

private:
    static std::string normalize_newlines(const std::string& msg);
    static std::string to_file_str(const std::string& msg);
    static QString to_window_str(const std::string& msg, Color color);

    void internal_log(const std::string& msg, Color color);
    void thread_loop();

private:
    QFile m_file;
    size_t m_max_queue_size;
    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    LastLogTracker m_last_log_tracker;
    bool m_stopping;
    std::deque<std::pair<std::string, Color>> m_queue;
    std::set<FileWindowLoggerWindow*> m_windows;
    std::thread m_thread;

//    LifetimeSanitizer m_sanitizer;
};


class FileWindowLoggerWindow : public QMainWindow, public ConfigOption::Listener{
    Q_OBJECT

public:
    FileWindowLoggerWindow(FileWindowLogger& logger, QWidget* parent = nullptr);
    virtual ~FileWindowLoggerWindow();

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
