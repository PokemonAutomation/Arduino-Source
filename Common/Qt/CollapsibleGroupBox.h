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
    CollapsibleGroupBox(QWidget& parent, const QString& title, bool expanded = true);

    QWidget* widget();
    void set_widget(QWidget* widget);

private:
    void set_expanded(bool expanded);

    QAbstractButton* m_header;
    QWidget* m_content;
    QWidget* m_widget;
};

}
#endif
