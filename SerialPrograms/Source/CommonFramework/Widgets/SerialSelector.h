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
#include "CommonFramework/Logging/DeviceLogger.h"
#include "CommonFramework/Tools/BotBaseHandle.h"

namespace PokemonAutomation{


class SerialSelectorUI;


class SerialSelector{
public:
    SerialSelector(
        QString label, std::string logger_tag,
        PABotBaseLevel minimum_pabotbase
    );
    SerialSelector(
        QString label, std::string logger_tag,
        PABotBaseLevel minimum_pabotbase,
        const QJsonValue& json
    );

    void load_json(const QJsonValue& json);
    QJsonValue to_json() const;

    const QSerialPortInfo* port() const;

    SerialSelectorUI* make_ui(QWidget& parent, Logger& logger);

private:
    friend class SerialSelectorUI;

    const QString m_label;
    const PABotBaseLevel m_minimum_pabotbase;
    const std::string m_logger_tag;

    QSerialPortInfo m_port;
};


class SerialSelectorUI : public QWidget{
    Q_OBJECT

public:
    SerialSelectorUI(
        QWidget& parent,
        SerialSelector& value,
        Logger& logger
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


