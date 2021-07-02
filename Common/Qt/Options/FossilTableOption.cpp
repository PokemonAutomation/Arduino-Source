/*  Fossil Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include "Common/Qt/QtJsonTools.h"
#include "FossilTableOption.h"

namespace PokemonAutomation{


const QString FossilTableOption::JSON_GAME_SLOT = "game_slot";
const QString FossilTableOption::JSON_USER_SLOT = "user_slot";
const QString FossilTableOption::JSON_FOSSIL    = "fossil";
const QString FossilTableOption::JSON_REVIVES   = "revives";

const std::vector<QString> FossilTableOption::FOSSIL_LIST{
    "Dracozolt",
    "Arctozolt",
    "Dracovish",
    "Arctovish",
};
const std::map<QString, int> FOSSIL_MAP{
    {"Dracozolt", 0},
    {"Arctozolt", 1},
    {"Dracovish", 2},
    {"Arctovish", 3},
};

std::vector<FossilTableOption::GameSlot> parse_fossil_slot(const QJsonValue& json){
    std::vector<FossilTableOption::GameSlot> list;
    for (const auto item : json.toArray()){
        QJsonObject line = item.toObject();
        FossilTableOption::GameSlot slot;

        json_get_int(slot.game_slot, line, FossilTableOption::JSON_GAME_SLOT, 1, 2);
        json_get_int(slot.user_slot, line, FossilTableOption::JSON_USER_SLOT, 1, 8);

        QString str;
        json_get_string(str, line, FossilTableOption::JSON_FOSSIL);
        auto iter = FOSSIL_MAP.find(str);
        if (iter != FOSSIL_MAP.end()){
            slot.fossil = (FossilTableOption::Fossil)iter->second;
        }

        json_get_int(slot.revives, line, FossilTableOption::JSON_REVIVES, 0, 965);

        list.emplace_back(slot);
    }
    return list;
}
QJsonArray write_fossil_slot(const std::vector<FossilTableOption::GameSlot>& list){
    QJsonArray ret;
    for (const auto& item : list){
        QJsonObject obj;
        obj.insert(FossilTableOption::JSON_GAME_SLOT, item.game_slot);
        obj.insert(FossilTableOption::JSON_USER_SLOT, item.user_slot);
        obj.insert(FossilTableOption::JSON_FOSSIL, FossilTableOption::FOSSIL_LIST[item.fossil]);
        obj.insert(FossilTableOption::JSON_REVIVES, item.revives);
        ret += std::move(obj);
    }
    return ret;
}

FossilTableOption::FossilTableOption(QString label)
    : m_label(std::move(label))
    , m_default({
        {
            .game_slot = 1,
            .user_slot = 1,
            .fossil = Dracovish,
            .revives = 960,
        },
    })
    , m_current(m_default)
{}

void FossilTableOption::load_default(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_default = parse_fossil_slot(json);
}
void FossilTableOption::load_current(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_current = parse_fossil_slot(json);
}
QJsonValue FossilTableOption::write_default() const{
    return write_fossil_slot(m_default);
}
QJsonValue FossilTableOption::write_current() const{
    return write_fossil_slot(m_current);
}

bool FossilTableOption::is_valid() const{
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
        if (item.revives < 0 || item.revives > 965){
            return false;
        }
    }
    return true;
}
void FossilTableOption::restore_defaults(){
    m_current = m_default;
}


FossilTableOptionUI::FossilTableOptionUI(QWidget& parent, FossilTableOption& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    replace_table();
}
void FossilTableOptionUI::replace_table(){
    QVBoxLayout* layout = static_cast<QVBoxLayout*>(this->layout());
    if (m_table != nullptr){
        layout->removeWidget(m_table);
        delete m_table;
        m_index_table.clear();
        m_table = nullptr;
    }

    m_table = new QTableWidget(this);
    layout->addWidget(m_table);
    m_table->setColumnCount(6);

    QStringList header;
    header << "Game" << "User" << "Fossil" << "Revives" << "" << "";
    m_table->setHorizontalHeaderLabels(header);
    QFont font;
    font.setBold(true);
    m_table->horizontalHeader()->setFont(font);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    size_t rows = m_value.m_current.size();
    m_table->setRowCount((int)(m_value.m_current.size() + 1));
    for (size_t c = 0; c < rows; c++){
        const auto& item = m_value.m_current[c];
        m_index_table.emplace_back(new int);
        add_row((int)c, item, *m_index_table.back());
    }

    QPushButton* button = new QPushButton(m_table);
    button->setText("Add Row");
    m_table->setCellWidget((int)m_value.m_current.size(), 0, button);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            int index = (int)m_index_table.size();

            //  Update data vector.
            m_value.m_current.emplace_back(
                FossilTableOption::GameSlot{1, 1, FossilTableOption::Dracovish, 960}
            );
            const auto& item = m_value.m_current.back();

            //  Update index vector.
            m_index_table.emplace_back(new int);

            //  Update UI.
            m_table->insertRow(index);
            add_row(index, item, *m_index_table.back());
        }
    );
}
void FossilTableOptionUI::add_row(int row, const FossilTableOption::GameSlot& game, int& index_ref){
    index_ref = row;
    m_table->setCellWidget(row, 0, make_game_slot_box(*m_table, index_ref, game.game_slot - 1));
    m_table->setCellWidget(row, 1, make_user_slot_box(*m_table, index_ref, game.user_slot - 1));
    m_table->setCellWidget(row, 2, make_fossil_slot_box(*m_table, index_ref, game.fossil));
    m_table->setCellWidget(row, 3, make_revives_slot_box(*m_table, index_ref, game.revives));
    m_table->setCellWidget(row, 4, make_insert_button(*m_table, index_ref));
    m_table->setCellWidget(row, 5, make_remove_button(*m_table, index_ref));
}
QComboBox* FossilTableOptionUI::make_game_slot_box(QWidget& parent, int& row, int slot){
    QComboBox* box = new QComboBox(&parent);
    box->addItem("Slot 1");
    box->addItem("Slot 2");
    box->setCurrentIndex(slot);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].game_slot = index + 1;
        }
    );
    return box;
}
QComboBox* FossilTableOptionUI::make_user_slot_box(QWidget& parent, int& row, int slot){
    QComboBox* box = new QComboBox(&parent);
    box->addItem("User 1");
    box->addItem("User 2");
    box->addItem("User 3");
    box->addItem("User 4");
    box->addItem("User 5");
    box->addItem("User 6");
    box->addItem("User 7");
    box->addItem("User 8");
    box->setCurrentIndex(slot);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].user_slot = index + 1;
        }
    );
    return box;
}
QComboBox* FossilTableOptionUI::make_fossil_slot_box(QWidget& parent, int& row, FossilTableOption::Fossil fossil){
    QComboBox* box = new QComboBox(&parent);
    box->addItem(FossilTableOption::FOSSIL_LIST[0]);
    box->addItem(FossilTableOption::FOSSIL_LIST[1]);
    box->addItem(FossilTableOption::FOSSIL_LIST[2]);
    box->addItem(FossilTableOption::FOSSIL_LIST[3]);
    box->setCurrentIndex(fossil);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].fossil = (FossilTableOption::Fossil)index;
        }
    );
    return box;
}
QLineEdit* FossilTableOptionUI::make_revives_slot_box(QWidget& parent, int& row, int revives){
    QLineEdit* box = new QLineEdit(&parent);
    QIntValidator* validator = new QIntValidator(0, 965, box);
    box->setValidator(validator);
    box->setText(QString::number(revives));
    box->setMaxLength(3);
    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
    connect(
        box, &QLineEdit::textChanged,
        this, [&, box](const QString& text){
            int revives = text.toInt();
            int fixed = revives;
            fixed = std::max(fixed, 0);
            fixed = std::min(fixed, 965);
            if (revives != fixed){
                box->setText(QString::number(fixed));
            }
            m_value.m_current[row].revives = fixed;
        }
    );
    return box;
}
QPushButton* FossilTableOptionUI::make_insert_button(QWidget& parent, int& row){
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
            int index = row;
            FossilTableOption::GameSlot item{1, 1, FossilTableOption::Dracovish, 960};

            //  Update data vector.
            m_value.m_current.insert(m_value.m_current.begin() + index, item);

            //  Update index vector.
            m_index_table.insert(m_index_table.begin() + index, std::unique_ptr<int>(new int(index)));
            int& new_row = *m_index_table[index];
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = (int)c;
            }

            //  Update UI.
            m_table->insertRow(index);
            add_row(index, item, new_row);
        }
    );
    return button;
}
QPushButton* FossilTableOptionUI::make_remove_button(QWidget& parent, int& row){
    QPushButton* button = new QPushButton(&parent);
    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");
    button->setMaximumWidth(60);
    connect(
        button, &QPushButton::clicked,
        this, [&](bool){
            int index = row;

            //  Update data vector.
            m_index_table.erase(m_index_table.begin() + index);

            //  Update index vector.
            for (size_t c = 0; c < m_index_table.size(); c++){
                *m_index_table[c] = (int)c;
            }
            m_value.m_current.erase(m_value.m_current.begin() + index);

            //  Update UI.
            m_table->removeRow(index);
        }
    );
    return button;
}


void FossilTableOptionUI::restore_defaults(){
    m_value.restore_defaults();
    replace_table();
}










}









