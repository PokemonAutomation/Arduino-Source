/*  Switch Setup
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchSetup_H
#define PokemonAutomation_SwitchSetup_H

#include <QJsonValue>
#include <QWidget>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/BotBaseHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchSetup;

class SwitchSetupFactory{
public:
    SwitchSetupFactory(
        PABotBaseLevel min_pabotbase, FeedbackType feedback
    )
        : m_min_pabotbase(min_pabotbase)
        , m_feedback(feedback)
    {}
    virtual ~SwitchSetupFactory() = default;

    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    virtual SwitchSetup* make_ui(QWidget& parent, Logger& logger, uint64_t program_id) = 0;

protected:
    const PABotBaseLevel m_min_pabotbase;
    const FeedbackType m_feedback;
};


class SwitchSetup : public QWidget{
    Q_OBJECT

public:
    virtual ~SwitchSetup() = default;
    SwitchSetup(const SwitchSetup&) = delete;
    void operator=(const SwitchSetup&) = delete;

public:
    SwitchSetup(
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
