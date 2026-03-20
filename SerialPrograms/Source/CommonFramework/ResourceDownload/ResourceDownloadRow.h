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
#include <optional>


namespace PokemonAutomation{

class ResourceDownloadRow;

class ResourceDownloadButton : public QObject, public ConfigOptionImpl<ResourceDownloadButton>{
    Q_OBJECT
public:
    ~ResourceDownloadButton();
    ResourceDownloadButton(ResourceDownloadRow& p_row);

signals:
    void download_finished();

public:
    void run_download();
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    ResourceDownloadRow& row;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked
    AsyncTask m_worker;


};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

    ResourceDownloadRow& row;
};

enum class ResourceVersionStatus{
    CURRENT,
    OUTDATED, // still used, but newer version available
    FUTURE_VERSION, // current version number is greater than the expected version number
    NOT_APPLICABLE, // resource not downloaded locally, so can't get its version
    // RETIRED, // no longer used
    // BLANK, // not yet fetched version info from remote
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
