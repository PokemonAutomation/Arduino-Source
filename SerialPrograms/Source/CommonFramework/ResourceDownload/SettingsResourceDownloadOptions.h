/*  Resource Download Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadOptions_H
#define PokemonAutomation_ResourceDownloadOptions_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "ResourceDownload.h"

namespace PokemonAutomation{

class SettingsResourceDownloadRow;


class SettingsResourceButton {
public:
    struct Listener{
        virtual void on_change_text(const std::string& text){}
    };

    void add_button_listener(Listener& listener);
    void remove_button_listener(Listener& listener);

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

class SettingsResourceProgressBar : public ConfigOptionImpl<SettingsResourceProgressBar>, public ResourceDownload::Listener{
public:
    SettingsResourceProgressBar(SettingsResourceDownloadRow& p_row);

public:
    struct Listener{
        virtual void on_change_text(const std::string& text){}
        virtual void on_update_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_reset_progress(){}
    };

    void add_progress_listener(Listener& listener);
    void remove_progress_listener(Listener& listener);

    void change_text(const std::string& text);
    void update_progress(uint64_t bytes_done, uint64_t total_bytes);
    void reset_progress();

public: // ResourceDownload::Listener
    virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes) override;

public:
    SettingsResourceDownloadRow& row;

private:
    ListenerSet<Listener> m_listeners;

};

class SettingsDownloadPopup : public ConfigOptionImpl<SettingsDownloadPopup>, public ResourceDownload::Listener{
public:
    SettingsDownloadPopup(SettingsResourceDownloadRow& p_row);
public:
    SettingsResourceDownloadRow& row;

};

class SettingsDownloadError : public ConfigOptionImpl<SettingsDownloadError>{
public:
    SettingsDownloadError();


};


}
#endif
