/*  Collapsible Group Box
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CollapsibleGroupBox_H
#define PokemonAutomation_CollapsibleGroupBox_H

#include <QWidget>

class QString;
class QAbstractButton;

namespace PokemonAutomation{

class CollapsibleGroupBox : public QWidget{
public:
    //  Horizontal sections collapse to a narrow header with a vertical title.
    CollapsibleGroupBox(
        QWidget& parent, const QString& title, bool expanded = true,
        Qt::Orientation orientation = Qt::Vertical
    );

    QWidget* widget();
    void set_widget(QWidget* widget);

private:
    void set_expanded(bool expanded);

    QAbstractButton* m_header;
    QWidget* m_content;
    QWidget* m_widget;
    const Qt::Orientation m_orientation;
};

}
#endif
