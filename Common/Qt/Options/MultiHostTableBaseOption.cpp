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
#include <QLineEdit>
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "MultiHostTableBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const QString MultiHostSlot::JSON_GAME_SLOT         = "game_slot";
const QString MultiHostSlot::JSON_USER_SLOT         = "user_slot";
const QString MultiHostSlot::JSON_SKIPS             = "skips";
const QString MultiHostSlot::JSON_BACKUP_SAVE       = "backup_save";
const QString MultiHostSlot::JSON_ALWAYS_CATCHABLE  = "always_catchable";
const QString MultiHostSlot::JSON_USE_RAID_CODE     = "use_raid_code";
const QString MultiHostSlot::JSON_ACCEPT_FRS        = "accept_FRs";
const QString MultiHostSlot::JSON_MOVE_SLOT         = "move_slot";
const QString MultiHostSlot::JSON_DYNAMAX           = "dynamax";
const QString MultiHostSlot::JSON_POST_RAID_DELAY   = "post_raid_delay";



MultiHostSlot::MultiHostSlot(bool raid_code_option)
    : m_raid_code_option(raid_code_option)
{}
void MultiHostSlot::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    json_get_int(game_slot, obj, JSON_GAME_SLOT, 1, 2);
    json_get_int(user_slot, obj, JSON_USER_SLOT, 1, 8);
    json_get_int(skips, obj, JSON_SKIPS, 0, 7);

    json_get_bool(backup_save, obj, JSON_BACKUP_SAVE);
    json_get_bool(always_catchable, obj, JSON_ALWAYS_CATCHABLE);
    json_get_bool(use_raid_code, obj, JSON_USE_RAID_CODE);
    json_get_bool(accept_FRs, obj, JSON_ACCEPT_FRS);

    json_get_int(move_slot, obj, JSON_MOVE_SLOT, 0, 4);
    json_get_bool(dynamax, obj, JSON_DYNAMAX);

    json_get_string(post_raid_delay, obj, MultiHostSlot::JSON_POST_RAID_DELAY);
}
QJsonValue MultiHostSlot::to_json() const{
    QJsonObject obj;
    obj.insert(JSON_GAME_SLOT, game_slot);
    obj.insert(JSON_USER_SLOT, user_slot);
    obj.insert(JSON_SKIPS, skips);
    obj.insert(JSON_BACKUP_SAVE, backup_save);
    obj.insert(JSON_ALWAYS_CATCHABLE, always_catchable);
    obj.insert(JSON_ACCEPT_FRS, accept_FRs);
    obj.insert(JSON_USE_RAID_CODE, use_raid_code);
    obj.insert(JSON_MOVE_SLOT, move_slot);
    obj.insert(JSON_DYNAMAX, dynamax);
    obj.insert(JSON_POST_RAID_DELAY, post_raid_delay);
    return obj;
}
std::unique_ptr<EditableTableRow> MultiHostSlot::clone() const{
    return std::unique_ptr<EditableTableRow>(new MultiHostSlot(*this));
}
std::vector<QWidget*> MultiHostSlot::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_game_slot_box(parent));
    widgets.emplace_back(make_user_slot_box(parent));
    widgets.emplace_back(make_skips_box(parent));
    widgets.emplace_back(make_backup_save_box(parent));
    widgets.emplace_back(make_catchable_box(parent));
    if (m_raid_code_option){
        widgets.emplace_back(make_raid_code_box(parent));
    }
    widgets.emplace_back(make_accept_FRs_box(parent));
    widgets.emplace_back(make_move_slot_box(parent));
    widgets.emplace_back(make_dynamax_box(parent));
    widgets.emplace_back(make_delay_box(parent));
    return widgets;
}
QWidget* MultiHostSlot::make_game_slot_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem("Game 1");
    box->addItem("Game 2");
    box->setCurrentIndex((int)game_slot - 1);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            game_slot = (uint8_t)(index + 1);
        }
    );
    return box;
}
QWidget* MultiHostSlot::make_user_slot_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem("User 1");
    box->addItem("User 2");
    box->addItem("User 3");
    box->addItem("User 4");
    box->addItem("User 5");
    box->addItem("User 6");
    box->addItem("User 7");
    box->addItem("User 8");
    box->setCurrentIndex((int)user_slot - 1);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            user_slot = (uint8_t)(index + 1);
        }
    );
    return box;
}
QWidget* MultiHostSlot::make_skips_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    QIntValidator* validator = new QIntValidator(0, 7, box);
    box->setValidator(validator);
    box->setText(QString::number(skips));
    box->setMaxLength(3);
    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&, box](const QString& text){
            int revives = text.toInt();
            int fixed = revives;
            fixed = std::max(fixed, 0);
            fixed = std::min(fixed, 7);
            if (revives != fixed){
                box->setText(QString::number(fixed));
            }
            skips = fixed;
        }
    );
    return box;
}
QWidget* MultiHostSlot::make_backup_save_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(backup_save);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            backup_save = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostSlot::make_catchable_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(always_catchable);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            always_catchable = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostSlot::make_raid_code_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(use_raid_code);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            use_raid_code = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostSlot::make_accept_FRs_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(accept_FRs);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            accept_FRs = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostSlot::make_move_slot_box(QWidget& parent){
    QComboBox* box = new NoWheelComboBox(&parent);
    box->addItem("None");
    box->addItem("Slot 1");
    box->addItem("Slot 2");
    box->addItem("Slot 3");
    box->addItem("Slot 4");
    box->setCurrentIndex((int)move_slot);
    box->connect(
        box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        box, [&](int index){
            move_slot = index;
        }
    );
    return box;
}
QWidget* MultiHostSlot::make_dynamax_box(QWidget& parent){
    QWidget* widget = new QWidget(&parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    QCheckBox* box = new QCheckBox(&parent);
    layout->addWidget(box);
    box->setChecked(dynamax);
    box->connect(
        box, &QCheckBox::stateChanged,
        box, [&, box](int){
            dynamax = box->isChecked();
        }
    );
    return widget;
}
QWidget* MultiHostSlot::make_delay_box(QWidget& parent){
    QLineEdit* box = new QLineEdit(&parent);
    box->setText(post_raid_delay);
//    box->setMaxLength(3);
//    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
//    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            post_raid_delay = text;
//            QSize size = box->document()->size().toSize();
//            box->setFixedWidth(size);
        }
    );
    return box;
}


MultiHostSlotOptionFactory::MultiHostSlotOptionFactory(bool raid_code_option)
    : m_raid_code_option(raid_code_option)
{}
QStringList MultiHostSlotOptionFactory::make_header() const{
    QStringList list;
    list << "Game" << "User" << "Skips" << "Backup Save" << "Always Catchable";
    if (m_raid_code_option){
        list << "Use Raid Code";
    }
    list << "Accept FRs" << "1st Move" << "Dynamax" << "Post Raid Delay";
    return list;
}
std::unique_ptr<EditableTableRow> MultiHostSlotOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new MultiHostSlot(m_raid_code_option));
}






}
}
}










