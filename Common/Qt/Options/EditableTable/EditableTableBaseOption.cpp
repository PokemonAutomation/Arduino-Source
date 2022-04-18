/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QJsonArray>
#include "EditableTableBaseOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


EditableTableBaseOption::EditableTableBaseOption(
    QString label, const EditableTableFactory& factory,
    std::vector<std::unique_ptr<EditableTableRow>> default_value
)
    : m_label(std::move(label))
    , m_factory(factory)
    , m_default(std::move(default_value))
{
    restore_defaults();
}

std::vector<std::unique_ptr<EditableTableRow>> EditableTableBaseOption::load_json(const QJsonValue& json){
    std::vector<std::unique_ptr<EditableTableRow>> table;
    for (const auto& row : json.toArray()){
        table.emplace_back(m_factory.make_row());
        table.back()->load_json(row);
    }
    return table;
}
QJsonValue EditableTableBaseOption::to_json(const std::vector<std::unique_ptr<EditableTableRow>>& table) const{
    QJsonArray array;
    for (const std::unique_ptr<EditableTableRow>& row : table){
        array.append(row->to_json());
    }
    return array;
}
void EditableTableBaseOption::load_default(const QJsonValue& json){
    m_default = load_json(json);
}
void EditableTableBaseOption::load_current(const QJsonValue& json){
    if (json.isArray()){
        m_current = load_json(json);
    }
}
QJsonValue EditableTableBaseOption::write_default() const{
    return to_json(m_default);
}
QJsonValue EditableTableBaseOption::write_current() const{
    return to_json(m_current);
}

const EditableTableRow& EditableTableBaseOption::operator[](size_t index) const{
    return *m_current[index];
}

QString EditableTableBaseOption::check_validity() const{
    for (const std::unique_ptr<EditableTableRow>& item : m_current){
        QString error = item->check_validity();
        if (!error.isEmpty()){
            return error;
        }
    }
    return QString();
}
void EditableTableBaseOption::restore_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> tmp;
    for (const std::unique_ptr<EditableTableRow>& item : m_default){
        tmp.emplace_back(item->clone());
    }
    m_current = std::move(tmp);
}
void EditableTableBaseOption::add_row(size_t index, std::unique_ptr<EditableTableRow> row){
    m_current.insert(m_current.begin() + index, std::move(row));
}
void EditableTableBaseOption::remove_row(size_t index){
    m_current.erase(m_current.begin() + index);
}









}
