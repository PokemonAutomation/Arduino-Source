/*  Multi-Host Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QCheckBox>
#include <QScrollBar>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "MultiHostTableOption.h"

namespace PokemonAutomation{


const QString MultiHostTableOption::JSON_GAME_SLOT          = "game_slot";
const QString MultiHostTableOption::JSON_USER_SLOT          = "user_slot";
const QString MultiHostTableOption::JSON_SKIPS              = "skips";
const QString MultiHostTableOption::JSON_BACKUP_SAVE        = "backup_save";
const QString MultiHostTableOption::JSON_ALWAYS_CATCHABLE   = "always_catchable";
const QString MultiHostTableOption::JSON_ACCEPT_FRS         = "accept_FRs";
const QString MultiHostTableOption::JSON_MOVE_SLOT          = "move_slot";
const QString MultiHostTableOption::JSON_DYNAMAX            = "dynamax";
const QString MultiHostTableOption::JSON_POST_RAID_DELAY    = "post_raid_delay";

std::vector<MultiHostTableOption::GameSlot> parse_game_slot(const QJsonValue& json){
    std::vector<MultiHostTableOption::GameSlot> list;
    for (const auto item : json.toArray()){
        QJsonObject line = item.toObject();
        MultiHostTableOption::GameSlot slot;

        json_get_int(slot.game_slot, line, MultiHostTableOption::JSON_GAME_SLOT, 1, 2);
        json_get_int(slot.user_slot, line, MultiHostTableOption::JSON_USER_SLOT, 1, 8);
        json_get_int(slot.skips, line, MultiHostTableOption::JSON_SKIPS, 1, 7);

        json_get_bool(slot.backup_save, line, MultiHostTableOption::JSON_BACKUP_SAVE);
        json_get_bool(slot.always_catchable, line, MultiHostTableOption::JSON_ALWAYS_CATCHABLE);
        json_get_bool(slot.accept_FRs, line, MultiHostTableOption::JSON_ACCEPT_FRS);

        json_get_int(slot.move_slot, line, MultiHostTableOption::JSON_MOVE_SLOT, 0, 4);
        json_get_bool(slot.dynamax, line, MultiHostTableOption::JSON_DYNAMAX);

        json_get_string(slot.post_raid_delay, line, MultiHostTableOption::JSON_POST_RAID_DELAY);

        list.emplace_back(std::move(slot));
    }
    return list;
}
QJsonArray write_game_slot(const std::vector<MultiHostTableOption::GameSlot>& list){
    QJsonArray ret;
    for (const auto& item : list){
        QJsonObject obj;
        obj.insert(MultiHostTableOption::JSON_GAME_SLOT, item.game_slot);
        obj.insert(MultiHostTableOption::JSON_USER_SLOT, item.user_slot);
        obj.insert(MultiHostTableOption::JSON_SKIPS, item.skips);
        obj.insert(MultiHostTableOption::JSON_BACKUP_SAVE, item.backup_save);
        obj.insert(MultiHostTableOption::JSON_ALWAYS_CATCHABLE, item.always_catchable);
        obj.insert(MultiHostTableOption::JSON_ACCEPT_FRS, item.accept_FRs);
        obj.insert(MultiHostTableOption::JSON_MOVE_SLOT, item.move_slot);
        obj.insert(MultiHostTableOption::JSON_DYNAMAX, item.dynamax);
        obj.insert(MultiHostTableOption::JSON_POST_RAID_DELAY, item.post_raid_delay);
        ret += std::move(obj);
    }
    return ret;
}

MultiHostTableOption::MultiHostTableOption(QString label)
    : m_label(std::move(label))
    , m_default({
        {
            .game_slot = 1,
            .user_slot = 1,
            .skips = 3,
            .backup_save = false,
            .always_catchable = true,
            .accept_FRs = true,
            .move_slot = 0,
            .dynamax = true,
            .post_raid_delay = "0 * TICKS_PER_SECOND",
        },
    })
    , m_current(m_default)
{}

void MultiHostTableOption::load_default(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_default = parse_game_slot(json);
}
void MultiHostTableOption::load_current(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_current = parse_game_slot(json);
}
QJsonValue MultiHostTableOption::write_default() const{
    return write_game_slot(m_default);
}
QJsonValue MultiHostTableOption::write_current() const{
    return write_game_slot(m_current);
}


bool MultiHostTableOption::is_valid() const{
    if (m_current.empty()){
        return false;
    }
    for (const auto& item : m_current){
        if (item.game_slot < 1 || item.game_slot > 2){
            return false;
        }
        if (item.user_slot < 1 || item.user_slot > 8){
            return false;
        }
        if (item.skips < 0 || item.skips > 7){
            return false;
        }
        if (item.move_slot < 0 || item.move_slot > 4){
            return false;
        }
        int ticks;
        try{
            ticks = parse_ticks_i32(item.post_raid_delay);
        }catch (...){
            return false;
        }
        if (ticks < 0 || ticks > 65535){
            return false;
        }
    }
    return true;
}
void MultiHostTableOption::restore_defaults(){
    m_current = m_default;
}




MultiHostTableOptionUI::MultiHostTableOptionUI(QWidget& parent, MultiHostTableOption& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    replace_table();
}
void MultiHostTableOptionUI::replace_table(){
    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    if (m_table != nullptr){
        layout->removeWidget(m_table);
        delete m_table;
        m_index_table.clear();
        m_table = nullptr;
    }

    m_table = new QTableWidget(this);
    layout->addWidget(m_table, 0, Qt::AlignTop);
    m_table->setColumnCount(11);

    QStringList header;
    header << "Game" << "User" << "Skips" << "Backup Save" << "Always Catchable" << "Accept FRs" << "1st Move" << "Dynamax" << "Post Raid Delay" << "" << "";
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


    size_t rows = m_value.m_current.size();
    m_table->setRowCount((int)(m_value.m_current.size() + 1));
    for (size_t c = 0; c < rows; c++){
        const auto& item = m_value.m_current[c];
        m_index_table.emplace_back(new size_t);
        add_row((int)c, item, *m_index_table.back());
    }

    QPushButton* button = new QPushButton(m_table);
    button->setText("Add Row");
    m_table->setCellWidget((int)m_value.m_current.size(), 0, button);
    update_table_height();
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            size_t index = m_index_table.size();

            //  Update data vector.
            m_value.m_current.emplace_back(
                MultiHostTableOption::GameSlot{
                    1, 1,
                    3, false, true,
                    true,
                    0, true,
                    "0 * TICKS_PER_SECOND"
                }
            );
            const auto& item = m_value.m_current.back();

            //  Update index vector.
            m_index_table.emplace_back(new size_t);

            //  Update UI.
            m_table->insertRow((int)index);
            add_row(index, item, *m_index_table.back());
            update_table_height();
        }
    );
}
void MultiHostTableOptionUI::add_row(size_t row, const MultiHostTableOption::GameSlot& game, size_t& index_ref){
    index_ref = row;
    m_table->setCellWidget((int)row,  0, make_game_slot_box(*m_table, index_ref, game.game_slot - 1));
    m_table->setCellWidget((int)row,  1, make_user_slot_box(*m_table, index_ref, game.user_slot - 1));
    m_table->setCellWidget((int)row,  2, make_skips_box(*m_table, index_ref, game.skips));
    m_table->setCellWidget((int)row,  3, make_backup_save_box(*m_table, index_ref, game.backup_save));
    m_table->setCellWidget((int)row,  4, make_catchable_box(*m_table, index_ref, game.always_catchable));
    m_table->setCellWidget((int)row,  5, make_accept_FRs_box(*m_table, index_ref, game.accept_FRs));
    m_table->setCellWidget((int)row,  6, make_move_slot_box(*m_table, index_ref, game.move_slot));
    m_table->setCellWidget((int)row,  7, make_dynamax_box(*m_table, index_ref, game.dynamax));
    m_table->setCellWidget((int)row,  8, make_delay_box(*m_table, index_ref, game.post_raid_delay));
    m_table->setCellWidget((int)row,  9, make_insert_button(*m_table, index_ref));
    m_table->setCellWidget((int)row, 10, make_remove_button(*m_table, index_ref));
}
QComboBox* MultiHostTableOptionUI::make_game_slot_box(QWidget& parent, size_t& row, size_t slot){
    QComboBox* box = new QComboBox(&parent);
    box->addItem("Slot 1");
    box->addItem("Slot 2");
    box->setCurrentIndex((int)slot);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].game_slot = index + 1;
        }
    );
    return box;
}
QComboBox* MultiHostTableOptionUI::make_user_slot_box(QWidget& parent, size_t& row, size_t slot){
    QComboBox* box = new QComboBox(&parent);
    box->addItem("User 1");
    box->addItem("User 2");
    box->addItem("User 3");
    box->addItem("User 4");
    box->addItem("User 5");
    box->addItem("User 6");
    box->addItem("User 7");
    box->addItem("User 8");
    box->setCurrentIndex((int)slot);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].user_slot = index + 1;
        }
    );
    return box;
}
QLineEdit* MultiHostTableOptionUI::make_skips_box(QWidget& parent, size_t& row, size_t skips){
    QLineEdit* box = new QLineEdit(&parent);
    QIntValidator* validator = new QIntValidator(0, 965, box);
    box->setValidator(validator);
    box->setText(QString::number(skips));
    box->setMaxLength(3);
    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
    connect(
        box, &QLineEdit::textChanged,
        this, [&, box](const QString& text){
            int revives = text.toInt();
            int fixed = revives;
            fixed = std::max(fixed, 0);
            fixed = std::min(fixed, 7);
            if (revives != fixed){
                box->setText(QString::number(fixed));
            }
            m_value.m_current[row].skips = fixed;
        }
    );
    return box;
}
QWidget* MultiHostTableOptionUI::make_backup_save_box(QWidget& parent, size_t& row, bool backup_save){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(backup_save);
    connect(
        box, &QCheckBox::stateChanged,
        this, [&, box](int){
            m_value.m_current[row].backup_save = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostTableOptionUI::make_catchable_box(QWidget& parent, size_t& row, bool always_catchable){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(always_catchable);
    connect(
        box, &QCheckBox::stateChanged,
        this, [&, box](int){
            m_value.m_current[row].always_catchable = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostTableOptionUI::make_accept_FRs_box(QWidget& parent, size_t& row, bool accept_FRs){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(accept_FRs);
    connect(
        box, &QCheckBox::stateChanged,
        this, [&, box](int){
            m_value.m_current[row].accept_FRs = box->isChecked();
        }
    );
    return widget;
}
QComboBox* MultiHostTableOptionUI::make_move_slot_box(QWidget& parent, size_t& row, size_t move_slot){
    QComboBox* box = new QComboBox(&parent);
    box->addItem("None");
    box->addItem("Slot 1");
    box->addItem("Slot 2");
    box->addItem("Slot 3");
    box->addItem("Slot 4");
    box->setCurrentIndex((int)move_slot);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].move_slot = index;
        }
    );
    return box;
}
QWidget* MultiHostTableOptionUI::make_dynamax_box(QWidget& parent, size_t& row, bool dynamax){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setMargin(0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(dynamax);
    connect(
        box, &QCheckBox::stateChanged,
        this, [&, box](int){
            m_value.m_current[row].dynamax = box->isChecked();
        }
    );
    return widget;
}
QLineEdit* MultiHostTableOptionUI::make_delay_box(QWidget& parent, size_t& row, const QString& post_raid_delay){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(post_raid_delay);
//    box->setMaxLength(3);
//    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
//    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(
        box, &QLineEdit::textChanged,
        this, [&](const QString& text){
            m_value.m_current[row].post_raid_delay = text;
//            QSize size = box->document()->size().toSize();
//            box->setFixedWidth(size);
        }
    );
    return box;
}
QPushButton* MultiHostTableOptionUI::make_insert_button(QWidget& parent, size_t& row){
    QPushButton* button = new QPushButton(&parent);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Insert");
//    QSizePolicy policy;
//    policy.setHorizontalPolicy(QSizePolicy::Minimum);
//    policy.setHorizontalStretch(0);
//    button->setSizePolicy(policy);
    button->setMaximumWidth(60);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            size_t index = row;
            MultiHostTableOption::GameSlot item{
                1, 1,
                3, false, true,
                true,
                0, true,
                "0 * TICKS_PER_SECOND"
            };

            //  Update data vector.
            m_value.m_current.insert(m_value.m_current.begin() + index, item);

            //  Update index vector.
            m_index_table.insert(m_index_table.begin() + index, std::unique_ptr<size_t>(new size_t(index)));
            size_t& new_row = *m_index_table[index];
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = c;
            }

            //  Update UI.
            m_table->insertRow((int)index);
            add_row(index, item, new_row);
            update_table_height();
        }
    );
    return button;
}
QPushButton* MultiHostTableOptionUI::make_remove_button(QWidget& parent, size_t& row){
    QPushButton* button = new QPushButton(&parent);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");
    button->setMaximumWidth(60);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            size_t index = row;

            //  Update data vector.
            m_index_table.erase(m_index_table.begin() + index);

            //  Update index vector.
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = (int)c;
            }
            m_value.m_current.erase(m_value.m_current.begin() + index);

            //  Update UI.
            m_table->removeRow((int)index);
            update_table_height();
        }
    );
    return button;
}

void MultiHostTableOptionUI::update_table_height(){
    int rows = m_table->rowCount();
    int total_height = 5;
    total_height += m_table->horizontalHeader()->height();
    total_height += m_table->horizontalScrollBar()->height();
    for (int c = 0; c < rows; c++) {
        total_height += m_table->verticalHeader()->sectionSize(c);
    }
//    cout << total_height << endl;
    m_table->setMinimumHeight(total_height);
    m_table->setMaximumHeight(total_height);
//    this->adjustSize();
//    this->parentWidget()->adjustSize();
//    this->parentWidget()->parentWidget()->adjustSize();
}


void MultiHostTableOptionUI::restore_defaults(){
    m_value.restore_defaults();
    replace_table();
}



}










