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

// SettingsResourceDownloadButton::~SettingsResourceDownloadButton(){}
SettingsResourceDownloadButton::SettingsResourceDownloadButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


SettingsResourceDeleteButton::SettingsResourceDeleteButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


SettingsResourceCancelButton::SettingsResourceCancelButton(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceCancelButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


SettingsResourceProgressBar::SettingsResourceProgressBar(SettingsResourceDownloadRow& p_row)
    : ConfigOptionImpl<SettingsResourceProgressBar>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}


SettingsDownloadError::SettingsDownloadError()
    : ConfigOptionImpl<SettingsDownloadError>(LockMode::LOCK_WHILE_RUNNING)
{}

}
