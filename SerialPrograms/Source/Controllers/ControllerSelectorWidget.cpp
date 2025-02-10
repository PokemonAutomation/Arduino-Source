/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "ControllerSelectorWidget.h"

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

    QHBoxLayout* dropdowns = new QHBoxLayout();
    serial_row->addLayout(dropdowns, 5);
    serial_row->addSpacing(5);

    m_devices_dropdown = new NoWheelComboBox(this);
    dropdowns->addWidget(m_devices_dropdown, 3);
    refresh_devices();

    if (PreloadSettings::instance().DEVELOPER_MODE){
        dropdowns->addSpacing(5);
        m_controllers_dropdown = new NoWheelComboBox(this);
        dropdowns->addWidget(m_controllers_dropdown, 2);
    }

    m_status_text = new QLabel(this);
    serial_row->addWidget(m_status_text, 3);
    serial_row->addSpacing(5);

    m_status_text->setText(QString::fromStdString(session.status_text()));

    m_reset_button = new QPushButton("Reset Ctrl.", this);
    serial_row->addWidget(m_reset_button, 1);

    bool options_locked = session.options_locked();
    m_devices_dropdown->setEnabled(!options_locked);
    m_reset_button->setEnabled(!options_locked);

    connect(
        m_devices_dropdown, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            index = std::max(index, 0);
            index = std::min(index, (int)m_device_list.size() - 1);
            const std::shared_ptr<const ControllerDescriptor>& selected = m_device_list[index];

            std::shared_ptr<const ControllerDescriptor> current = m_session.descriptor();
            if (*current == *selected){
                return;
            }

            m_session.set_device(selected);
            refresh_devices();
        }
    );
    if (PreloadSettings::instance().DEVELOPER_MODE){
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
    }
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            m_session.reset();
        }
    );

    session.add_listener(*this);
}



void ControllerSelectorWidget::refresh_devices(){
    m_device_list.clear();
    m_devices_dropdown->clear();

    //  Rebuild the device list.
    m_device_list = get_compatible_descriptors(m_session.requirements());
//    for (const auto& item : m_device_list){
//        cout << item->display_name() << endl;
//    }
//    m_device_list.insert(m_device_list.begin(), std::make_unique<NullControllerDescriptor>());

    std::shared_ptr<const ControllerDescriptor> current = m_session.descriptor();

    size_t index = 0;
    for (size_t c = 0; c < m_device_list.size(); c++){
        std::string name = m_device_list[c]->display_name();
        m_devices_dropdown->addItem(QString::fromStdString(name));
        if (*current == *m_device_list[c]){
            index = c;
        }
    }
//    cout << "m_device_list = " << m_device_list.size() << endl;
//    cout << "items = " << m_devices_dropdown->count() << endl;
    m_devices_dropdown->setCurrentIndex((int)index);
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
    QMetaObject::invokeMethod(this, [this]{
        refresh_devices();
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
        m_devices_dropdown->setEnabled(!locked);
        m_reset_button->setEnabled(!locked);
    });
}





}
