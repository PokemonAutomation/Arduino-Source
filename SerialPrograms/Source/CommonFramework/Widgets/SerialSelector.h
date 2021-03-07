/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialSelector_H
#define PokemonAutomation_SerialSelector_H

#include <QSerialPortInfo>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/PokemonSwSh/PokemonProgramIDs.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Windows/OutputWindow.h"

namespace PokemonAutomation{


class SerialSelectorUI;


class SerialSelector{
public:
    SerialSelector(
        QString label, QString logger_tag,
        PABotBaseLevel minimum_pabotbase
    );
    SerialSelector(
        QString label, QString logger_tag,
        PABotBaseLevel minimum_pabotbase,
        const QJsonValue& json
    );

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    const QSerialPortInfo* port() const;

    SerialSelectorUI* make_ui(QWidget& parent, OutputWindow& log_window);

private:
    friend class SerialSelectorUI;

    const QString m_label;
    const PABotBaseLevel m_minimum_pabotbase;
    const QString m_logger_tag;

    QSerialPortInfo m_port;
};


class SerialSelectorUI : public QWidget{
    Q_OBJECT

public:
    SerialSelectorUI(
        QWidget& parent,
        SerialSelector& value,
        OutputWindow& log_window
    );
    ~SerialSelectorUI();

    bool is_ready() const;
    BotBaseHandle& botbase();

    void set_options_enabled(bool enabled);

    void stop();
    void reset();

signals:
    void on_ready(bool ready);

private:
    void refresh();

private:
    SerialSelector& m_value;
    SerialLogger m_logger;

    QComboBox* m_serial_box;
    QLabel* m_serial_program;
    QLabel* m_serial_uptime;
    QPushButton* m_reset_button;

    QList<QSerialPortInfo> m_ports;
    BotBaseHandle m_connection;
};



}
#endif


