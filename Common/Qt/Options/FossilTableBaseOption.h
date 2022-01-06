/*  Fossil Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FossilTableBaseOption_H
#define PokemonAutomation_FossilTableBaseOption_H

#include "Common/Qt/Options/EditableTable/EditableTableBaseOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class FossilGame : public EditableTableRow{
    static const QString JSON_GAME_SLOT;
    static const QString JSON_USER_SLOT;
    static const QString JSON_FOSSIL;
    static const QString JSON_REVIVES;

public:
    enum Fossil{
        Dracozolt   =   0,
        Arctozolt   =   1,
        Dracovish   =   2,
        Arctovish   =   3,
    };
    static const std::vector<QString> FOSSIL_LIST;

public:
    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    virtual std::vector<QWidget*> make_widgets(QWidget& parent) override;

private:
    QWidget* make_game_slot_box(QWidget& parent);
    QWidget* make_user_slot_box(QWidget& parent);
    QWidget* make_fossil_slot_box(QWidget& parent);
    QWidget* make_revives_slot_box(QWidget& parent);

public:
    uint8_t game_slot   = 1;
    uint8_t user_slot   = 1;
    Fossil fossil       = Dracovish;
    uint16_t revives    = 960;
};
class FossilGameOptionFactory : public EditableTableFactory{
public:
    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;
};







}
}
}
#endif

