/*  Switch System (4 Switches)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include "Common/Qt/NoWheelComboBox.h"
#include "NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch_MultiSwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemWidget::MultiSwitchSystemWidget(
    QWidget& parent,
    MultiSwitchSystemFactory& factory,
    Logger& logger,
    uint64_t program_id
)
    : SwitchSetupWidget(parent, factory)
    , m_program_id(program_id)
    , m_factory(factory)
    , m_logger(logger)
    , m_videos(nullptr)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* row = new QHBoxLayout();
    vbox->addLayout(row, 0);
    row->setContentsMargins(0, 0, 0, 0);
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
void MultiSwitchSystemWidget::redraw_videos(size_t count){
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
        m_switches.emplace_back((SwitchSystemWidget*)item->make_ui(*this, m_logger, m_program_id));
    }

    m_videos = new QWidget(this);
    this->layout()->addWidget(m_videos);
    QVBoxLayout* vbox = new QVBoxLayout(m_videos);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* vrow0 = new QHBoxLayout();
    vbox->addLayout(vrow0, 1);
    vrow0->setContentsMargins(0, 0, 0, 0);

    vrow0->addWidget(m_switches[0], 1);
    if (m_switches.size() >= 2){
        vrow0->addWidget(m_switches[1], 1);
    }
    if (m_switches.size() >= 3){
        QHBoxLayout* vrow1 = new QHBoxLayout();
        vbox->addLayout(vrow1, 1);
        vrow1->setContentsMargins(0, 0, 0, 0);
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
            item, &SwitchSystemWidget::on_program_state_changed,
            this, [=]{ on_program_state_changed(); }
        );
    }
    on_setup_changed();
}

bool MultiSwitchSystemWidget::serial_ok() const{
    for (const auto& item : m_switches){
        if (!item->serial_ok()){
            return false;
        }
    }
    return true;
}
void MultiSwitchSystemWidget::wait_for_all_requests(){
    for (const auto& item : m_switches){
        item->wait_for_all_requests();
    }
}
void MultiSwitchSystemWidget::stop_serial(){
    for (const auto& item : m_switches){
        item->stop_serial();
    }
}
void MultiSwitchSystemWidget::reset_serial(){
    for (const auto& item : m_switches){
        item->reset_serial();
    }
}
void MultiSwitchSystemWidget::update_ui(ProgramState state){
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
