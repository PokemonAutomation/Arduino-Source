/*  Text Window for Output Logging
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_TextWindow_H
#define PokemonAutomation_TextWindow_H

#include <QFile>
#include <QTextEdit>
#include <QMainWindow>
#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{


class OutputWindow;

class TaggedLoggerOld : public QObject, public Logger{
    Q_OBJECT

public:
    TaggedLoggerOld(OutputWindow& window, QString tag);
    virtual void log(const char* msg, Color color = QColor()) override;
    virtual void log(const std::string& msg, Color color = QColor()) override;
    virtual void log(const QString& msg, Color color = QColor()) override;

signals:
    void signal_log(QString msg, Color color);

private:
    const QString m_tag;
};

class SerialLoggerOld : public TaggedLoggerOld, public PokemonAutomation::MessageLogger{
public:
    SerialLoggerOld(OutputWindow& window, QString tag);
    using TaggedLoggerOld::log;
    virtual void log(std::string msg) override;
};


class OutputWindow : public QMainWindow{
    Q_OBJECT

public:
    OutputWindow(QWidget *parent = nullptr);
    ~OutputWindow();

    void operator+=(TaggedLoggerOld& logger);
    void operator-=(TaggedLoggerOld& logger);

public slots:
    void log(QString msg, Color color);

private:
    QColor m_default_color;
    QTextEdit* m_text;
    QMenuBar* m_menubar;
//    QStatusBar* m_statusbar;
    QFile m_log_file;
};



}
#endif
