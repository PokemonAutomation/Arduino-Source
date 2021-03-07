/*  Multi-Host Table Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_MultiHostTableOption_H
#define PokemonAutomation_MultiHostTableOption_H

#include <memory>
#include <vector>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidget>

namespace PokemonAutomation{


class MultiHostTableOptionUI;
class MultiHostTableOption{
public:
    static const QString JSON_GAME_SLOT;
    static const QString JSON_USER_SLOT;
    static const QString JSON_SKIPS;
    static const QString JSON_BACKUP_SAVE;
    static const QString JSON_ALWAYS_CATCHABLE;
    static const QString JSON_ACCEPT_FRS;
    static const QString JSON_MOVE_SLOT;
    static const QString JSON_DYNAMAX;
    static const QString JSON_POST_RAID_DELAY;

    struct GameSlot{
        int game_slot;
        int user_slot;
        int skips;
        bool backup_save;
        bool always_catchable;
        bool accept_FRs;
        int move_slot;
        bool dynamax;
        QString post_raid_delay;
    };

public:
    MultiHostTableOption(QString label);

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    size_t size() const{ return m_current.size(); }
    const GameSlot& operator[](size_t index) const{ return m_current[index]; }
    const std::vector<GameSlot>& value() const{ return m_current; }

    bool is_valid() const;
    void restore_defaults();

private:
    friend class MultiHostTableOptionUI;
    const QString m_label;
    std::vector<GameSlot> m_default;
    std::vector<GameSlot> m_current;
};


class MultiHostTableOptionUI : public QWidget{
public:
    MultiHostTableOptionUI(QWidget& parent, MultiHostTableOption& value);
    void restore_defaults();

private:
    void replace_table();
    void add_row(int row, const MultiHostTableOption::GameSlot& game, int& index_ref);
    QComboBox* make_game_slot_box(QWidget& parent, int& row, int slot);
    QComboBox* make_user_slot_box(QWidget& parent, int& row, int slot);
    QLineEdit* make_skips_box(QWidget& parent, int& row, int skips);
    QWidget* make_backup_save_box(QWidget& parent, int& row, bool backup_save);
    QWidget* make_catchable_box(QWidget& parent, int& row, bool always_catchable);
    QWidget* make_accept_FRs_box(QWidget& parent, int& row, bool accept_FRs);
    QComboBox* make_move_slot_box(QWidget& parent, int& row, int move_slot);
    QWidget* make_dynamax_box(QWidget& parent, int& row, bool dynamax);
    QLineEdit* make_delay_box(QWidget& parent, int& row, const QString& post_raid_delay);
    QPushButton* make_insert_button(QWidget& parent, int& row);
    QPushButton* make_remove_button(QWidget& parent, int& row);

    void update_table_height();

private:
    MultiHostTableOption& m_value;
    QTableWidget* m_table;
    std::vector<std::unique_ptr<int>> m_index_table;
};


}
#endif
