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
#include "CommonFramework/Globals.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "LanguageOCRWidget.h"

namespace PokemonAutomation{
namespace OCR{



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
    QLabel* text = new QLabel(QString::fromStdString(m_value.m_label), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    QVBoxLayout* vbox = new QVBoxLayout();
    layout->addLayout(vbox, 1);
    m_box = new NoWheelComboBox(&parent);

    for (const auto& item : m_value.m_case_list){
        m_box->addItem(QString::fromStdString(language_data(item.first).name));
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
    m_box->setCurrentIndex((int)(Language)m_value);
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
//            m_value.m_current = index;
            m_value.set((Language)index);

//            const LanguageData& data = language_data(m_value);
//            cout << "index = " << index << ", " << data.code << endl;

//            update_status();
        }
    );
}

void LanguageOCRWidget::update_status(){
    const std::pair<Language, bool>& item = m_value.m_case_list[(size_t)(Language)m_value];
    const LanguageData& language = language_data(m_value);
    if (item.second){
        m_status->setVisible(false);
    }else{
        m_status->setText(
            QString::fromStdString(
                "<font color=\"red\">No text recognition data found for " + language.name + ".</font>\n" +
                "<a href=\"" + PROJECT_GITHUB_URL + "Packages/blob/master/SerialPrograms/Resources/Tesseract/\">Download from here.</a>"
            )
        );
        m_status->setVisible(true);
    }
}

void LanguageOCRWidget::restore_defaults(){
    m_value.restore_defaults();
}
void LanguageOCRWidget::update_ui(){
    m_box->setCurrentIndex((int)(Language)m_value);
    update_status();
}
void LanguageOCRWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update_ui();
    }, Qt::QueuedConnection);
}





}
}
