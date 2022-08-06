/*  Serial Port Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SerialPortWidget_H
#define PokemonAutomation_SerialPortWidget_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "SerialPortOption.h"
#include "SerialPortSession.h"

namespace PokemonAutomation{


class SerialPortWidget : public QWidget, public SerialPortSession::Listener{
    Q_OBJECT

public:
    SerialPortWidget(
        QWidget& parent,
        SerialPortSession& session,
        Logger& logger
    );
    ~SerialPortWidget();

    bool is_ready() const;
    BotBaseHandle& botbase();

    void set_options_enabled(bool enabled);

    void stop();
    void reset();

signals:
    void signal_on_ready(bool ready);

private:
    void refresh();
    virtual void on_ready(bool ready) override;

private:
    SerialPortSession& m_session;

    QComboBox* m_serial_box;
    QLabel* m_serial_program;
    QLabel* m_serial_uptime;
    QPushButton* m_reset_button;

    std::vector<QSerialPortInfo> m_ports;
};



}
#endif
