/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadOptions_H
#define PokemonAutomation_ResourceDownloadOptions_H

// #include "Common/Cpp/Containers/Pimpl.h"
// #include "Common/Cpp/Concurrency/AsyncTask.h"
// #include "Common/Cpp/Options/StaticTableOption.h"
// #include "ResourceDownloadHelpers.h"


namespace PokemonAutomation{

class SettingsResourceDownloadRow;


class SettingsResourceDownloadButton : public ConfigOptionImpl<SettingsResourceDownloadButton>{
public:
    // ~SettingsResourceDownloadButton();
    SettingsResourceDownloadButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    SettingsResourceDownloadRow& row;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked

    
    

};

class SettingsResourceDeleteButton : public ConfigOptionImpl<SettingsResourceDeleteButton>{
public:
    SettingsResourceDeleteButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    SettingsResourceDownloadRow& row;

private:
    bool m_enabled;
};

class SettingsResourceCancelButton : public ConfigOptionImpl<SettingsResourceCancelButton>{
public:
    SettingsResourceCancelButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    SettingsResourceDownloadRow& row;

private:
    bool m_enabled;    
};

class SettingsResourceProgressBar : public ConfigOptionImpl<SettingsResourceProgressBar>{
public:
    SettingsResourceProgressBar(SettingsResourceDownloadRow& p_row);

    SettingsResourceDownloadRow& row;
};


class SettingsDownloadError : public ConfigOptionImpl<SettingsDownloadError>{
public:
    SettingsDownloadError();


};


}
#endif
