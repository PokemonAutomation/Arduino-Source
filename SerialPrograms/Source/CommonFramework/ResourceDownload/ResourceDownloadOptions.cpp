/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ResourceDownloadRow.h"
#include "ResourceDownloadOptions.h"

// #include <thread>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

// ResourceDownloadButton::~ResourceDownloadButton(){}
ResourceDownloadButton::ResourceDownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


ResourceDeleteButton::ResourceDeleteButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


ResourceCancelButton::ResourceCancelButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceCancelButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


ResourceProgressBar::ResourceProgressBar(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceProgressBar>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}


}
