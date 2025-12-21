/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include <QLineEdit>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Logging/Logger.h"
#include "StringSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



ConfigWidget* StringSelectCell::make_QtWidget(QWidget& parent){
    return new StringSelectCellWidget(parent, *this);
}
ConfigWidget* StringSelectOption::make_QtWidget(QWidget& parent){
    return new StringSelectOptionWidget(parent, *this);
}




StringSelectCellWidget::~StringSelectCellWidget(){
    m_value.remove_listener(*this);
}
StringSelectCellWidget::StringSelectCellWidget(QWidget& parent, StringSelectCell& value)
    : NoWheelComboBox(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);
    this->completer()->setCompletionMode(QCompleter::PopupCompletion);
    this->completer()->setFilterMode(Qt::MatchContains);
    this->setIconSize(QSize(25, 25));
//    this->setFrame(false);

    const StringSelectEntry& entry = value.entry();
    QPixmap pixmap = QPixmap::fromImage(entry.icon.to_QImage_ref());
    this->addItem(pixmap, QString::fromStdString(entry.display_name));
    this->setCurrentIndex(0);
//    this->setMinimumContentsLength((int)value.database().longest_text_length());

    StringSelectCellWidget::update_value();

    QPalette palette;
    palette.setColor(QPalette::Text, QColor((uint32_t)entry.text_color));
    this->lineEdit()->setPalette(palette);

    connect(
        this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.set_by_name(this->itemText(index).toStdString());
//            cout << "index = " << index << endl;
        }
    );

    value.add_listener(*this);
}

void StringSelectCellWidget::load_options(){
//    cout << "load_options()" << endl;
    const std::vector<StringSelectEntry>& cases = m_value.database().case_list();
    if (this->count() <= 1){
        global_logger_tagged().log("Loading dropdown with " + tostr_u_commas(cases.size()) + " elements.");
        this->clear();
        for (const StringSelectEntry& item : cases){
            QPixmap pixmap = QPixmap::fromImage(item.icon.to_QImage_ref());
            this->addItem(pixmap, QString::fromStdString(item.display_name));
            if (item.text_color){
                this->setItemData(this->count() - 1, QBrush(QColor((uint32_t)item.text_color)), Qt::ForegroundRole);
            }
        }
    }
    size_t index = m_value.index();
    this->setCurrentIndex((int)index);

    QPalette palette;
    palette.setColor(QPalette::Text, QColor((uint32_t)cases[index].text_color));
//    palette.setColor(QPalette::Text, Qt::red);
    this->lineEdit()->setPalette(palette);

    update_size_cache();
}
void StringSelectCellWidget::hide_options(){
//    cout << "hide_options()" << endl;
    do{
        //  All options shown.
        if (this->count() > 1){
            break;
        }

        //  No valid index.
        int ui_index = this->currentIndex();
        if (ui_index < 0){
            break;
        }

        //  Mismatching selection.
        std::string name = this->itemText(ui_index).toStdString();
        if (name != m_value.display_name()){
            break;
        }

        return;
    }while (false);

    //  Remove all elements and add the one that is selected.
    this->clear();
    const StringSelectEntry& entry = m_value.entry();
    QPixmap pixmap = QPixmap::fromImage(entry.icon.to_QImage_ref());
    this->addItem(pixmap, QString::fromStdString(entry.display_name));
    this->setCurrentIndex(0);

    QPalette palette;
    palette.setColor(QPalette::Text, QColor((uint32_t)entry.text_color));
//    palette.setColor(QPalette::Text, Qt::red);
    this->lineEdit()->setPalette(palette);

}
QSize StringSelectCellWidget::sizeHint() const{
    QSize ret = NoWheelComboBox::sizeHint();
//    cout << ret.width() << " x " << ret.height() << endl;

    double width = ret.width();
    double height = ret.height();

    width *= 1.25;
    height *= 1.25;

    return QSize((int)width, (int)height);
}
void StringSelectCellWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent()" << endl;
    update_value();
    NoWheelComboBox::focusInEvent(event);
}
void StringSelectCellWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent()" << endl;
    NoWheelComboBox::focusOutEvent(event);
//    update_value();
}
void StringSelectCellWidget::update_value(){
    if (hasFocus()){
        load_options();
    }else{
        hide_options();
    }
}
void StringSelectCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





StringSelectOptionWidget::StringSelectOptionWidget(QWidget& parent, StringSelectOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_cell(new StringSelectCellWidget(parent, value))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
}





}
