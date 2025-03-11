/*  QLineEdit with auto-width sizing.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AutoWidthLineEdit_H
#define PokemonAutomation_AutoWidthLineEdit_H

#include <QLineEdit>

namespace PokemonAutomation{


class AutoWidthLineEdit : public QLineEdit{
public:
    AutoWidthLineEdit(QWidget* parent);

private:
    void resize_to_content();

};

}
#endif
