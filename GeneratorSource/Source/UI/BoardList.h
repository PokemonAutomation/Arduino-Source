/*  Board List
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_BoardList_H
#define PokemonAutomation_BoardList_H

#include <QComboBox>

namespace PokemonAutomation{


class BoardList : public QComboBox{
public:
    BoardList(QWidget& parent);
    const std::string& get_board() const;
private:
};


}
#endif
