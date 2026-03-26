/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadRow_H
#define PokemonAutomation_ResourceDownloadRow_H

#include <QObject>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Options/StaticTableOption.h"
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
    void download_finished();

public:
    enum class RemoteMetadataStatus{
        UNINITIALIZED,
        NOT_AVAILABLE,
        AVAILABLE,
    };
    struct RemoteMetadata {
        RemoteMetadataStatus status = RemoteMetadataStatus::UNINITIALIZED;
        DownloadedResourceMetadata metadata;
    };

    // get the DownloadedResourceMetadata from the remote JSON, that corresponds to this button/row
    void initialize_remote_metadata();
    RemoteMetadata& fetch_remote_metadata();
    DownloadedResourceMetadata initialize_local_metadata();
    void ensure_remote_metadata_loaded();
    std::string predownload_warning_summary(RemoteMetadata& remote_metadata);

    void run_download();
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    ResourceDownloadRow& row;
    std::once_flag init_flag;
    std::unique_ptr<RemoteMetadata> m_remote_metadata;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked
    DownloadedResourceMetadata m_local_metadata;
    AsyncTask m_worker1;
    AsyncTask m_worker2;

    
    

};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;
};



class ResourceDownloadRow : public StaticTableRow{

public:
    ~ResourceDownloadRow();
    ResourceDownloadRow(
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    );

public:
    struct Data;
    Pimpl<Data> m_data;

    ResourceDownloadButton m_download_button;
    ResourceDeleteButton m_delete_button;

};

}
#endif
