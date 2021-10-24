/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableOptionBase_H
#define PokemonAutomation_EditableTableOptionBase_H

#include <memory>
#include <QTableWidget>
#include "Common/Qt/AutoHeightTable.h"

namespace PokemonAutomation{


class EditableTableRow{
public:
    virtual ~EditableTableRow() = default;

    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    virtual std::unique_ptr<EditableTableRow> clone() const = 0;

    virtual QString check_validity() const{ return QString(); }
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) = 0;

private:
    friend class EditableTableBaseUI;
    size_t m_index;
};

class EditableTableFactory{
public:
    virtual QStringList make_header() const = 0;
    virtual std::unique_ptr<EditableTableRow> make_row() const = 0;
};



class EditableTableBase{
public:
    EditableTableBase(QString label, const EditableTableFactory& factory, bool margin);

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    size_t size() const{ return m_current.size(); }
    const EditableTableRow& operator[](size_t index) const;

    QString check_validity() const;
    void restore_defaults();

public:
    void add_row(size_t index, std::unique_ptr<EditableTableRow> row);
    void remove_row(size_t index);

private:
    std::vector<std::unique_ptr<EditableTableRow>> load_json(const QJsonValue& json);
    QJsonValue to_json(const std::vector<std::unique_ptr<EditableTableRow>>& table) const;

private:
    friend class EditableTableBaseUI;
    QString m_label;
    const EditableTableFactory& m_factory;
    bool m_margin;
    std::vector<std::unique_ptr<EditableTableRow>> m_default;
    std::vector<std::unique_ptr<EditableTableRow>> m_current;
};

class EditableTableBaseUI : public QWidget{
public:
    EditableTableBaseUI(QWidget& parent, EditableTableBase& value);
    void restore_defaults();

    void update_column_sizes();

private:
    void redraw_table();
    void insert_row(int index, std::unique_ptr<EditableTableRow> row);
    void delete_row(int index);
    QWidget* make_insert_button(EditableTableRow& row);
    QWidget* make_delete_button(EditableTableRow& row);

private:
    EditableTableBase& m_value;
    AutoHeightTableWidget* m_table;
};



}
#endif
