/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "Common/Qt/QtJsonTools.h"
#include "FriendCodeList.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


std::vector<uint8_t> FriendCodeList::parse(const QString& line){
    std::vector<uint8_t> code;
    for (QChar ch : line){
        if ('0' <= ch && ch <= '9'){
            code.emplace_back((uint8_t)ch.unicode());
        }
    }
    return code;
}

FriendCodeList::FriendCodeList()
    : ConfigOption("<b>Friend Codes:</b> One per line only. Invalid characters are ignored.")
    , m_lines{
        "SW-1234-5678-9012",
        "123456789012",
    }
{}

void FriendCodeList::load_json(const QJsonValue& json){
    m_lines.clear();
    QJsonArray list = json.toArray();
    for (const auto& line : list){
        QString str = line.toString();
        if (str.size() > 0){
            m_lines.emplace_back(str);
        }
    }
}
QJsonValue FriendCodeList::to_json() const{
    QJsonArray list;
    for (const QString& line : m_lines){
        list.append(line);
    }
    return list;
}
bool FriendCodeList::is_valid() const{
#if 0
    for (const QString& line : m_lines){
        if (parse(line).size() != 12){
            return false;
        }
    }
#endif
    return true;
}
void FriendCodeList::restore_defaults(){
    m_lines = std::vector<QString>{
        "SW-1234-5678-9012",
        "123456789012",
    };
}
ConfigOptionUI* FriendCodeList::make_ui(QWidget& parent){
    return new FriendCodeListUI(parent, *this);
}



class FriendCodeListUI::Box : public QTextEdit{
public:
    Box(FriendCodeListUI& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
    }

    void redraw(){
        this->clear();
        for (const QString& line : m_parent.m_value.m_lines){
            if (FriendCodeList::parse(line).size() == 12){
                this->append(line);
            }else{
//                this->append("<font color=\"red\">" + line + "</font>");
            }
        }
    }
    void update_backing(){
        m_parent.m_value.m_lines.clear();
        QString body = this->toPlainText();
        QString line;
        for (QChar ch : body){
            if (ch == '\n'){
                m_parent.m_value.m_lines.emplace_back(std::move(line));
                line.clear();
                continue;
            }
            line += ch;
        }
        if (line.size() > 0){
            m_parent.m_value.m_lines.emplace_back(std::move(line));
        }
    }

    virtual void focusOutEvent(QFocusEvent* event) override{
        QTextEdit::focusOutEvent(event);
//        cout << "focusOutEvent()" << endl;
        update_backing();
        redraw();
    }

private:
    FriendCodeListUI& m_parent;
};

FriendCodeListUI::FriendCodeListUI(QWidget& parent, FriendCodeList& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(value.m_label, this));
    m_box = new Box(*this);
    layout->addWidget(m_box);

    m_box->redraw();

#if 0
    connect(
        m_box, &QTextEdit::textChanged,
        this, [=](){
            cout << "textChanged" << endl;
            cout << m_box->hasFocus() << endl;
        }
    );
#endif
}

void FriendCodeListUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->redraw();
}



}
}









