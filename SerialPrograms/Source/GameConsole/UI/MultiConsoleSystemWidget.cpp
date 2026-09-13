/*  Multi-Console System Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "Common/Qt/Redispatch.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "MultiConsoleSystemWidget.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<GameConsole::MultiConsoleSystemWidget>;

namespace GameConsole{


MultiConsoleSystemWidget::~MultiConsoleSystemWidget(){
    m_session.remove_console_count_lock(*this);
    m_session.remove_listener(*this);

    delete m_videos;
}
MultiConsoleSystemWidget::MultiConsoleSystemWidget(
    QWidget& parent,
    MultiConsoleSystemSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* row = new QHBoxLayout();
    vbox->addLayout(row, 0);
    row->setContentsMargins(0, 0, 0, 0);
    row->addStretch(2);
    row->addWidget(new QLabel("<b>Console Count:</b>", this), 0);
    m_console_count_box = new NoWheelCompactComboBox(this);
    row->addWidget(m_console_count_box, 1);
    row->addStretch(2);

    for (size_t c = session.min_consoles(); c <= session.max_consoles(); c++){
        m_console_count_box->addItem(QString::number(c));
    }

    m_session.add_listener(*this);

    redraw_videos();

    connect(
        m_console_count_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [this](int index){
            if (index < 0 || index > (int)(m_session.max_consoles() - m_session.min_consoles())){
                return;
            }
            m_session.set_active_consoles(index + m_session.min_consoles());
        }
    );
}


void MultiConsoleSystemWidget::on_console_count_lock(bool locked){
    cout << "MultiConsoleSystemWidget::on_console_count_lock(): " << locked << endl;
    QMetaObject::invokeMethod(this, [=, this]{
        m_console_count_box->setEnabled(!locked);
    }, Qt::QueuedConnection);
}
void MultiConsoleSystemWidget::shutdown(){
    run_on_main_thread_and_wait([this]{
        delete m_videos;
        m_videos = nullptr;
    });
    m_session.remove_console_count_lock(*this);
}
void MultiConsoleSystemWidget::startup(size_t console_count){
    QMetaObject::invokeMethod(this, [this]{
        redraw_videos();
    }, Qt::QueuedConnection);
}

void MultiConsoleSystemWidget::redraw_videos(){
    int old_index = m_console_count_box->currentIndex();
    int new_index = (int)(m_session.active_consoles() - m_session.min_consoles());
    if (new_index != old_index){
        m_console_count_box->setCurrentIndex(new_index);
    }

    delete m_videos;

    m_videos = new QWidget(this);
    this->layout()->addWidget(m_videos);
    QVBoxLayout* vbox = new QVBoxLayout(m_videos);
    vbox->setContentsMargins(0, 0, 0, 0);

    std::vector<QWidget*> consoles;
    consoles.reserve(m_session.active_consoles());

    m_session.add_console_count_lock(*this);

    for (size_t c = 0; c < m_session.active_consoles(); c++){
        UiWrapper wrapper = m_session[c].make_ui_component(this);
        consoles.emplace_back(dynamic_cast<QWidget*>(wrapper.release()));
    }

    QHBoxLayout* vrow0 = new QHBoxLayout();
    vbox->addLayout(vrow0, 1);
    vrow0->setContentsMargins(0, 0, 0, 0);

    vrow0->addWidget(consoles[0], 1);
    if (consoles.size() >= 2){
        vrow0->addWidget(consoles[1], 1);
    }
    if (consoles.size() >= 3){
        QHBoxLayout* vrow1 = new QHBoxLayout();
        vbox->addLayout(vrow1, 1);
        vrow1->setContentsMargins(0, 0, 0, 0);
        vrow1->addWidget(consoles[2], 1);
        if (consoles.size() >= MultiConsoleSystemOption::MAX_CONSOLES){
            vrow1->addWidget(consoles[3], 1);
        }else{
            vrow1->addWidget(new QWidget(), 1);
        }
    }
    static_assert(
        MultiConsoleSystemOption::MAX_CONSOLES <= 4,
        "Can't display more than 4 consoles."
    );
}







}
}
