/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchSystem4_H
#define PokemonAutomation_SwitchSystem4_H

#include <vector>
#include <map>
#include "SwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystem;

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

    SwitchSetup* make_ui(QWidget& parent, OutputWindow& log_window) override;

private:
    friend class MultiSwitchSystem;
    const size_t m_min_switches;
    const size_t m_max_switches;
    size_t m_active_switches;
    std::vector<std::unique_ptr<SwitchSystemFactory>> m_switches;
};


class MultiSwitchSystem : public SwitchSetup{
    Q_OBJECT

public:
    MultiSwitchSystem(
        QWidget& parent,
        MultiSwitchSystemFactory& factory,
        OutputWindow& log_window
    );
    void redraw_videos(size_t count);

    size_t switch_count() const{ return m_switches.size(); }
    SwitchSystem& operator[](size_t index){ return *m_switches[index]; }

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

public:
    virtual void update_ui(ProgramState state) override;

private:
//    void change_serial(size_t old_index, size_t new_index, SwitchSystem& system);

private:
    MultiSwitchSystemFactory& m_factory;
    OutputWindow& m_log_window;
    std::vector<SwitchSystem*> m_switches;
    QWidget* m_videos;
//    std::map<size_t, SwitchSystem*> m_active_ports;
};



}
}
#endif
