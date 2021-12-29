/*  Multi-Host Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MultiHostTableBaseOption_H
#define PokemonAutomation_MultiHostTableBaseOption_H

#include "Common/Qt/Options/EditableTable/EditableTableBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiHostSlot : public EditableTableRow{
    static const QString JSON_GAME_SLOT;
    static const QString JSON_USER_SLOT;
    static const QString JSON_SKIPS;
    static const QString JSON_BACKUP_SAVE;
    static const QString JSON_ALWAYS_CATCHABLE;
    static const QString JSON_USE_RAID_CODE;
    static const QString JSON_ACCEPT_FRS;
    static const QString JSON_MOVE_SLOT;
    static const QString JSON_DYNAMAX;
    static const QString JSON_POST_RAID_DELAY;

public:
    MultiHostSlot(bool raid_code_option);

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_game_slot_box  (QWidget& parent);
    QWidget* make_user_slot_box  (QWidget& parent);
    QWidget* make_skips_box      (QWidget& parent);
    QWidget* make_backup_save_box(QWidget& parent);
    QWidget* make_catchable_box  (QWidget& parent);
    QWidget* make_raid_code_box  (QWidget& parent);
    QWidget* make_accept_FRs_box (QWidget& parent);
    QWidget* make_move_slot_box  (QWidget& parent);
    QWidget* make_dynamax_box    (QWidget& parent);
    QWidget* make_delay_box      (QWidget& parent);

private:
    bool m_raid_code_option;
public:
    uint8_t game_slot       = 1;
    uint8_t user_slot       = 1;
    uint8_t skips           = 3;
    bool backup_save        = false;
    bool always_catchable   = true;
    bool use_raid_code      = true;
    bool accept_FRs         = true;
    uint8_t move_slot       = 0;
    bool dynamax            = true;
    QString post_raid_delay = "0 * TICKS_PER_SECOND";
};
class MultiHostSlotOptionFactory : public EditableTableFactory{
public:
    MultiHostSlotOptionFactory(bool raid_code_option);
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
private:
    bool m_raid_code_option;
};





}
}
}
#endif
