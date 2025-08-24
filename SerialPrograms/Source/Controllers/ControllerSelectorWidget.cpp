/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include <QHBoxLayout>
#include "Common/Qt/NoWheelComboBox.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "Controllers/ControllerTypeStrings.h"
#include "ControllerSelectorWidget.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"

#include "SerialPABotBase/SerialPABotBase_SelectorWidget.h"
#include "NintendoSwitch/Controllers/SysbotBase/SysbotBase_SelectorWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




ControllerSelectorWidget::~ControllerSelectorWidget(){
    m_session.remove_listener(*this);
}
ControllerSelectorWidget::ControllerSelectorWidget(QWidget& parent, ControllerSession& session)
    : QWidget(&parent)
    , m_session(session)
{
    QHBoxLayout* serial_row = new QHBoxLayout(this);
    serial_row->setContentsMargins(0, 0, 0, 0);

    serial_row->addWidget(new QLabel("<b>Controller:</b>", this), 1);
    serial_row->addSpacing(5);

    m_dropdowns = new QHBoxLayout();
    serial_row->addLayout(m_dropdowns, 5);
    serial_row->addSpacing(5);

    interface_dropdown = new NoWheelComboBox(this);
    m_dropdowns->addWidget(interface_dropdown, 2);

    interface_dropdown->addItem(QString::fromStdString(CONTROLLER_INTERFACE_STRINGS.get_string(ControllerInterface::SerialPABotBase)));
    interface_dropdown->addItem(QString::fromStdString(CONTROLLER_INTERFACE_STRINGS.get_string(ControllerInterface::TcpSysbotBase)));
//    interface_dropdown->addItem(QString::fromStdString(CONTROLLER_INTERFACE_STRINGS.get_string(ControllerInterface::UsbSysbotBase)));

//    interface_dropdown->setHidden(true);

    auto current = session.descriptor();
    if (current == nullptr || current->interface_type == ControllerInterface::None){
        current.reset(new SerialPABotBase::SerialPABotBase_Descriptor());
        session.set_device(std::move(current));
    }
    interface_dropdown->setCurrentIndex((int)current->interface_type - 1);
    m_selector = current->make_selector_QtWidget(*this);
    m_dropdowns->addWidget(m_selector);


    m_dropdowns->addSpacing(5);
    m_controllers_dropdown = new NoWheelComboBox(this);
    m_dropdowns->addWidget(m_controllers_dropdown, 3);
    refresh_controllers(session.controller_type(), session.available_controllers());

    m_status_text = new QLabel(this);
    serial_row->addWidget(m_status_text, 3);
    serial_row->addSpacing(5);

    m_status_text->setText(QString::fromStdString(session.status_text()));

    m_reset_button = new QPushButton("Reset Ctrl.", this);
#if 1
    m_reset_button->setToolTip(
        "<b>Click:</b> Reset the controller.<br><br>"
        "<b>Shift+Click:</b> Reset and clear the controller of any state. "
        "For controllers save pairing state, this will unpair it with any hosts it may be connected to."
    );
#endif
    serial_row->addWidget(m_reset_button, 1);

    bool options_locked = session.options_locked();
    if (m_selector){
        m_selector->setEnabled(!options_locked);
    }
    m_reset_button->setEnabled(!options_locked);

    setFocusPolicy(Qt::StrongFocus);

    connect(
        interface_dropdown, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            index = std::max(index, 0);
//            index = std::min(index, (int)m_device_list.size() - 1);

            ControllerInterface incoming = (ControllerInterface)(index + 1);
            ControllerInterface existing = m_session.descriptor()->interface_type;
//            cout << "incoming = " << (int)incoming << endl;
//            cout << "existing = " << (int)existing << endl;
            if (incoming == existing){
                return;
            }

            refresh_selection(incoming);
        }
    );
    connect(
        m_controllers_dropdown, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            index = std::max(index, 0);
            ControllerType new_value = CONTROLLER_TYPE_STRINGS.get_enum(
                m_controllers_dropdown->itemText(index).toStdString(),
                ControllerType::None
            );
            if (new_value == m_session.controller_type()){
                return;
            }
            m_session.set_controller(new_value);
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            bool shift_held = QGuiApplication::keyboardModifiers() & Qt::ShiftModifier;
            m_session.reset(shift_held);
        }
    );

    session.add_listener(*this);
}




