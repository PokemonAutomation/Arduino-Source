/*  Resource Download Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SettingsResourceDownloadRow.h"
#include "SettingsResourceDownloadOptions.h"


// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{



void SettingsResourceButton::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void SettingsResourceButton::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}

void SettingsResourceButton::change_text(const std::string& text){
    m_listeners.run_method(&Listener::on_change_text, text);
}


//////////////////////////////////////
// SettingsResourceDownloadButton
//////////////////////////////////////
// SettingsResourceDownloadButton::~SettingsResourceDownloadButton(){}
SettingsResourceDownloadButton::SettingsResourceDownloadButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}

void SettingsResourceDownloadButton::set_enabled(bool enabled){ 
    m_enabled = enabled; 
    change_text(enabled ? "Download" : "Downloading...");
    set_visibility(enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED);
}


//////////////////////////////////////
// SettingsResourceDeleteButton
//////////////////////////////////////
SettingsResourceDeleteButton::SettingsResourceDeleteButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}

void SettingsResourceDeleteButton::set_enabled(bool enabled){ 
    m_enabled = enabled; 
    change_text(enabled ? "Delete" : "Deleting...");
    set_visibility(enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED);
}

//////////////////////////////////////
// SettingsResourceCancelButton
//////////////////////////////////////
SettingsResourceCancelButton::SettingsResourceCancelButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceCancelButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}

void SettingsResourceCancelButton::set_enabled(bool enabled){ 
    m_enabled = enabled; 
    change_text(enabled ? "Cancel" : "Cancelling...");
    set_visibility(enabled ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED);
}
//////////////////////////////////////
// SettingsResourceProgressBar
//////////////////////////////////////
SettingsResourceProgressBar::SettingsResourceProgressBar(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceProgressBar>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}

//////////////////////////////////////
// SettingsDownloadError
//////////////////////////////////////
SettingsDownloadError::SettingsDownloadError()
    : ConfigOptionImpl<SettingsDownloadError>(LockMode::LOCK_WHILE_RUNNING)
{}

}
