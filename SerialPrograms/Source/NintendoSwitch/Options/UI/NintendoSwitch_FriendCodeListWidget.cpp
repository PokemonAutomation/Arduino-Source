/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "NintendoSwitch_FriendCodeListWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


ConfigWidget* FriendCodeListOption::make_QtWidget(QWidget& parent){
    return new FriendCodeListWidget(parent, *this);
}



class FriendCodeListWidget::Box : public QTextEdit{
public:
    Box(FriendCodeListWidget& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
    }

    void redraw(){
        std::vector<std::string> lines = m_parent.m_value.lines();
        this->clear();
        for (const std::string& line : lines){
            if (FriendCodeListOption::parse(line).size() == 12){
                this->append(QString::fromStdString(line));
            }else{
//                this->append("<font color=\"red\">" + line + "</font>");
            }
        }
    }

    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
//        cout << "focusOutEvent()" << endl;
        m_parent.m_value.set(this->toPlainText().toStdString());
        redraw();
    }

private:
    FriendCodeListWidget& m_parent;
};



FriendCodeListWidget::~FriendCodeListWidget(){
    m_value.remove_listener(*this);
}
FriendCodeListWidget::FriendCodeListWidget(QWidget& parent, FriendCodeListOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(QString::fromStdString(value.m_label), this);
    label->setWordWrap(true);
    layout->addWidget(label);
    m_box = new Box(*this);
    layout->addWidget(m_box);

    m_box->redraw();

    m_value.add_listener(*this);
}

void FriendCodeListWidget::update_value(){
    m_box->redraw();
}
void FriendCodeListWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_box, [this]{
        update_value();
    }, Qt::QueuedConnection);
}





}
}
