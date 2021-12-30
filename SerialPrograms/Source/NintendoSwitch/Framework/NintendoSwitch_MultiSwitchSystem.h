/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_SwitchSystem4_H
#define PokemonAutomationn_NintendoSwitch_SwitchSystem4_H

#include <memory>
#include <vector>
#include "NintendoSwitch_SwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemWidget;

class MultiSwitchSystemFactory : public SwitchSetupFactory{
public:
    static const size_t MAX_SWITCHES = 4;

public:
    MultiSwitchSystemFactory(
        PABotBaseLevel min_pabotbase, FeedbackType feedback,
        size_t min_switches,
        size_t max_switches,
        size_t switches
    );
    MultiSwitchSystemFactory(
        PABotBaseLevel min_pabotbase, FeedbackType feedback,
        size_t min_switches,
        size_t max_switches,
        const QJsonValue& json
    );
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    size_t count() const{ return m_active_switches; }
    void resize(size_t count);

    const QSerialPortInfo* port(size_t index) const;
//    const QCameraInfo* camera(size_t index) const;

    SwitchSetupWidget* make_ui(QWidget& parent, Logger& logger, uint64_t program_id) override;

private:
    friend class MultiSwitchSystemWidget;
    const size_t m_min_switches;
    const size_t m_max_switches;
    size_t m_active_switches;
    std::vector<std::unique_ptr<SwitchSystemFactory>> m_switches;
};




}
}
#endif
