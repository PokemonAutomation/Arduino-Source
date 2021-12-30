/*  Switch Setup
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSetupWidget_H
#define PokemonAutomation_NintendoSwitch_SwitchSetupWidget_H

#include <QWidget>
#include "NintendoSwitch_SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSetupWidget : public QWidget{
    Q_OBJECT

public:
    virtual ~SwitchSetupWidget() = default;
    SwitchSetupWidget(const SwitchSetupWidget&) = delete;
    void operator=(const SwitchSetupWidget&) = delete;

public:
    SwitchSetupWidget(
        QWidget& parent,
        SwitchSetupFactory& factory
    )
        : QWidget(&parent)
        , m_factory(factory)
    {}

    virtual bool serial_ok() const = 0;
    virtual void wait_for_all_requests() = 0;
    virtual void stop_serial() = 0;
    virtual void reset_serial() = 0;

    virtual void update_ui(ProgramState state) = 0;

signals:
    void on_setup_changed();
    void on_program_state_changed();

protected:
    SwitchSetupFactory& m_factory;
};



}
}
#endif
