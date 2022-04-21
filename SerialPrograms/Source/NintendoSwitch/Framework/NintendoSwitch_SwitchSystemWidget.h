/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H

#include "Integrations/ProgramTracker.h"
#include "NintendoSwitch_SwitchSetupWidget.h"
#include "NintendoSwitch_SwitchSystem.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class AudioDisplayWidget;

namespace NintendoSwitch{

class CommandRow;

class SwitchSystemWidget : public SwitchSetupWidget, public ConsoleSystem{
    Q_OBJECT

public:
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemFactory& factory,
        LoggerQt& raw_logger,
        uint64_t program_id
    );
    virtual ~SwitchSystemWidget();

    ProgramState last_known_state() const;

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

public:
    LoggerQt& logger();
    BotBase* botbase();
    VideoFeed& camera();
    VideoOverlay& overlay();
    AudioFeed& audio();
    virtual void update_ui(ProgramState state) override;

    virtual VideoFeed& video() override;
    virtual BotBaseHandle& sender() override;

private:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    uint64_t m_instance_id = 0;
    SwitchSystemFactory& m_factory;
    TaggedLogger m_logger;

    CollapsibleGroupBox* m_group_box;

    SerialSelectorWidget* m_serial;
    CommandRow* m_command;
    CameraSelectorWidget* m_camera;
    AudioSelectorWidget* m_audio;
    
    VideoDisplayWidget* m_video_display;
    AudioDisplayWidget* m_audio_display;
};




}
}
#endif
