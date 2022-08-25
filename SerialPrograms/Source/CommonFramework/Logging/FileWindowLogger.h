/*  File and Window Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Logging_FileWindowLogger_H
#define PokemonAutomation_Logging_FileWindowLogger_H

#include <set>
#include <mutex>
#include <QFile>
#include <QTextEdit>
#include <QMainWindow>
#include "Logger.h"

namespace PokemonAutomation{

class FileWindowLoggerWindow;


class FileWindowLogger : public Logger{
public:
    FileWindowLogger(const std::string& path);

    void operator+=(FileWindowLoggerWindow& widget);
    void operator-=(FileWindowLoggerWindow& widget);

    virtual void log(const char* msg, Color color = Color()) override;
    virtual void log(const std::string& msg, Color color = Color()) override;

private:
    static std::string normalize_newlines(const std::string& msg);
    static std::string to_file_str(const std::string& msg);
    static QString to_window_str(const std::string& msg, Color color);

private:
    QFile m_file;
    std::mutex m_lock;
    std::set<FileWindowLoggerWindow*> m_windows;
};


class FileWindowLoggerWindow : public QMainWindow{
    Q_OBJECT

public:
    FileWindowLoggerWindow(FileWindowLogger& logger, QWidget* parent = nullptr);
    virtual ~FileWindowLoggerWindow();

    void log(const QString& msg);

signals:
    void signal_log(const QString& msg);

private:
    FileWindowLogger& m_logger;
    QMenuBar* m_menubar;
    QTextEdit* m_text;
};


}
#endif
