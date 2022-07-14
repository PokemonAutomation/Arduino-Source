/*  Multi-Host Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QCheckBox>
#include <QScrollBar>
#include <QLineEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/ExpressionEvaluator.h"
#include "MultiHostTableBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::string MultiHostSlot::JSON_GAME_SLOT         = "game_slot";
const std::string MultiHostSlot::JSON_USER_SLOT         = "user_slot";
const std::string MultiHostSlot::JSON_SKIPS             = "skips";
const std::string MultiHostSlot::JSON_BACKUP_SAVE       = "backup_save";
const std::string MultiHostSlot::JSON_ALWAYS_CATCHABLE  = "always_catchable";
const std::string MultiHostSlot::JSON_USE_RAID_CODE     = "use_raid_code";
const std::string MultiHostSlot::JSON_ACCEPT_FRS        = "accept_FRs";
const std::string MultiHostSlot::JSON_MOVE_SLOT         = "move_slot";
const std::string MultiHostSlot::JSON_DYNAMAX           = "dynamax";
const std::string MultiHostSlot::JSON_POST_RAID_DELAY   = "post_raid_delay";



MultiHostSlot::MultiHostSlot(bool raid_code_option)
    : m_raid_code_option(raid_code_option)
{}
void MultiHostSlot::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_integer(game_slot, JSON_GAME_SLOT, 1, 2);
    obj->read_integer(user_slot, JSON_USER_SLOT, 1, 8);
    obj->read_integer(skips, JSON_SKIPS, 1, 7);

    obj->read_boolean(backup_save, JSON_BACKUP_SAVE);
    obj->read_boolean(always_catchable, JSON_ALWAYS_CATCHABLE);
    obj->read_boolean(use_raid_code, JSON_USE_RAID_CODE);
    obj->read_boolean(accept_FRs, JSON_ACCEPT_FRS);

    obj->read_integer(move_slot, JSON_MOVE_SLOT, 0, 4);
    obj->read_boolean(dynamax, JSON_DYNAMAX);

    std::string str;
    if (obj->read_string(str, MultiHostSlot::JSON_POST_RAID_DELAY)){
        post_raid_delay = str;
    }
}
JsonValue MultiHostSlot::to_json() const{
    JsonObject obj;
    obj[JSON_GAME_SLOT] = game_slot;
    obj[JSON_USER_SLOT] = user_slot;
    obj[JSON_SKIPS] = skips;
    obj[JSON_BACKUP_SAVE] = backup_save;
    obj[JSON_ALWAYS_CATCHABLE] = always_catchable;
    obj[JSON_ACCEPT_FRS] = accept_FRs;
    obj[JSON_USE_RAID_CODE] = use_raid_code;
    obj[JSON_MOVE_SLOT] = move_slot;
    obj[JSON_DYNAMAX] = dynamax;
    obj[JSON_POST_RAID_DELAY] = post_raid_delay;
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
    box->setText(QString::fromStdString(post_raid_delay));
//    box->setMaxLength(3);
//    box->setMaximumWidth(50);
    box->setAlignment(Qt::AlignHCenter);
//    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    box->connect(
        box, &QLineEdit::textChanged,
        box, [&](const QString& text){
            post_raid_delay = text.toStdString();
//            QSize size = box->document()->size().toSize();
//            box->setFixedWidth(size);
        }
    );
    return box;
}


MultiHostSlotOptionFactory::MultiHostSlotOptionFactory(bool raid_code_option)
    : m_raid_code_option(raid_code_option)
{}
std::vector<std::string> MultiHostSlotOptionFactory::make_header() const{
    if (m_raid_code_option){
        return std::vector<std::string>{
            "Game",
            "User",
            "Skips",
            "Backup Save",
            "Always Catchable",
            "Use Raid Code",
            "Accept FRs",
            "1st Move",
            "Dynamax",
            "Post Raid Delay",
        };
    }else{
        return std::vector<std::string>{
            "Game",
            "User",
            "Skips",
            "Backup Save",
            "Always Catchable",
            "Accept FRs",
            "1st Move",
            "Dynamax",
            "Post Raid Delay",
        };
    }
}
std::unique_ptr<EditableTableRow> MultiHostSlotOptionFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new MultiHostSlot(m_raid_code_option));
}






}
}
}










