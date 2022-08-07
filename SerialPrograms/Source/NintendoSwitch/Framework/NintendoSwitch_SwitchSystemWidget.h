/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemWidget_H

#include "CommonFramework/ControllerDevices/SerialPortSession.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "Integrations/ProgramTracker.h"
#include "NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch_SwitchSetupWidget.h"
#include "NintendoSwitch_SwitchSystem.h"

namespace PokemonAutomation{
    class CollapsibleGroupBox;
    class AudioFeed;
    class CameraSelectorWidget;
    class VideoDisplayWidget;
    class AudioDisplayWidget;
    class VideoOverlay;

namespace NintendoSwitch{

class CommandRow;

class SwitchSystemWidget : public SwitchSetupWidget{
public:
    virtual ~SwitchSystemWidget();
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemFactory& factory,
        Logger& raw_logger,
        uint64_t program_id
    );
    SwitchSystemWidget(
        QWidget& parent,
        SwitchSystemFactory& factory,
        SwitchSystemSession& session,
        uint64_t program_id
    );

    ProgramState last_known_state() const;

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

public:
    Logger& logger();
    BotBase* botbase();
    VideoFeed& camera();
    VideoOverlay& overlay();
    AudioFeed& audio();
    virtual void update_ui(ProgramState state) override;

private:
    void init();

    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    uint64_t m_instance_id = 0;
    SwitchSystemFactory& m_factory;

    std::unique_ptr<SwitchSystemSession> m_session_owner;
    SwitchSystemSession& m_session;

    CollapsibleGroupBox* m_group_box;

    SerialPortWidget* m_serial_widget = nullptr;

    VideoDisplayWidget* m_video_display;
    AudioDisplayWidget* m_audio_display;

    CommandRow* m_command;

    CameraSelectorWidget* m_camera_widget;
    AudioSelectorWidget* m_audio_widget;
};




}
}
#endif
