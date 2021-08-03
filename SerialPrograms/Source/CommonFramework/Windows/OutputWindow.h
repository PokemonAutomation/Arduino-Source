/*  Text Window for Output Logging
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TextWindow_H
#define PokemonAutomation_TextWindow_H

#include <QFile>
#include <QTextEdit>
#include <QMainWindow>
#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{


class OutputWindow;

class TaggedLogger : public QObject, public Logger{
    Q_OBJECT

public:
    TaggedLogger(OutputWindow& window, QString tag);
    virtual void log(const char* msg, QColor color = QColor()) override;
    virtual void log(const std::string& msg, QColor color = QColor()) override;
    virtual void log(const QString& msg, QColor color = QColor()) override;

signals:
    void signal_log(QString msg, QColor color);

private:
    const QString m_tag;
};

class SerialLogger : public TaggedLogger, public PokemonAutomation::MessageLogger{
public:
    SerialLogger(OutputWindow& window, QString tag);
    using TaggedLogger::log;
    virtual void log(std::string msg) override;
};


class OutputWindow : public QMainWindow{
    Q_OBJECT

public:
    OutputWindow(QWidget *parent = nullptr);
    ~OutputWindow();

    void operator+=(TaggedLogger& logger);
    void operator-=(TaggedLogger& logger);

public slots:
    void log(QString msg, QColor color);

private:
    QColor m_default_color;
    QTextEdit* m_text;
    QMenuBar* m_menubar;
//    QStatusBar* m_statusbar;
    QFile m_log_file;
};



}
#endif
