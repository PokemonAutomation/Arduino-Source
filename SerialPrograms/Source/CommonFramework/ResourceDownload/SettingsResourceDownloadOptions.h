/*  Resource Download Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadOptions_H
#define PokemonAutomation_ResourceDownloadOptions_H

#include "Common/Cpp/ListenerSet.h"

namespace PokemonAutomation{

class SettingsResourceDownloadRow;


class SettingsResourceButton {
public:
    struct Listener{
        virtual void on_change_text(const std::string& text){}
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);

    void change_text(const std::string& text);

private:
    ListenerSet<Listener> m_listeners;

};

class SettingsResourceDownloadButton : public ConfigOptionImpl<SettingsResourceDownloadButton>, public SettingsResourceButton{
public:
    // ~SettingsResourceDownloadButton();
    SettingsResourceDownloadButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    void set_enabled(bool enabled);

public:
    SettingsResourceDownloadRow& row;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked

    

};

class SettingsResourceDeleteButton : public ConfigOptionImpl<SettingsResourceDeleteButton>, public SettingsResourceButton{
public:
    SettingsResourceDeleteButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    void set_enabled(bool enabled);

public:
    SettingsResourceDownloadRow& row;

private:
    bool m_enabled;
};

class SettingsResourceCancelButton : public ConfigOptionImpl<SettingsResourceCancelButton>, public SettingsResourceButton{
public:
    SettingsResourceCancelButton(SettingsResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    void set_enabled(bool enabled);

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
