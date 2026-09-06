/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QMessageBox>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/Panels/ConsoleSettingsStretch.h"
#include "Controllers/ControllerTypeStrings.h"
#include "ControllerSelectorWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<ControllerSelectorWidget>;





ControllerSelectorWidget::~ControllerSelectorWidget(){
    m_session.remove_listener(*this);
}
ControllerSelectorWidget::ControllerSelectorWidget(
    QWidget& parent,
    ControllerSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
//    cout << "ControllerSelectorWidget()" << endl;

    QHBoxLayout* layoutL = new QHBoxLayout(this);
    layoutL->setContentsMargins(0, 0, 0, 0);

    if (!session.index().has_value()){
        layoutL->addWidget(new QLabel("<b>Controller:</b>", this), CONSOLE_SETTINGS_STRETCH_L0_LABEL);
    }else{
        QHBoxLayout* layoutL0 = new QHBoxLayout();
        layoutL->addLayout(layoutL0, CONSOLE_SETTINGS_STRETCH_L0_LABEL);
        layoutL0->setContentsMargins(0, 0, 0, 0);

        layoutL0->addWidget(
            new QLabel(
                QString::fromStdString("<b>Controller " + std::to_string(session.index().value()) + ":</b>"),
                this
            )
        );

        QCheckBox* check_box = new QCheckBox(this);
        layoutL0->addWidget(check_box, 1, Qt::AlignRight);
        if (session.input_enabled()){
            check_box->setCheckState(Qt::Checked);
        }else{
            check_box->setCheckState(Qt::Unchecked);
        }
        connect(
            check_box, &QCheckBox::checkStateChanged,
            this, [this, check_box](int){
                m_session.set_input_enabled(check_box->isChecked());
            }
        );
    }

    QHBoxLayout* layoutR = new QHBoxLayout();
    layoutL->addLayout(layoutR, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
    layoutR->setContentsMargins(0, 0, 0, 0);

    m_dropdowns = new QHBoxLayout();
    layoutR->addLayout(m_dropdowns, CONSOLE_SETTINGS_STRETCH_L1_BODY);
    layoutR->addSpacing(5);

    m_interface_dropdown = new NoWheelCompactComboBox(this);
    m_dropdowns->addWidget(m_interface_dropdown);


    //  Add all the supported interfaces.
    {
        m_interface_list.emplace_back(ControllerInterface::SerialPABotBase2);
        m_interface_list.emplace_back(ControllerInterface::TcpSysbotBase);
//        m_interface_list.emplace_back(ControllerInterface::UsbSysbotBase);
    }


    for (ControllerInterface item : m_interface_list){
        m_interface_dropdown->addItem(QString::fromStdString(CONTROLLER_INTERFACE_STRINGS.get_string(item)));
    }

//    m_interface_dropdown->setHidden(true);

    refresh_selection();


//    m_dropdowns->addSpacing(5);
    m_controllers_dropdown = new NoWheelCompactComboBox(this);
//    m_controllers_dropdown->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_dropdowns->addWidget(m_controllers_dropdown, 3);
    refresh_controllers(session.controller_type(), session.available_controllers());

    m_status_text = new QLabel(this);
    layoutR->addWidget(m_status_text, CONSOLE_SETTINGS_STRETCH_L1_RIGHT);
    layoutR->addSpacing(5);

    m_status_text->setText(QString::fromStdString(session.status_text()));
    m_status_text->setTextFormat(Qt::RichText);
    m_status_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_status_text->setOpenExternalLinks(true);

    m_reset_button = new QPushButton("Reset Ctrl.", this);
#if 1
    m_reset_button->setToolTip(
        "<b>Click:</b> Reset controller and reconnect.<br><br>"
        "<b>Shift+Click:</b> Reset and clear the controller of any state. "
        "If the controller supports pairing, this will unpair it and allow it to pair with a new host."
    );
#endif
    layoutR->addWidget(m_reset_button, CONSOLE_SETTINGS_STRETCH_L1_BUTTON);

    bool options_locked = session.options_locked();
    if (m_selector){
        m_selector->setEnabled(!options_locked);
    }
    m_reset_button->setEnabled(!options_locked);

    setFocusPolicy(Qt::StrongFocus);

    connect(
        m_interface_dropdown, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            index = std::max(index, 0);
//            index = std::min(index, (int)m_device_list.size() - 1);

            ControllerInterface incoming = m_interface_list[index];
            ControllerInterface existing = m_session.descriptor()->interface_type;
//            cout << "incoming = " << (int)incoming << endl;
//            cout << "existing = " << (int)existing << endl;
            if (incoming == existing){
                return;
            }

            m_session.set_interface(incoming);

            refresh_selection();
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
            ControllerType previous_controller = m_session.controller_type();
            if (new_value == previous_controller){
                return;
            }

            if (previous_controller != ControllerType::NintendoSwitch_WiredProController &&
                previous_controller != ControllerType::NintendoSwitch2_WiredProController
            ){
                m_session.set_controller(new_value);
                return;
            }

            if (new_value != ControllerType::NintendoSwitch_WiredController &&
                new_value != ControllerType::NintendoSwitch2_WiredController
            ){
                m_session.set_controller(new_value);
                return;
            }

            QMessageBox box;
            QMessageBox::StandardButton reply = box.warning(
                this,
                "Warning",
                "You are attempting to change from a Pro Controller to a 3rd party wired controller.<br><br>"
                "Are you sure you wish to do this?<br><br>"
                "If you are doing this because our YouTube video is telling you to do this, STOP. "
                "That video is out of date.<br><br>"
                "Since that video was made, we have added support for the Pro Controller which "
                "supports many more features. You should not need to use the 3rd party wired controllers anymore.",
                QMessageBox::Yes | QMessageBox::Cancel
            );
            if (reply == QMessageBox::Yes){
                m_session.set_controller(new_value);
            }else{
                refresh_controllers(previous_controller, m_session.available_controllers());
            }
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



void ControllerSelectorWidget::update_interface_dropdown(ControllerInterface interface_type){
    for (size_t index = 0; index < m_interface_list.size(); index++){
        if (interface_type == m_interface_list[index]){
            m_interface_dropdown->setCurrentIndex((int)index);
            return;
        }
    }

//    m_session.set_controller(ControllerType::None);
    m_interface_dropdown->setCurrentIndex(-1);
}
void ControllerSelectorWidget::refresh_selection(){
//    cout << "refresh_selection()" << endl;

    delete m_selector;
    m_selector = nullptr;

    auto current = m_session.descriptor();
    if (current == nullptr){
        m_selector = new QWidget(this);
    }else{
        update_interface_dropdown(current->interface_type);
        m_selector = &static_cast<UiComponentQtWidget&>(*current->make_ui_component(this)).widget();
        m_dropdowns->insertWidget(1, m_selector, 1);
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
        refresh_selection();
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
        if (m_selector){
            m_selector->setEnabled(!locked);
        }
        m_interface_dropdown->setEnabled(!locked);
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
#endif



}
