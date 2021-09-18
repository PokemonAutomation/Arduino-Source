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
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/QtJsonTools.h"
#include "FossilTableOptionBase.h"

namespace PokemonAutomation{


const QString FossilTableOptionBase::JSON_GAME_SLOT = "game_slot";
const QString FossilTableOptionBase::JSON_USER_SLOT = "user_slot";
const QString FossilTableOptionBase::JSON_FOSSIL    = "fossil";
const QString FossilTableOptionBase::JSON_REVIVES   = "revives";

const std::vector<QString> FossilTableOptionBase::FOSSIL_LIST{
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

std::vector<FossilTableOptionBase::GameSlot> parse_fossil_slot(const QJsonValue& json){
    std::vector<FossilTableOptionBase::GameSlot> list;
    for (const auto item : json.toArray()){
        QJsonObject line = item.toObject();
        FossilTableOptionBase::GameSlot slot;

        json_get_int(slot.game_slot, line, FossilTableOptionBase::JSON_GAME_SLOT, 1, 2);
        json_get_int(slot.user_slot, line, FossilTableOptionBase::JSON_USER_SLOT, 1, 8);

        QString str;
        json_get_string(str, line, FossilTableOptionBase::JSON_FOSSIL);
        auto iter = FOSSIL_MAP.find(str);
        if (iter != FOSSIL_MAP.end()){
            slot.fossil = (FossilTableOptionBase::Fossil)iter->second;
        }

        json_get_int(slot.revives, line, FossilTableOptionBase::JSON_REVIVES, 0, 965);

        list.emplace_back(slot);
    }
    return list;
}
QJsonArray write_fossil_slot(const std::vector<FossilTableOptionBase::GameSlot>& list){
    QJsonArray ret;
    for (const auto& item : list){
        QJsonObject obj;
        obj.insert(FossilTableOptionBase::JSON_GAME_SLOT, item.game_slot);
        obj.insert(FossilTableOptionBase::JSON_USER_SLOT, item.user_slot);
        obj.insert(FossilTableOptionBase::JSON_FOSSIL, FossilTableOptionBase::FOSSIL_LIST[item.fossil]);
        obj.insert(FossilTableOptionBase::JSON_REVIVES, item.revives);
        ret += std::move(obj);
    }
    return ret;
}

FossilTableOptionBase::FossilTableOptionBase(QString label)
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

void FossilTableOptionBase::load_default(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_default = parse_fossil_slot(json);
}
void FossilTableOptionBase::load_current(const QJsonValue& json){
    if (json.isNull()){
        return;
    }
    m_current = parse_fossil_slot(json);
}
QJsonValue FossilTableOptionBase::write_default() const{
    return write_fossil_slot(m_default);
}
QJsonValue FossilTableOptionBase::write_current() const{
    return write_fossil_slot(m_current);
}

bool FossilTableOptionBase::is_valid() const{
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
void FossilTableOptionBase::restore_defaults(){
    m_current = m_default;
}


FossilTableOptionBaseUI::FossilTableOptionBaseUI(QWidget& parent, FossilTableOptionBase& value)
    : QWidget(&parent)
    , m_value(value)
    , m_table(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* text = new QLabel(value.m_label, this);
    layout->addWidget(text);
    replace_table();
}
void FossilTableOptionBaseUI::replace_table(){
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
                FossilTableOptionBase::GameSlot{1, 1, FossilTableOptionBase::Dracovish, 960}
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
void FossilTableOptionBaseUI::add_row(int row, const FossilTableOptionBase::GameSlot& game, int& index_ref){
    index_ref = row;
    m_table->setCellWidget(row, 0, make_game_slot_box(*m_table, index_ref, game.game_slot - 1));
    m_table->setCellWidget(row, 1, make_user_slot_box(*m_table, index_ref, game.user_slot - 1));
    m_table->setCellWidget(row, 2, make_fossil_slot_box(*m_table, index_ref, game.fossil));
    m_table->setCellWidget(row, 3, make_revives_slot_box(*m_table, index_ref, game.revives));
    m_table->setCellWidget(row, 4, make_insert_button(*m_table, index_ref));
    m_table->setCellWidget(row, 5, make_remove_button(*m_table, index_ref));
    m_table->resizeColumnsToContents();
}
QComboBox* FossilTableOptionBaseUI::make_game_slot_box(QWidget& parent, int& row, int slot){
    QComboBox* box = new NoWheelComboBox(&parent);
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
QComboBox* FossilTableOptionBaseUI::make_user_slot_box(QWidget& parent, int& row, int slot){
    QComboBox* box = new NoWheelComboBox(&parent);
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
QComboBox* FossilTableOptionBaseUI::make_fossil_slot_box(QWidget& parent, int& row, FossilTableOptionBase::Fossil fossil){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem(FossilTableOptionBase::FOSSIL_LIST[0]);
    box->addItem(FossilTableOptionBase::FOSSIL_LIST[1]);
    box->addItem(FossilTableOptionBase::FOSSIL_LIST[2]);
    box->addItem(FossilTableOptionBase::FOSSIL_LIST[3]);
    box->setCurrentIndex(fossil);
    connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [&](int index){
            m_value.m_current[row].fossil = (FossilTableOptionBase::Fossil)index;
        }
    );
    return box;
}
QLineEdit* FossilTableOptionBaseUI::make_revives_slot_box(QWidget& parent, int& row, int revives){
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
QPushButton* FossilTableOptionBaseUI::make_insert_button(QWidget& parent, int& row){
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
            FossilTableOptionBase::GameSlot item{1, 1, FossilTableOptionBase::Dracovish, 960};

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
QPushButton* FossilTableOptionBaseUI::make_remove_button(QWidget& parent, int& row){
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


void FossilTableOptionBaseUI::restore_defaults(){
    m_value.restore_defaults();
    replace_table();
}










}









