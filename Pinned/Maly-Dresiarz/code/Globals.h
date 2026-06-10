#pragma once

#include <map>
#include <string>
#include <memory>

class Game;
class GameApplication;

// Global storage for all active games -> maps gameID to Game object
extern std::map<std::string, std::shared_ptr<Game>> activeGames;

