/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchSystem_H
#define PokemonAutomation_SwitchSystem_H

#include <QJsonValue>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "CommonFramework/Widgets/SerialSelector.h"
#include "CommonFramework/Widgets/CameraSelector.h"
#include "SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class CommandRow;
class SwitchSystem;

class SwitchSystemFactory : public SwitchSetupFactory{
    static const QString JSON_SERIAL;
    static const QString JSON_CAMERA;
    static const QString JSON_CAMERA_INDEX;
    static const QString JSON_CAMERA_RESOLUTION;

public:
    SwitchSystemFactory(
        QString label, QString logger_tag,
        PABotBaseLevel min_pabotbase, FeedbackType feedback
    );
    SwitchSystemFactory(
        QString label, QString logger_tag,
        PABotBaseLevel min_pabotbase, FeedbackType feedback,
        const QJsonValue& json
    );
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    const QSerialPortInfo* port() const;

    SwitchSetup* make_ui(QWidget& parent, OutputWindow& log_window) override;

private:
    friend class SwitchSystem;

    QString m_label;
    QString m_logger_tag;

    SerialSelector m_serial;
    CameraSelector m_camera;
};


class SwitchSystem : public SwitchSetup{
    Q_OBJECT

public:
    SwitchSystem(
        QWidget& parent,
        SwitchSystemFactory& factory,
        OutputWindow& log_window
    );
    virtual ~SwitchSystem();

    ProgramState last_known_state() const;

    virtual bool serial_ok() const override;
    virtual void wait_for_all_requests() override;
    virtual void stop_serial() override;
    virtual void reset_serial() override;

    template <uint8_t SendType, typename Parameters>
    void send_request(Parameters& params){
        if (last_known_state() == ProgramState::STOPPED){
            m_serial->botbase().async_try_send_request<SendType>(params);
        }else{
            m_logger.log("SwitchSystem::send_request() - Request dropped due to incorrect program state.", Qt::red);
        }
    }

public:
    BotBase* botbase();
    VideoFeed& camera();
    virtual void update_ui(ProgramState state) override;

private:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    SwitchSystemFactory& m_factory;
    SerialLogger m_logger;

    SerialSelectorUI* m_serial;
    CommandRow* m_command;
    CameraSelectorUI* m_camera;
};



}
}
#endif
