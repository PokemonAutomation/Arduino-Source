/*  Friend Code List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "NintendoSwitch_FriendCodeListOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class FriendCodeListWidget : public QWidget, public ConfigWidget{
public:
    FriendCodeListWidget(QWidget& parent, FriendCodeListOption& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    class Box;

    FriendCodeListOption& m_value;
    Box* m_box;
};




FriendCodeListOption::FriendCodeListOption(std::string label, std::vector<std::string> default_lines)
    : m_label(std::move(label))
    , m_default(std::move(default_lines))
    , m_lines(m_default)
{}

void FriendCodeListOption::load_json(const JsonValue& json){
    const JsonArray* list = json.get_array();
    if (list == nullptr){
        return;
    }
    m_lines.clear();
    for (const auto& line : *list){
        const std::string* str = line.get_string();
        if (str == nullptr || str->empty()){
            continue;
        }
        m_lines.emplace_back(*str);
    }
}
JsonValue FriendCodeListOption::to_json() const{
    JsonArray list;
    for (const std::string& line : m_lines){
        list.push_back(line);
    }
    return list;
}
void FriendCodeListOption::restore_defaults(){
    m_lines = m_default;
}
ConfigWidget* FriendCodeListOption::make_ui(QWidget& parent){
    return new FriendCodeListWidget(parent, *this);
}



std::vector<uint8_t> FriendCodeListOption::parse(const std::string& line){
    std::vector<uint8_t> code;
    for (char ch : line){
        if ('0' <= ch && ch <= '9'){
            code.emplace_back(ch);
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
        this->clear();
        for (const std::string& line : m_parent.m_value.m_lines){
            if (FriendCodeListOption::parse(line).size() == 12){
                this->append(QString::fromStdString(line));
            }else{
//                this->append("<font color=\"red\">" + line + "</font>");
            }
        }
    }
    void update_backing(){
        m_parent.m_value.m_lines.clear();
        std::string body = this->toPlainText().toStdString();
        std::string line;
        for (char ch : body){
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
    FriendCodeListWidget& m_parent;
};

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
}

void FriendCodeListWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void FriendCodeListWidget::update_ui(){
    m_box->redraw();
}



}
}









