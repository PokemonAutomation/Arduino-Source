/*  PABotBase2 Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_Connection_H
#define PokemonAutomation_Controllers_PABotBase2_Connection_H

#include <memory>
#include "Common/Cpp/CancellableScope.h"
#include "Controllers/ControllerConnection.h"
#include "PABotBase2_DeviceHandle.h"

namespace PokemonAutomation{
namespace PABotBase2{


class Connection : public ControllerConnection{
public:
    PABotBase2::DeviceHandle& device(){
        return *m_device;
    }

    virtual void try_set_controller_type(
        ControllerType controller_type,
        bool clear_settings
    ) noexcept override{
        if (!is_ready()){
            return;
        }
        m_device->try_set_controller_type(controller_type, clear_settings);
    }

protected:
    std::unique_ptr<PABotBase2::DeviceHandle> m_device;
};



}
}
#endif
