/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Logging/Logger.h"
#include "StringSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



ConfigWidget* StringSelectCell::make_ui(QWidget& parent){
    return new StringSelectCellWidget(parent, *this);
}
ConfigWidget* StringSelectOption::make_ui(QWidget& parent){
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

    const StringSelectEntry& entry = value.entry();
    QPixmap pixmap = QPixmap::fromImage(entry.icon.to_QImage_ref());
    this->addItem(pixmap, QString::fromStdString(entry.display_name));
    this->setCurrentIndex(0);

    StringSelectCellWidget::update();

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
    if (this->count() <= 1){
        const std::vector<StringSelectEntry>& cases = m_value.database().case_list();
        global_logger_tagged().log("Loading dropdown with " + tostr_u_commas(cases.size()) + " elements.");
        this->clear();
        for (const auto& item : cases){
            QPixmap pixmap = QPixmap::fromImage(item.icon.to_QImage_ref());
            this->addItem(pixmap, QString::fromStdString(item.display_name));
        }
    }
    this->setCurrentIndex((int)m_value.index());
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
}
void StringSelectCellWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent()" << endl;
    update();
    NoWheelComboBox::focusInEvent(event);
}
void StringSelectCellWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent()" << endl;
    NoWheelComboBox::focusOutEvent(event);
//    update();
}
void StringSelectCellWidget::update(){
    ConfigWidget::update();
    if (hasFocus()){
        load_options();
    }else{
        hide_options();
    }
}
void StringSelectCellWidget::value_changed(){
    QMetaObject::invokeMethod(this, [this]{
        update();
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
void StringSelectOptionWidget::update(){
    ConfigWidget::update();
    m_cell->update();
}





}
