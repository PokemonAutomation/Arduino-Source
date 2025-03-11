/*  QLineEdit with auto-width sizing.
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Compiler.h"
#include "AutoWidthLineEdit.h"

namespace PokemonAutomation{


AutoWidthLineEdit::AutoWidthLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    connect(
        this, &QLineEdit::textChanged,
        this, [this](const QString& line){
            resize_to_content();
        }
    );
}

void AutoWidthLineEdit::resize_to_content(){
    QString text = this->text();
    QFontMetrics fm(this->font());
    int pixelsWide = fm.boundingRect(text).width();
    this->setFixedWidth(pixelsWide);
    adjustSize();
}


}
