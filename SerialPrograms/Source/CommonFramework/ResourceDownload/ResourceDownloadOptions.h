/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadOptions_H
#define PokemonAutomation_ResourceDownloadOptions_H

#include <QObject>
// #include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
// #include "Common/Cpp/Options/StaticTableOption.h"
#include "ResourceDownloadHelpers.h"


namespace PokemonAutomation{

class ResourceDownloadRow;


class ResourceDownloadButton : public QObject, public ConfigOptionImpl<ResourceDownloadButton>{
    Q_OBJECT
public:
    ~ResourceDownloadButton();
    ResourceDownloadButton(ResourceDownloadRow& p_row);

signals:
    void metadata_fetch_finished(std::string popup_message);
    void exception_caught(std::string function_name);
    void download_failed();
    void download_finished();

public:
    void start_download();

    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(RemoteMetadata& remote_metadata);

    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    ResourceDownloadRow& row;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked

    AsyncTask m_worker1;
    AsyncTask m_worker2;

    
    

};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;

private:
    bool m_enabled;
};

class ResourceCancelButton : public ConfigOptionImpl<ResourceCancelButton>{
public:
    ResourceCancelButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;

private:
    bool m_enabled;    
};

class ResourceProgressBar : public ConfigOptionImpl<ResourceProgressBar>{
public:
    ResourceProgressBar(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;
};



}
#endif
