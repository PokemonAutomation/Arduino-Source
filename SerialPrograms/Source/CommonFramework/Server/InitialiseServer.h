/*  Routes registration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Server_Routes_H
#define PokemonAutomation_Server_Routes_H

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Server/HTTP.h"
#include "CommonFramework/Server/WebSocket.h"
#include "CommonFramework/Options/Environment/APIOptions.h"
#include "Routes/ProgramRoutes.h"
#include "Routes/GlobalSettingsRoutes.h"

namespace PokemonAutomation{
namespace Server{

inline void init_server(){
    if (GlobalSettings::instance().API->ENABLE_API){
        // Start HTTP Server
        HTTPServer& httpServer = HTTPServer::instance();
        httpServer.start(GlobalSettings::instance().API->HTTP_PORT);

        // Register all HTTP routes
        register_program_routes();
        register_settings_routes();

        // Start WebSocket Server
        WSServer& wsServer = WSServer::instance();
        wsServer.start(GlobalSettings::instance().API->WS_PORT);
    }
}

}
}

#endif
