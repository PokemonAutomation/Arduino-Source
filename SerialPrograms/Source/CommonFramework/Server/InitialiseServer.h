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
#include "Routes/ProgramRoutes.h"
#include "Routes/GlobalSettingsRoutes.h"

namespace PokemonAutomation{
namespace Server{

inline void init_server(){
    if (GlobalSettings::instance().ENABLE_API){
        // Start HTTP Server
        HTTPServer& httpServer = HTTPServer::instance();
        httpServer.start(8080);

        // Register all HTTP routes
        register_program_routes();
        register_settings_routes();

        // Start WebSocket Server
        WSServer& wsServer = WSServer::instance();
        wsServer.start(8081);
    }
}

}
}

#endif
