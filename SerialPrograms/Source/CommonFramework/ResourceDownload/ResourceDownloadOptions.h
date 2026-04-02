/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadOptions_H
#define PokemonAutomation_ResourceDownloadOptions_H

#include <QObject>
// #include "Common/Cpp/Containers/Pimpl.h"
// #include "Common/Cpp/Concurrency/AsyncTask.h"
// #include "Common/Cpp/Options/StaticTableOption.h"
// #include "ResourceDownloadHelpers.h"


namespace PokemonAutomation{

class ResourceDownloadRow;


class ResourceDownloadButton : public QObject, public ConfigOptionImpl<ResourceDownloadButton>{
    Q_OBJECT
public:
    // ~ResourceDownloadButton();
    ResourceDownloadButton(ResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    ResourceDownloadRow& row;

private:
    bool m_enabled;  // button should be blocked during an active task. m_enabled is false when blocked

    
    

};

class ResourceDeleteButton : public ConfigOptionImpl<ResourceDeleteButton>{
public:
    ResourceDeleteButton(ResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
    ResourceDownloadRow& row;

private:
    bool m_enabled;
};

class ResourceCancelButton : public ConfigOptionImpl<ResourceCancelButton>{
public:
    ResourceCancelButton(ResourceDownloadRow& p_row);

public:
    inline bool get_enabled(){ return m_enabled; }
    inline void set_enabled(bool enabled){ 
        m_enabled = enabled; 
    }

public:
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
