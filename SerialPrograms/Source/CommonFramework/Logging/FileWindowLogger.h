/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "Logger.h"

namespace PokemonAutomation{

class FileWindowLoggerWindow;


class FileWindowLogger : public Logger{
public:
    ~FileWindowLogger();
    FileWindowLogger(const std::string& path);

    void operator+=(FileWindowLoggerWindow& widget);
    void operator-=(FileWindowLoggerWindow& widget);

    virtual void log(const std::string& msg, Color color = Color()) override;
    virtual void log(std::string&& msg, Color color = Color()) override;

private:
    static std::string normalize_newlines(const std::string& msg);
    static std::string to_file_str(const std::string& msg);
    static QString to_window_str(const std::string& msg, Color color);

    void internal_log(const std::string& msg, Color color);
    void thread_loop();

private:
    QFile m_file;
    size_t m_max_queue_size;
    std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_stopping;
    std::deque<std::pair<std::string, Color>> m_queue;
    std::set<FileWindowLoggerWindow*> m_windows;
    std::thread m_thread;
};


class FileWindowLoggerWindow : public QMainWindow{
    Q_OBJECT

public:
    FileWindowLoggerWindow(FileWindowLogger& logger, QWidget* parent = nullptr);
    virtual ~FileWindowLoggerWindow();

    void log(QString msg);

signals:
    void signal_log(QString msg);

private:
    FileWindowLogger& m_logger;
    QMenuBar* m_menubar;
    QTextEdit* m_text;
};


}
#endif
