/*  Path Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include "PathWidget.h"

namespace PokemonAutomation{


template class RegisterConfigWidget<PathCellWidget>;
template class RegisterConfigWidget<PathOptionWidget>;




PathCellWidget::~PathCellWidget(){
    m_value.remove_listener(*this);
}
PathCellWidget::PathCellWidget(QWidget& parent, PathCell& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_line_edit = new QLineEdit(QString::fromStdString(value), this);
    m_line_edit->setPlaceholderText(QString::fromStdString(value.placeholder_text()));
    m_line_edit->setReadOnly(value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
    layout->addWidget(m_line_edit, 1);

    m_browse_button = new QPushButton("Browse...", this);
    m_browse_button->setEnabled(value.lock_mode() != LockMode::READ_ONLY && !m_value.is_locked());
    layout->addWidget(m_browse_button);

    connect(
        m_line_edit, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(m_line_edit->text().toStdString());
        }
    );

    connect(
        m_browse_button, &QPushButton::clicked,
        this, [this](){
            browse_file();
        }
    );

    m_value.add_listener(*this);
}

void PathCellWidget::browse_file(){
    QString current_path = m_line_edit->text();
    QString file_path = QFileDialog::getOpenFileName(
        this,
        "Select File",
        current_path,
        QString::fromStdString(m_value.filter_string())
    );

    if (!file_path.isEmpty()){
        m_line_edit->setText(file_path);
        m_value.set(file_path.toStdString());
    }
}

void PathCellWidget::update_value(){
    m_line_edit->setText(QString::fromStdString(m_value));
}
void PathCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void PathCellWidget::on_config_visibility_changed(){
    this->setEnabled(true);
    QMetaObject::invokeMethod(this, [this]{
        const ConfigOptionState visibility = m_value.visibility();
        m_line_edit->setReadOnly(visibility != ConfigOptionState::ENABLED ||
            m_value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
        m_browse_button->setEnabled(visibility == ConfigOptionState::ENABLED &&
            m_value.lock_mode() != LockMode::READ_ONLY && !m_value.is_locked());
        switch (visibility){
        case ConfigOptionState::ENABLED:
        case ConfigOptionState::DISABLED:
            this->setVisible(true);
            break;
        case ConfigOptionState::HIDDEN:
            this->setVisible(false);
            break;
        }
    }, Qt::QueuedConnection);
}




PathOptionWidget::~PathOptionWidget(){
    m_value.remove_listener(*this);
}
PathOptionWidget::PathOptionWidget(QWidget& parent, PathOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);

    m_line_edit = new QLineEdit(QString::fromStdString(m_value), this);
    m_line_edit->setPlaceholderText(QString::fromStdString(value.placeholder_text()));
    m_line_edit->setReadOnly(value.lock_mode() == LockMode::READ_ONLY);
    layout->addWidget(m_line_edit, 1);

    m_browse_button = new QPushButton("Browse...", this);
    m_browse_button->setEnabled(value.lock_mode() != LockMode::READ_ONLY);
    layout->addWidget(m_browse_button);

    connect(
        m_line_edit, &QLineEdit::editingFinished,
        this, [this](){
            m_value.set(m_line_edit->text().toStdString());
        }
    );

    connect(
        m_browse_button, &QPushButton::clicked,
        this, [this](){
            browse_file();
        }
    );

    m_value.add_listener(*this);
}

void PathOptionWidget::browse_file(){
    QString current_path = m_line_edit->text();
    QString file_path = QFileDialog::getOpenFileName(
        this,
        "Select File",
        current_path,
        QString::fromStdString(m_value.filter_string())
    );

    if (!file_path.isEmpty()){
        m_line_edit->setText(file_path);
        m_value.set(file_path.toStdString());
    }
}

void PathOptionWidget::update_value(){
    m_line_edit->setText(QString::fromStdString(m_value));
}
void PathOptionWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(m_line_edit, [this]{
        update_value();
    }, Qt::QueuedConnection);
}
void PathOptionWidget::on_config_visibility_changed(){
    this->setEnabled(true);
    QMetaObject::invokeMethod(this, [this]{
        const ConfigOptionState visibility = m_value.visibility();
        m_line_edit->setReadOnly(visibility != ConfigOptionState::ENABLED ||
            m_value.lock_mode() == LockMode::READ_ONLY || m_value.is_locked());
        m_browse_button->setEnabled(visibility == ConfigOptionState::ENABLED &&
            m_value.lock_mode() != LockMode::READ_ONLY && !m_value.is_locked());
        switch (visibility){
        case ConfigOptionState::ENABLED:
        case ConfigOptionState::DISABLED:
            this->setVisible(true);
            break;
        case ConfigOptionState::HIDDEN:
            this->setVisible(false);
            break;
        }
    }, Qt::QueuedConnection);
}





}