void ControllerSelectorWidget::refresh_selection(ControllerInterface interface_type){
//    cout << "refresh_selection(): "<< endl;

    if (interface_type == ControllerInterface::None){
        interface_type = ControllerInterface::SerialPABotBase;
    }
    interface_dropdown->setCurrentIndex((int)interface_type - 1);

    delete m_selector;
    m_selector = nullptr;

//    m_status_text->setText(QString::fromStdString(html_color_text("Not Connected", COLOR_RED)));

    switch (interface_type){
    case ControllerInterface::SerialPABotBase:
        m_selector = new SerialPABotBase::SerialPABotBase_SelectorWidget(*this, m_session.descriptor().get());
        m_dropdowns->insertWidget(1, m_selector);
        break;

    case ControllerInterface::TcpSysbotBase:
        m_selector = new SysbotBase::TcpSysbotBase_SelectorWidget(*this, m_session.descriptor().get());
        m_dropdowns->insertWidget(1, m_selector);
        break;

    default:;
    }


}

void ControllerSelectorWidget::refresh_controllers(
    ControllerType controller_type,
    const std::vector<ControllerType>& available_controllers
){
    if (m_controllers_dropdown == nullptr){
        return;
    }
//    cout << "refresh_controllers()" << endl;

    m_controllers_dropdown->clear();

    size_t index = 0;
    for (size_t c = 0; c < available_controllers.size(); c++){
        const std::string& name = CONTROLLER_TYPE_STRINGS.get_string(available_controllers[c]);
        m_controllers_dropdown->addItem(QString::fromStdString(name));
        if (controller_type == available_controllers[c]){
            index = c;
        }
    }
    m_controllers_dropdown->setCurrentIndex((int)index);
}



void ControllerSelectorWidget::descriptor_changed(
    const std::shared_ptr<const ControllerDescriptor>& descriptor
){
//    cout << "descriptor_changed()" << endl;
    QMetaObject::invokeMethod(this, [=, this]{
        refresh_selection(descriptor->interface_type);
        refresh_controllers(ControllerType::None, {});
    }, Qt::QueuedConnection);
}
void ControllerSelectorWidget::controller_changed(
    ControllerType controller_type,
    const std::vector<ControllerType>& available_controllers
){
//    cout << "ControllerSelectorWidget::controller_changed()" << endl;
    QMetaObject::invokeMethod(this, [=, this]{
        refresh_controllers(controller_type, available_controllers);
    }, Qt::QueuedConnection);
}
void ControllerSelectorWidget::post_status_text_changed(const std::string& text){
//    cout << "ControllerSelectorWidget::status_text_changed(): " << text << endl;
    QMetaObject::invokeMethod(this, [this, text]{
        m_status_text->setText(QString::fromStdString(text));
    });
}
void ControllerSelectorWidget::options_locked(bool locked){
    QMetaObject::invokeMethod(this, [this, locked]{
        m_selector->setEnabled(!locked);
        interface_dropdown->setEnabled(!locked);
        m_controllers_dropdown->setEnabled(!locked);
        m_reset_button->setEnabled(!locked);
    });
}


#if 0
void ControllerSelectorWidget::update_buttons(){
    if (m_shift_held){
        m_reset_button->setText("Clear Ctrl.");
    }else{
        m_reset_button->setText("Reset Ctrl.");
    }
}


void ControllerSelectorWidget::keyPressEvent(QKeyEvent* event){
//    cout << "ControllerSelectorWidget::keyPressEvent()" << endl;
    if (event->key() == Qt::Key_Shift){
        m_shift_held = true;
    }
    update_buttons();
//    QWidget::keyPressEvent(event);
}
void ControllerSelectorWidget::keyReleaseEvent(QKeyEvent* event){
//    cout << "ControllerSelectorWidget::keyReleaseEvent()" << endl;
    if (event->key() == Qt::Key_Shift){
        m_shift_held = false;
    }
    update_buttons();
//    QWidget::keyReleaseEvent(event);
}
void ControllerSelectorWidget::focusInEvent(QFocusEvent* event){
//    cout << "ControllerSelectorWidget::focusInEvent()" << endl;
    QWidget::focusInEvent(event);
}
void ControllerSelectorWidget::focusOutEvent(QFocusEvent* event){
//    cout << "ControllerSelectorWidget::focusOutEvent()" << endl;
    m_shift_held = false;
    update_buttons();
    QWidget::focusOutEvent(event);
}
#endif



}
