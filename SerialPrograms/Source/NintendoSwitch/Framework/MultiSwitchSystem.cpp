/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "MultiSwitchSystem.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemFactory::MultiSwitchSystemFactory(
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    size_t min_switches,
    size_t max_switches,
    size_t switches
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    switches = std::max(switches, m_min_switches);
    switches = std::min(switches, m_max_switches);
    resize(switches);
}
MultiSwitchSystemFactory::MultiSwitchSystemFactory(
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    size_t min_switches,
    size_t max_switches,
    const QJsonValue& json
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    MultiSwitchSystemFactory::load_json(json);
    if (m_switches.size() < m_min_switches){
        resize(m_min_switches);
    }
}
void MultiSwitchSystemFactory::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    QJsonArray array = json_get_array_nothrow(obj, "DeviceList");
    if (array.size() == 0 || (size_t)array.size() > MAX_SWITCHES){
        return;
    }
    m_switches.clear();
    for (int c = 0; c < array.size(); c++){
        m_switches.emplace_back(
            new SwitchSystemFactory(
                QString("Switch ") + QString::number(c),
                "Switch " + std::to_string(c),
                m_min_pabotbase, m_feedback,
                array[c]
            )
        );
    }
    json_get_int(m_active_switches, obj, "ActiveDevices", (int)m_min_switches, (int)m_max_switches);
}
QJsonValue MultiSwitchSystemFactory::to_json() const{
    QJsonObject obj;
    obj.insert("ActiveDevices", QJsonValue((int)m_active_switches));
    QJsonArray array;
    for (const auto& item : m_switches){
        array.push_back(item->to_json());
    }
    obj.insert("DeviceList", array);
    return obj;
}
void MultiSwitchSystemFactory::resize(size_t count){
    while (m_switches.size() < count){
        m_switches.emplace_back(
            new SwitchSystemFactory(
                QString("Switch ") + QString::number(m_switches.size()),
                "Switch " + std::to_string(m_switches.size()),
                m_min_pabotbase, m_feedback
            )
        );
    }
    m_active_switches = count;
}

const QSerialPortInfo* MultiSwitchSystemFactory::port(size_t index) const{
    return m_switches[index]->port();
}
//const QCameraInfo* MultiSwitchSystemFactory::camera(size_t index) const{
//    return m_switches[index]->camera();
//}

SwitchSetup* MultiSwitchSystemFactory::make_ui(QWidget& parent, Logger& logger, uint64_t program_id){
    return new MultiSwitchSystem(parent, *this, logger, program_id);
}


MultiSwitchSystem::MultiSwitchSystem(
    QWidget& parent,
    MultiSwitchSystemFactory& factory,
    Logger& logger,
    uint64_t program_id
)
    : SwitchSetup(parent, factory)
    , m_program_id(program_id)
    , m_factory(factory)
    , m_logger(logger)
    , m_videos(nullptr)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setMargin(0);

    QHBoxLayout* row = new QHBoxLayout();
    vbox->addLayout(row, 0);
    row->setMargin(0);
    row->addStretch(2);
    row->addWidget(new QLabel("<b>Switch Count:</b>", this), 0);
    m_console_count_box = new NoWheelComboBox(this);
    row->addWidget(m_console_count_box, 1);
    row->addStretch(2);
    for (size_t c = factory.m_min_switches; c <= factory.m_max_switches; c++){
        m_console_count_box->addItem(QString::number(c));
    }
    m_console_count_box->setCurrentIndex((int)(m_factory.m_active_switches - factory.m_min_switches));

    connect(
        m_console_count_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0 || index > (int)(m_factory.m_max_switches - m_factory.m_min_switches)){
                return;
            }
            redraw_videos(index + m_factory.m_min_switches);
        }
    );

    redraw_videos(factory.m_active_switches);
}
void MultiSwitchSystem::redraw_videos(size_t count){
    if (count == m_switches.size()){
        return;
    }

    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    m_switches.clear();
    if (m_videos != nullptr){
        layout->removeWidget(m_videos);
        delete m_videos;
        m_videos = nullptr;
    }

    m_factory.resize(count);
    for (size_t c = 0; c < m_factory.m_active_switches; c++){
        const auto& item = m_factory.m_switches[c];
        m_switches.emplace_back((SwitchSystem*)item->make_ui(*this, m_logger, m_program_id));
    }

    m_videos = new QWidget(this);
    this->layout()->addWidget(m_videos);
    QVBoxLayout* vbox = new QVBoxLayout(m_videos);
    vbox->setMargin(0);

    QHBoxLayout* vrow0 = new QHBoxLayout();
    vbox->addLayout(vrow0, 1);
    vrow0->setMargin(0);

    vrow0->addWidget(m_switches[0], 1);
    if (m_switches.size() >= 2){
        vrow0->addWidget(m_switches[1], 1);
    }
    if (m_switches.size() >= 3){
        QHBoxLayout* vrow1 = new QHBoxLayout();
        vbox->addLayout(vrow1, 1);
        vrow1->setMargin(0);
        vrow1->addWidget(m_switches[2], 1);
        if (m_switches.size() >= MultiSwitchSystemFactory::MAX_SWITCHES){
            vrow1->addWidget(m_switches[3], 1);
        }else{
            vrow1->addWidget(new QWidget(), 1);
        }
    }
    static_assert(MultiSwitchSystemFactory::MAX_SWITCHES <= 4, "Can't display more than 4 Switches.");

    for (const auto& item : m_switches){
        connect(
            item, &SwitchSystem::on_program_state_changed,
            this, [=]{ on_program_state_changed(); }
        );
    }
    on_setup_changed();
}

bool MultiSwitchSystem::serial_ok() const{
    for (const auto& item : m_switches){
        if (!item->serial_ok()){
            return false;
        }
    }
    return true;
}
void MultiSwitchSystem::wait_for_all_requests(){
    for (const auto& item : m_switches){
        item->wait_for_all_requests();
    }
}
void MultiSwitchSystem::stop_serial(){
    for (const auto& item : m_switches){
        item->stop_serial();
    }
}
void MultiSwitchSystem::reset_serial(){
    for (const auto& item : m_switches){
        item->reset_serial();
    }
}
void MultiSwitchSystem::update_ui(ProgramState state){
    m_console_count_box->setEnabled(state == ProgramState::STOPPED);
    for (const auto& item : m_switches){
        item->update_ui(state);
    }
}

#if 0
void SwitchSystem4::change_serial(size_t old_index, size_t new_index, SwitchSystem& system){
    {
        auto iter = m_active_ports.find(old_index);
        if (iter != m_active_ports.end()){
            iter->second->clear_serial();
        }
    }
    {
        auto iter = m_active_ports.find(new_index);
        if (iter != m_active_ports.end()){
            iter->second = &system;
        }
    }
}
#endif




}
}





