/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "OCR_LanguageOptionOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



class LanguageOCRWidget : public QWidget, public ConfigWidget{
public:
    LanguageOCRWidget(QWidget& parent, LanguageOCR& value);

    virtual void restore_defaults() override;
    virtual void update_ui() override;

private:
    void update_status();

private:
    LanguageOCR& m_value;
    QComboBox* m_box;
    QLabel* m_status;
//    bool m_updating = false;
};




LanguageOCR::LanguageOCR(QString label, const LanguageSet& languages, bool required)
    : m_label(std::move(label))
    , m_default(0)
    , m_current(0)
{
    size_t index = 0;
    if (!required && !languages[Language::None]){
        m_case_list.emplace_back(
            Language::None,
            true
        );
        m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(Language::None),
            std::forward_as_tuple(index)
        );
        index++;
    }
    for (Language language : languages){
        m_case_list.emplace_back(
            language,
            language == Language::None || OCR::language_available(language)
        );
        m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(language),
            std::forward_as_tuple(index)
        );
        index++;
    }
}



void LanguageOCR::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    Language language;
    try{
        language = language_code_to_enum(*str);
    }catch (const InternalProgramError&){
        return;
    }

    auto iter = m_case_map.find(language);
    if (iter != m_case_map.end()){
        m_current = iter->second;
    }
}
JsonValue LanguageOCR::to_json() const{
    return language_data(m_case_list[m_current].first).code;
}

QString LanguageOCR::check_validity() const{
    return m_case_list[m_current].second ? QString() : "Language data is not available.";
}
void LanguageOCR::restore_defaults(){
    m_current = m_default;
}

ConfigWidget* LanguageOCR::make_ui(QWidget& parent){
    return new LanguageOCRWidget(parent, *this);
}



LanguageOCRWidget::LanguageOCRWidget(QWidget& parent, LanguageOCR& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(m_value.m_label, this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    QVBoxLayout* vbox = new QVBoxLayout();
    layout->addLayout(vbox, 1);
    m_box = new NoWheelComboBox(&parent);

    for (const auto& item : m_value.m_case_list){
        m_box->addItem(language_data(item.first).name);
        auto* model = qobject_cast<QStandardItemModel*>(m_box->model());
        if (model == nullptr){
            continue;
        }
        QStandardItem* line_handle = model->item(m_box->count() - 1);
        if (line_handle != nullptr){
//            line_handle->setEnabled(item.second);
            if (!item.second){
                QFont font = line_handle->font();
                font.setStrikeOut(true);
                line_handle->setFont(font);

                QBrush brush = line_handle->foreground();
                brush.setColor(Qt::red);
                line_handle->setForeground(brush);
            }
        }
    }
    m_box->setCurrentIndex((int)m_value.m_current);
    vbox->addWidget(m_box);

    m_status = new QLabel(this);
    m_status->setTextFormat(Qt::RichText);
    m_status->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_status->setOpenExternalLinks(true);
    vbox->addWidget(m_status);

    update_status();

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.m_current = index;

//            const LanguageData& data = language_data(m_value);
//            cout << "index = " << index << ", " << data.code << endl;

            update_status();
        }
    );
}

void LanguageOCRWidget::update_status(){
    const std::pair<Language, bool>& item = m_value.m_case_list[m_value.m_current];
    const LanguageData& data = language_data(m_value);
    if (item.second){
        m_status->setVisible(false);
    }else{
        m_status->setText(
            "<font color=\"red\">No text recognition data found for " + data.name + ".</font>\n" +
            "<a href=\"" + PROJECT_GITHUB_URL + "Packages/blob/master/SerialPrograms/Resources/Tesseract/\">Download from here.</a>"
        );
        m_status->setVisible(true);
    }
}

void LanguageOCRWidget::restore_defaults(){
    m_value.restore_defaults();
    update_ui();
}
void LanguageOCRWidget::update_ui(){
    m_box->setCurrentIndex((int)m_value.m_current);
}






}
}

