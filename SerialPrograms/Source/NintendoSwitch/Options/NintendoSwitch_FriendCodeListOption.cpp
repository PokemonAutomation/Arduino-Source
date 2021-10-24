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
#include "NintendoSwitch_FriendCodeListOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


FriendCodeListOption::FriendCodeListOption(QString label, std::vector<QString> default_lines)
    : m_label(std::move(label))
    , m_default(std::move(default_lines))
    , m_lines(m_default)
{}

void FriendCodeListOption::load_json(const QJsonValue& json){
    m_lines.clear();
    QJsonArray list = json.toArray();
    for (const auto& line : list){
        QString str = line.toString();
        if (str.size() > 0){
            m_lines.emplace_back(str);
        }
    }
}
QJsonValue FriendCodeListOption::to_json() const{
    QJsonArray list;
    for (const QString& line : m_lines){
        list.append(line);
    }
    return list;
}
void FriendCodeListOption::restore_defaults(){
    m_lines = m_default;
}
ConfigOptionUI* FriendCodeListOption::make_ui(QWidget& parent){
    return new FriendCodeListOptionUI(parent, *this);
}



std::vector<uint8_t> FriendCodeListOption::parse(const QString& line){
    std::vector<uint8_t> code;
    for (QChar ch : line){
        if ('0' <= ch && ch <= '9'){
            code.emplace_back((uint8_t)ch.unicode());
        }
    }
    return code;
}
std::vector<std::string> FriendCodeListOption::list() const{
    std::vector<std::string> ret;
    for (const auto& item : m_lines){
        std::vector<uint8_t> line = parse(item);
        if (line.size() != 12){
            continue;
        }
        std::string str = "SW";
        for (size_t c = 0; c < 12; c++){
            if (c % 4 == 0){
                str += "-";
            }
            str += line[c];
        }
        ret.emplace_back(std::move(str));
    }
    return ret;
}



class FriendCodeListOptionUI::Box : public QTextEdit{
public:
    Box(FriendCodeListOptionUI& parent)
        : QTextEdit(&parent)
        , m_parent(parent)
    {
        this->setAcceptRichText(false);
        this->setFocusPolicy(Qt::StrongFocus);
    }

    void redraw(){
        this->clear();
        for (const QString& line : m_parent.m_value.m_lines){
            if (FriendCodeListOption::parse(line).size() == 12){
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
    FriendCodeListOptionUI& m_parent;
};

FriendCodeListOptionUI::FriendCodeListOptionUI(QWidget& parent, FriendCodeListOption& value)
    : QWidget(&parent)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(value.m_label, this));
    m_box = new Box(*this);
    layout->addWidget(m_box);

    m_box->redraw();
}

void FriendCodeListOptionUI::restore_defaults(){
    m_value.restore_defaults();
    m_box->redraw();
}



}
}









