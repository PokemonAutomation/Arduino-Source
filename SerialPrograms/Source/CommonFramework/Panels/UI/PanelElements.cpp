/*  Panel Elements
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "Controllers/ControllerCapability.h"
#include "PanelElements.h"

namespace PokemonAutomation{



CollapsibleGroupBox* make_panel_header(
    QWidget& parent,
    const std::string& display_name,
    const std::string& doc_link,
    const std::string& description
){
    CollapsibleGroupBox* header = new CollapsibleGroupBox(parent, "Current Program");

    QWidget* body = new QWidget(header);
    QVBoxLayout* vbox = new QVBoxLayout(body);
    vbox->setContentsMargins(0, 0, 0, 0);

    std::string name_text = "<b>Name:</b> " + display_name;
    if (!doc_link.empty()){
        std::string path = ONLINE_DOC_URL + doc_link;
        name_text += " (" + make_text_url(path, "online documentation") + ")";
    }
    QLabel* name_label = new QLabel(QString::fromStdString(name_text), header);
    name_label->setWordWrap(true);
    name_label->setTextFormat(Qt::RichText);
    name_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    name_label->setOpenExternalLinks(true);
    vbox->addWidget(name_label);

    std::string description_text = "<b>Description:</b> ";
    description_text += description;
    QLabel* description_label = new QLabel(QString::fromStdString(description), header);
    description_label->setWordWrap(true);
    vbox->addWidget(description_label);

    header->set_widget(body);

    return header;
}
#if 0
CollapsibleGroupBox* make_panel_header(
    QWidget& parent,
    const std::string& display_name,
    const std::string& doc_link,
    const std::string& description,
    FeedbackType feedback
){
    CollapsibleGroupBox* header = make_panel_header(parent, display_name, doc_link, description);
    QLayout* layout = header->widget()->layout();

    QLabel* text = nullptr;
    switch (feedback){
    case FeedbackType::NONE:
        text = new QLabel(
            QString::fromStdString(html_color_text("(This program does not use feedback. It can run without video input.)", COLOR_PURPLE)),
            header
        );
        break;
    case FeedbackType::OPTIONAL_:
        text = new QLabel(
            QString::fromStdString(html_color_text("(This program will use video feedback if it is available. Video input is not required.)", COLOR_PURPLE)),
            header
        );
        break;
    case FeedbackType::REQUIRED:
        text = new QLabel(
            "<font color=\"green\">(This program requires video feedback. Please make sure you choose the correct capture device.)</font>",
            header
        );
        break;
    case FeedbackType::VIDEO_AUDIO:
        text = new QLabel(
            "<font color=\"green\">(This program requires video and audio feedback. Please make sure you choose the correct capture device, as well as the correct audio device.)</font>",
            header
        );
        break;
    }
    text->setWordWrap(true);
    layout->addWidget(text);

    return header;
}
#endif
CollapsibleGroupBox* make_panel_header(
    QWidget& parent,
    const std::string& display_name,
    const std::string& doc_link,
    const std::string& description,
    const ControllerRequirements& requirements,
    FasterIfTickPrecise faster_if_tick_precise
){
    CollapsibleGroupBox* header = make_panel_header(parent, display_name, doc_link, description);
    QLayout* layout = header->widget()->layout();

    std::string text;
    do{
        if (requirements.contains(ControllerFeature::NintendoSwitch_DateSkip)){
            text = html_color_text("(This program requires advanced RPCs. It requires Serial PABotBase.)", COLOR_RED);
            break;
        }
        if (requirements.contains(ControllerFeature::TickPrecise)){
            text = html_color_text("(This program requires a tick-precise controller.)", COLOR_PURPLE);
            break;
        }

        switch (faster_if_tick_precise){
        case PokemonAutomation::FasterIfTickPrecise::MUCH_FASTER:
            text = html_color_text(
                "(This program does not have any special controller requirements. "
                "However, it is strongly recommended to use a tick-precise controller as the program will run much faster and/or more reliably.)",
                COLOR_DARKGREEN
            );
            break;
        case PokemonAutomation::FasterIfTickPrecise::FASTER:
            text = html_color_text(
                "(This program does not have any special controller requirements. "
                "However, it runs faster if the controller is tick-precise.)",
                COLOR_DARKGREEN
            );
            break;
        case PokemonAutomation::FasterIfTickPrecise::NOT_FASTER:
            text = html_color_text("(This program does not have any special controller requirements.)", COLOR_BLUE);
            break;
        }

    }while (false);

    QLabel* label = new QLabel(QString::fromStdString(text), header);
    label->setWordWrap(true);
    layout->addWidget(label);

    return header;
}



StatsBar::StatsBar(QWidget& parent)
    : QLabel(&parent)
{
    this->setWordWrap(true);
    this->setVisible(false);
    this->setAlignment(Qt::AlignCenter);
//    this->setText("<b>Encounters: 1,267 - Corrections: 0 - Star Shinies: 1 - Square Shinies: 0</b>");
    QFont font = this->font();
    font.setPointSize(10);
    this->setFont(font);
}
void StatsBar::set_stats(std::string current_stats, std::string historical_stats){
    if (current_stats.empty() && historical_stats.empty()){
        this->setText("");
        this->setVisible(false);
        return;
    }

    if (!current_stats.empty() && historical_stats.empty()){
        this->setText(QString::fromStdString(current_stats));
        this->setVisible(true);
        return;
    }

    if (current_stats.empty() && !historical_stats.empty()){
        this->setText(QString::fromStdString("<b>Past Runs</b> - " + historical_stats));
        this->setVisible(true);
        return;
    }

    std::string str;
    str += "<b>Current Run</b> - " + current_stats;
    str += "<br>";
    str += "<b>Past Totals</b> - " + historical_stats;

    this->setText(QString::fromStdString(str));
    this->setVisible(true);
}



RunnablePanelActionBar::RunnablePanelActionBar(QWidget& parent, ProgramState initial_state)
    : QGroupBox("Actions", &parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

//    layout->addWidget(new QPushButton("Start Raid Now!", this));

    QHBoxLayout* action_layout = new QHBoxLayout();
    layout->addLayout(action_layout);
//    action_layout->setContentsMargins(0, 0, 0, 0);

    {
        m_start_button = new QPushButton(this);
        QFont font = m_start_button->font();
        font.setPointSize(16);
        m_start_button->setFont(font);
        action_layout->addWidget(m_start_button, 2);
    }
    {
        m_default_button = new QPushButton("Restore Defaults", this);
        QFont font = m_default_button->font();
        font.setPointSize(16);
        m_default_button->setFont(font);
        action_layout->addWidget(m_default_button, 1);
    }

    set_state(initial_state);

    connect(
        m_start_button, &QPushButton::clicked,
        this, [this](bool){ emit start_clicked(m_last_known_state); }
    );
    connect(
        m_default_button, &QPushButton::clicked,
        this, [this](bool){
            QMessageBox::StandardButton button = QMessageBox::question(
                nullptr,
                "Restore Defaults",
                "Are you sure you wish to restore settings back to defaults? This will wipe the current settings.",
                QMessageBox::Ok | QMessageBox::Cancel
            );
            if (button == QMessageBox::Ok){
                emit defaults_clicked();
            }
        }
    );
}
void RunnablePanelActionBar::set_state(ProgramState state){
    m_last_known_state = state;
    switch (state){
    case ProgramState::NOT_READY:
        m_start_button->setText("Loading...");
        m_start_button->setEnabled(false);
        m_default_button->setEnabled(false);
        break;
    case ProgramState::STOPPED:
        m_start_button->setText("Start Program...");
        m_start_button->setEnabled(true);
        m_default_button->setEnabled(true);
        break;
    case ProgramState::RUNNING:
        m_start_button->setText("Stop Program...");
        m_start_button->setEnabled(true);
        m_default_button->setEnabled(false);
        break;
    case ProgramState::STOPPING:
        m_start_button->setText("Stopping Program...");
        m_start_button->setEnabled(false);
        m_default_button->setEnabled(false);
        break;
    }
}






}
