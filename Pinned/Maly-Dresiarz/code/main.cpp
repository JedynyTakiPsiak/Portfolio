#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WServer.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <memory>

#include "MainApplication.h"
#include "FinishedGameApplication.h"
#include "JoinGameApplication.h"
#include "GameApplication.h"
#include "InvalidGameApplication.h"
#include "Globals.h"
#include "Game.h"
#include "User.h"

extern std::map<std::string, std::shared_ptr<Game>> activeGames;

bool isValidGameId(const std::string& gameId)
{
    return gameId.length() == 8 && std::all_of(gameId.begin(), gameId.end(), ::isxdigit);
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
    std::string path = env.internalPath();
    std::cout << "internalPath: " << env.internalPath() << std::endl;

    if (path == "/" || path.empty()) {
        // Lobby / strona główna gry  
        return std::make_unique<MainApplication>(env);
    }

    const std::string prefix = "/game/";
    if (path.substr(0, prefix.size()) == prefix && path.length() > prefix.size()) {
        std::string gameId = path.substr(prefix.size());
        if (!isValidGameId(gameId)) {
            return std::make_unique<InvalidGameApplication>(env, gameId);
        }
        else {
            auto it = activeGames.find(gameId);

            if (it == activeGames.end()) {
                // No such game created
                return std::make_unique<InvalidGameApplication>(env, gameId);
            }
            else {
                // That game has been created
                const std::string* cookie = env.getCookie(gameId);
                auto currentGame = it->second;

                if (!cookie || currentGame->getNumberOfPlayersJoined() < currentGame->getNumberOfPlayers()) {
                    // Player has not joined yet or game not full
                    return std::make_unique<JoinGameApplication>(env, gameId);
                }
                else {
                    // Player joined
                    if (currentGame->isGameFinished()) {
                        // The game finished
                        return std::make_unique<FinishedGameApplication>(env, gameId);
                    }
                    else {
                        // Game in progress
                        return std::make_unique<GameApplication>(env, gameId);
                    }
                }
            }
        }
    }
    // Jeśli ścieżka jest nieznana, przekierowuje do lobby
    return std::make_unique<MainApplication>(env);
}


// Main entry point -> starts web server
int main(int argc, char** argv)
{
    try {
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);
        server.addEntryPoint(Wt::EntryPointType::Application, createApplication, "/maly_dresiarz");
        
        if (server.start()) {
            std::cout << "Gra wystartowala na http://localhost:" << server.httpPort() << "/maly_dresiarz" << std::endl;
            
            Wt::WServer::waitForShutdown();
            server.stop();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}