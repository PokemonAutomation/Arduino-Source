/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerWidget_H
#define PokemonAutomation_Controllers_ControllerWidget_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "ControllerSession.h"

namespace PokemonAutomation{




class ControllerSelectorWidget : public QWidget, private ControllerSession::Listener{
public:
    ~ControllerSelectorWidget();
    ControllerSelectorWidget(QWidget& parent, ControllerSession& session);

public:
    virtual void controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor) override;
    virtual void status_text_changed(const std::string& text) override;
    virtual void options_locked(bool locked) override;

private:
    void refresh();

private:
    ControllerSession& m_session;

    QComboBox* m_devices_dropdown;
    QLabel* m_status_text;
    QPushButton* m_reset_button;

    std::vector<std::shared_ptr<const ControllerDescriptor>> m_device_list;
};


}
#endif
