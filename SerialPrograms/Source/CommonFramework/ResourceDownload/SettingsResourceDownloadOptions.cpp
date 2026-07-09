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



void SettingsResourceButton::add_button_listener(Listener& listener){
    m_listeners.add(listener);
}
void SettingsResourceButton::remove_button_listener(Listener& listener){
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

void SettingsResourceProgressBar::add_progress_listener(Listener& listener){
    m_listeners.add(listener);
}
void SettingsResourceProgressBar::remove_progress_listener(Listener& listener){
    m_listeners.remove(listener);
}

void SettingsResourceProgressBar::change_text(const std::string& text){
    m_listeners.run_method(&Listener::on_change_text, text);
}

void SettingsResourceProgressBar::update_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_update_progress, bytes_done, total_bytes);
}

void SettingsResourceProgressBar::reset_progress(){
    m_listeners.run_method(&Listener::on_reset_progress);
}

////////////////////////////////
// ResourceDownload::Listener
////////////////////////////////
void SettingsResourceProgressBar::on_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    change_text("Downloading");
    set_visibility(ConfigOptionState::ENABLED);
    update_progress(bytes_done, total_bytes);
}
void SettingsResourceProgressBar::on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    change_text("Unzipping");
    set_visibility(ConfigOptionState::ENABLED);
    update_progress(bytes_done, total_bytes);
}
void SettingsResourceProgressBar::on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    change_text("Verifying");
    set_visibility(ConfigOptionState::ENABLED);
    update_progress(bytes_done, total_bytes);
}

//////////////////////////////////////
// SettingsDownloadPopup
//////////////////////////////////////
SettingsDownloadPopup::SettingsDownloadPopup(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsDownloadPopup>(LockMode::LOCK_WHILE_RUNNING)
    , row(p_row)
{}

//////////////////////////////////////
// SettingsDownloadError
//////////////////////////////////////
SettingsDownloadError::SettingsDownloadError()
    : ConfigOptionImpl<SettingsDownloadError>(LockMode::LOCK_WHILE_RUNNING)
{}

}
