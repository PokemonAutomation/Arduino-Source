/*  Program Notification Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/AutoHeightTable.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "ProgramNotifications.h"
#include "EventNotificationsTable.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{





class EventNotificationsTableWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~EventNotificationsTableWidget();
    EventNotificationsTableWidget(QWidget& parent, EventNotificationsTable& value);

    virtual void value_changed() override;

private:
    void redraw_table();
    QWidget* make_enabled_box   (EventNotificationOption& entry);
    QWidget* make_ping_box      (EventNotificationOption& entry);
    QWidget* make_screenshot_box(EventNotificationOption& entry);
    QWidget* make_tags_box      (EventNotificationOption& entry);
    QWidget* make_rate_limit_box(EventNotificationOption& entry);
    QWidget* make_test_box      (EventNotificationOption& entry);

private:
    EventNotificationsTable& m_value;
    AutoHeightTableWidget* m_table;
};




EventNotificationsTable::EventNotificationsTable(std::vector<EventNotificationOption*> options)
    : m_options(std::move(options))
{
    for (EventNotificationOption* option : m_options){
        auto iter = m_name_map.find(option->label());
        if (iter != m_name_map.end()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Key: " + option->label());
        }
        m_name_map.emplace(option->label(), option);
    }
}
void EventNotificationsTable::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    ScreenshotOption screenshot_option("");
    for (EventNotificationOption* option : m_options){
        auto iter = obj->find(option->label());
        if (iter == obj->end()){
            continue;
        }
        option->load_json(iter->second);
    }
}
JsonValue EventNotificationsTable::to_json() const{
    JsonObject obj;
    for (EventNotificationOption* option : m_options){
        obj[option->label()] = option->to_json();
    }
    return obj;
}
void EventNotificationsTable::restore_defaults(){
    for (EventNotificationOption* option : m_options){
        option->restore_defaults();
    }
    ConfigOption::push_update();
}
void EventNotificationsTable::reset_state(){
    for (EventNotificationOption* option : m_options){
        option->reset_rate_limit();
    }
}
ConfigWidget* EventNotificationsTable::make_ui(QWidget& parent){
    return new EventNotificationsTableWidget(parent, *this);
}
void EventNotificationsTable::set_enabled(bool enabled){
    for (EventNotificationOption* option : m_options){
        option->m_enabled = enabled;
    }
}




EventNotificationsTableWidget::~EventNotificationsTableWidget(){
    m_value.remove_listener(*this);
}
EventNotificationsTableWidget::EventNotificationsTableWidget(QWidget& parent, EventNotificationsTable& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_table = new AutoHeightTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);


    QStringList list;
//    list << "Enable" << "Event" << "Should Ping" << "Screenshot" << "Tags";
    list << "Enable" << "Event" << "Should Ping" << "Screenshot" << "Tags" << "Rate Limit (seconds)" << "";
    m_table->setColumnCount(list.size());
    m_table->setHorizontalHeaderLabels(list);

    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    redraw_table();

    value.add_listener(*this);
}
void EventNotificationsTableWidget::value_changed(){
    QMetaObject::invokeMethod(this, [this]{
        redraw_table();
    }, Qt::QueuedConnection);
}

void EventNotificationsTableWidget::redraw_table(){
    m_table->setRowCount(0);
    m_table->setRowCount((int)m_value.m_options.size());
    int stop = (int)m_value.m_options.size();
    for (int c = 0; c < stop; c++){
        EventNotificationOption& entry = *m_value.m_options[c];
        m_table->setCellWidget(c, 0, make_enabled_box(entry));
        m_table->setCellWidget(c, 1, new QLabel(QString::fromStdString(" " + entry.label() + " "), this));
        m_table->setCellWidget(c, 2, make_ping_box(entry));
        m_table->setCellWidget(c, 3, make_screenshot_box(entry));
        m_table->setCellWidget(c, 4, make_tags_box(entry));
        m_table->setCellWidget(c, 5, make_rate_limit_box(entry));
        m_table->setCellWidget(c, 6, make_test_box(entry));
    }
    m_table->resizeColumnsToContents();
}
QWidget* EventNotificationsTableWidget::make_enabled_box(EventNotificationOption& entry){
    QWidget* widget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(this);
    layout->addWidget(box);
    box->setChecked(entry.m_current.enabled);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            entry.m_current.enabled = box->isChecked();
        }
    );
    return widget;
}
QWidget* EventNotificationsTableWidget::make_ping_box(EventNotificationOption& entry){
    QWidget* widget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(this);
    layout->addWidget(box);
    box->setChecked(entry.m_current.ping);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            entry.m_current.ping = box->isChecked();
        }
    );
    return widget;
}
QWidget* EventNotificationsTableWidget::make_screenshot_box(EventNotificationOption& entry){
    if (entry.screenshot_supported){
        QComboBox* box = new NoWheelComboBox(this);
        box->addItem("No Screenshot.");
        box->addItem("Attach as .jpg.");
        box->addItem("Attach as .png.");
        box->setCurrentIndex((int)entry.m_current.screenshot);
        box->connect(
            box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            box, [&](int index){
                entry.m_current.screenshot = (ImageAttachmentMode)index;
            }
        );
        return box;
    }else{
        QLabel* box = new QLabel("---", this);
        box->setAlignment(Qt::AlignCenter);
        return box;
    }
}
QWidget* EventNotificationsTableWidget::make_tags_box(EventNotificationOption& entry){
    QLineEdit* box = new QLineEdit(this);
    box->setText(QString::fromStdString(EventNotificationSettings::tags_to_str(entry.m_current.tags)));
//    box->setAlignment(Qt::AlignCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            entry.m_current.tags = EventNotificationSettings::parse_tags(text.toStdString());
        }
    );
    return box;
}
QWidget* EventNotificationsTableWidget::make_rate_limit_box(EventNotificationOption& entry){
    QLineEdit* box = new QLineEdit(this);
    box->setText(QString::number(entry.m_current.rate_limit.count()));
    box->setAlignment(Qt::AlignCenter);
    QIntValidator* validator = new QIntValidator(0, std::numeric_limits<int>::max(), box);
    box->setValidator(validator);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            entry.m_current.rate_limit = std::chrono::seconds(text.toLongLong());
        }
    );
    return box;
}
QWidget* EventNotificationsTableWidget::make_test_box(EventNotificationOption& entry){
    QPushButton* button = new QPushButton(this);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Send Test Message");
//    button->setMaximumWidth(60);

    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            send_program_notification(
                global_logger_tagged(), entry,
                COLOR_GREEN,
                ProgramInfo("Notification Test"),
                "Notification Test",
                {
                    {"Event Type", entry.label()},
                }
            );
        }
    );

    return button;
}




EventNotificationsOption::EventNotificationsOption(std::vector<EventNotificationOption*> options)
    : GroupOption("Discord Notifications", true)
    , m_table(std::move(options))
{
    PA_ADD_OPTION(m_table);
}
void EventNotificationsOption::on_set_enabled(bool enabled){
    m_table.set_enabled(enabled);
}











}
