/*  Multi-Switch System Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include "Common/Qt/NoWheelComboBox.h"
#include "NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch_MultiSwitchSystemWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemWidget::~MultiSwitchSystemWidget(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        for (SwitchSystemWidget* console : m_switches){
            delete console;
        }
        m_switches.clear();
        delete m_videos;
        m_videos = nullptr;
        m_shutting_down = true;
    }
    m_session.remove_listener(*this);
}
MultiSwitchSystemWidget::MultiSwitchSystemWidget(
    QWidget& parent,
    MultiSwitchSystemSession& session,
    uint64_t program_id
)
    : QWidget(&parent)
    , m_program_id(program_id)
    , m_session(session)
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

    for (size_t c = session.min_switches(); c <= session.max_switches(); c++){
        m_console_count_box->addItem(QString::number(c));
    }
    m_console_count_box->setCurrentIndex((int)(m_session.count() - m_session.min_switches()));

    connect(
        m_console_count_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [this](int index){
            if (index < 0 || index > (int)(m_session.max_switches() - m_session.min_switches())){
                return;
            }
            m_session.set_switch_count(index + m_session.min_switches());
        }
    );


#if 0
    //  Acquire the lock now and attach listener. This will block the session
    //  from changing the # of switches while we draw everything.
    std::lock_guard<std::mutex> lg(m_lock);
    m_session.add_listener(*this);

    m_console_count_box->setCurrentIndex((int)(m_session.count() - m_session.min_switches()));

    redraw_videos(m_session.count());
#endif
    m_session.add_listener(*this);
}

void MultiSwitchSystemWidget::shutdown(){
//    cout << "MultiSwitchSystemWidget::shutdown()" << endl;
    std::lock_guard<std::mutex> lg(m_lock);
    for (SwitchSystemWidget* console : m_switches){
        delete console;
    }
    m_switches.clear();
    delete m_videos;
    m_videos = nullptr;
}
void MultiSwitchSystemWidget::startup(size_t switch_count){
//    cout << "MultiSwitchSystemWidget::startup()" << endl;
    std::lock_guard<std::mutex> lg(m_lock);
    redraw_videos(switch_count);
}

void MultiSwitchSystemWidget::redraw_videos(size_t count){
//    cout << "MultiSwitchSystemWidget::redraw_videos()" << endl;
//    std::lock_guard<std::mutex> lg(m_lock);
//    if (count == m_switches.size()){
//        return;
//    }
    if (m_shutting_down){
        return;
    }

    int old_index = m_console_count_box->currentIndex();
    int new_index = (int)(m_session.count() - m_session.min_switches());
    if (new_index != old_index){
        m_console_count_box->setCurrentIndex(new_index);
    }

    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    m_switches.clear();
    if (m_videos != nullptr){
        layout->removeWidget(m_videos);
        delete m_videos;
        m_videos = nullptr;
    }

    m_videos = new QWidget(this);
    this->layout()->addWidget(m_videos);
    QVBoxLayout* vbox = new QVBoxLayout(m_videos);
    vbox->setContentsMargins(0, 0, 0, 0);

//    m_option.resize(count);
    for (size_t c = 0; c < m_session.count(); c++){
//        const auto& item = m_option.m_switches[c];
//        m_switches.emplace_back(item->make_ui(*this, m_logger, m_program_id));
        m_switches.emplace_back(
            new SwitchSystemWidget(
                *m_videos,
                m_session[c],
                m_program_id
            )
        );
    }

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
        if (m_switches.size() >= MultiSwitchSystemOption::MAX_SWITCHES){
            vrow1->addWidget(m_switches[3], 1);
        }else{
            vrow1->addWidget(new QWidget(), 1);
        }
    }
    static_assert(MultiSwitchSystemOption::MAX_SWITCHES <= 4, "Can't display more than 4 Switches.");
}

void MultiSwitchSystemWidget::update_ui(ProgramState state){
    m_console_count_box->setEnabled(state == ProgramState::STOPPED);
    for (const auto& item : m_switches){
        item->update_ui(state);
    }
}




}
}
