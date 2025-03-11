/*  Multi-Switch System Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget implementation of the UI for MultiSwitchSystemSession.
 *
 *  On construction, this class attaches itself to the session it is constructed
 *  with and automatically detaches on destruction. Therefore, this class must
 *  not outlive the session it is constructed with. While not useful, it is also
 *  safe to construct multiple UI classes attached to the same session.
 *
 *  Modifications directly to the session object will automatically update this
 *  UI class. For example, if you use Discord to change the volume of the
 *  audio playback, it will move the slider as shown by this UI.
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H
#define PokemonAutomationn_NintendoSwitch_SwitchSystem4Widget_H

#include <QWidget>
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.h"

class QComboBox;

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemWidget;


class MultiSwitchSystemWidget final : public QWidget, private MultiSwitchSystemSession::Listener{
    Q_OBJECT

public:
    ~MultiSwitchSystemWidget();
    MultiSwitchSystemWidget(
        QWidget& parent,
        MultiSwitchSystemSession& session,
        uint64_t program_id
    );

    size_t switch_count() const{ return m_switches.size(); }
    SwitchSystemWidget& operator[](size_t index){ return *m_switches[index]; }

public:
    void update_ui(ProgramState state);

private:
    virtual void shutdown() override;
    virtual void startup(size_t switch_count) override;

    void redraw_videos(size_t count);

private:
    uint64_t m_program_id;
    MultiSwitchSystemSession& m_session;

    std::mutex m_lock;
    bool m_shutting_down = false;

    QComboBox* m_console_count_box;
    std::vector<SwitchSystemWidget*> m_switches;
    QWidget* m_videos;
//    std::map<size_t, SwitchSystem*> m_active_ports;
};


}
}
#endif
