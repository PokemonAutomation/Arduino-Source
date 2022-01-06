/*  Serial Connection Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialSelectorWidget_H
#define PokemonAutomation_SerialSelectorWidget_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "SerialSelector.h"

namespace PokemonAutomation{


class SerialSelectorWidget : public QWidget{
    Q_OBJECT

public:
    SerialSelectorWidget(
        QWidget& parent,
        SerialSelector& value,
        Logger& logger
    );
    ~SerialSelectorWidget();

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
