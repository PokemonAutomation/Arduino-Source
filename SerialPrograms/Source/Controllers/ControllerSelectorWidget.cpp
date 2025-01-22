/*  Controller Selector Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include "Common/Qt/NoWheelComboBox.h"
#include "Controllers/NullController.h"
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

    m_devices_dropdown = new NoWheelComboBox(this);
    serial_row->addWidget(m_devices_dropdown, 5);
    refresh();
    serial_row->addSpacing(5);

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
            refresh();
        }
    );
    connect(
        m_reset_button, &QPushButton::clicked,
        this, [this](bool){
            m_session.reset();
        }
    );

    session.add_listener(*this);
}



void ControllerSelectorWidget::refresh(){
    m_device_list.clear();
    m_devices_dropdown->clear();

    //  Rebuild the device list.
    m_device_list = get_compatible_descriptors(m_session.requirements());
    m_device_list.insert(m_device_list.begin(), std::make_unique<NullControllerDescriptor>());

    std::shared_ptr<const ControllerDescriptor> current = m_session.descriptor();

    size_t index = 0;
    for (size_t c = 0; c < m_device_list.size(); c++){
        std::string name = m_device_list[c]->display_name();
        m_devices_dropdown->addItem(QString::fromStdString(name));
        if (*current == *m_device_list[c]){
            index = c;
        }
    }
    m_devices_dropdown->setCurrentIndex((int)index);
}



void ControllerSelectorWidget::controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor){
    QMetaObject::invokeMethod(this, [this]{
        refresh();
    });
}
void ControllerSelectorWidget::status_text_changed(const std::string& text){
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
