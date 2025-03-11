/*  Collapsible Group Box
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CollapsibleGroupBox_H
#define PokemonAutomation_CollapsibleGroupBox_H

#include <QWidget>
#include <QGroupBox>

class QToolButton;

namespace PokemonAutomation{

class CollapsibleGroupBox : public QGroupBox{
public:
    CollapsibleGroupBox(QWidget& parent, const QString& title, bool expanded = true);

    QWidget* widget();
    void set_widget(QWidget* widget);

private:
    void set_expanded(bool expanded);

    QWidget* m_widget;
};




#if 0
class CollapsibleGroupBox : public QWidget{
public:
    explicit CollapsibleGroupBox(QWidget& parent, const QString& title);
    QWidget* box();

private:
    QToolButton* m_arrow;
    QGroupBox* m_box;

};
#endif



}
#endif
