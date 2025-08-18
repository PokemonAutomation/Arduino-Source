/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerWidget_H
#define PokemonAutomation_Controllers_ControllerWidget_H

#include <QHBoxLayout>
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

    ControllerSession& session(){
        return m_session;
    }

public:
    virtual void descriptor_changed(
        const std::shared_ptr<const ControllerDescriptor>& descriptor
    ) override;
    virtual void controller_changed(
        ControllerType controller_type,
        const std::vector<ControllerType>& available_controllers
    ) override;
    virtual void post_status_text_changed(const std::string& text) override;
    virtual void options_locked(bool locked) override;

private:
    void refresh_selection(ControllerInterface interface_type);
    void refresh_controllers(
        ControllerType controller_type,
        const std::vector<ControllerType>& available_controllers
    );

private:
    void update_buttons();

#if 0
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
#endif

private:
    ControllerSession& m_session;

    QHBoxLayout* m_dropdowns;
    QWidget* m_selector = nullptr;
    QComboBox* interface_dropdown = nullptr;
    QComboBox* m_controllers_dropdown = nullptr;
    QLabel* m_status_text = nullptr;
    QPushButton* m_reset_button = nullptr;

//    bool m_shift_held = false;

    std::vector<std::shared_ptr<const ControllerDescriptor>> m_device_list;
};


}
#endif
