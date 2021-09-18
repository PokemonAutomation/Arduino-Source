/*  Fossil Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FossilTableOptionBase_H
#define PokemonAutomation_FossilTableOptionBase_H

#include <memory>
#include <vector>
#include <QJsonValue>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QComboBox>

namespace PokemonAutomation{


class FossilTableOptionBase{
public:
    static const QString JSON_GAME_SLOT;
    static const QString JSON_USER_SLOT;
    static const QString JSON_FOSSIL;
    static const QString JSON_REVIVES;

    static const std::vector<QString> FOSSIL_LIST;

    enum Fossil{
        Dracozolt   =   0,
        Arctozolt   =   1,
        Dracovish   =   2,
        Arctovish   =   3,
    };
    struct GameSlot{
        int game_slot;
        int user_slot;
        Fossil fossil;
        int revives;
    };

public:
    FossilTableOptionBase(QString label);

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    size_t size() const{ return m_current.size(); }
    const GameSlot& operator[](size_t index) const{ return m_current[index]; }
    const std::vector<GameSlot>& value() const{ return m_current; }

    bool is_valid() const;
    void restore_defaults();

protected:
    friend class FossilTableOptionBaseUI;
    const QString m_label;
    std::vector<GameSlot> m_default;
    std::vector<GameSlot> m_current;
};


class FossilTableOptionBaseUI : public QWidget{
public:
    FossilTableOptionBaseUI(QWidget& parent, FossilTableOptionBase& value);
    void restore_defaults();

private:
    void replace_table();
    void add_row(int row, const FossilTableOptionBase::GameSlot& game, int& index_ref);
    QComboBox* make_game_slot_box(QWidget& parent, int& row, int slot);
    QComboBox* make_user_slot_box(QWidget& parent, int& row, int slot);
    QComboBox* make_fossil_slot_box(QWidget& parent, int& row, FossilTableOptionBase::Fossil fossil);
    QLineEdit* make_revives_slot_box(QWidget& parent, int& row, int revives);
    QPushButton* make_insert_button(QWidget& parent, int& row);
    QPushButton* make_remove_button(QWidget& parent, int& row);

private:
    FossilTableOptionBase& m_value;
    QTableWidget* m_table;
    std::vector<std::unique_ptr<int>> m_index_table;
};



}
#endif

