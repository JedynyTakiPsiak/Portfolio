#include "Game.h"

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <vector>


Game::Game(int numberOfPlayers, int numberOfRounds):
    numberOfPlayers(numberOfPlayers),
    numberOfPlayersJoined(0),
    currentRound(1),
    currentTurn(1),
    numberOfRounds(numberOfRounds),
    gameID(generateGameID()) {}

Game::~Game(){}

void Game::addPlayer(std::shared_ptr<User> user) {
    players[user->getUserID()] = user;
    numberOfPlayersJoined++;
}

std::shared_ptr<User> Game::getUserById(int userId) {
    auto it = players.find(userId);
    if (it != players.end()) {
        return it->second;
    }
    return nullptr;
}

std::map<int, std::shared_ptr<User>>& Game::getPlayers() {
    return players;
}

void Game::nextRound(){
    currentRound++;
}

int Game::getCurrentRound() const{
    return currentRound;
}

bool Game::arePlayersAccepted(){
    return numberOfPlayersJoined < numberOfPlayers;
}

bool Game::isNicknameTaken(const std::string& nickname) const {
    for (const auto& [_, userPtr] : players) {
        if (userPtr->getNickname() == nickname) {
            return true;
        }
    }
    return false;
}

std::string Game::getGameID() const {
    return gameID;
}

int Game::getNumberOfPlayers() const {
    return numberOfPlayers;
}

int Game::getNumberOfPlayersJoined() const {
    return numberOfPlayersJoined;
}

int Game::getNumberOfRounds() const {
    return numberOfRounds;
}

int Game::whosTurn(){
    return currentTurn;
}

// Handles automatic actions at start of each turn (extortion, BMW)
void Game::processStartOfTurn() {
    int currentPlayerId = currentTurn;
    if (players.count(currentPlayerId)) {
        auto player = players[currentPlayerId];
        collectExtortion(*player);
        useBMW(*player);
    }
}

void Game::nextTurn(){
    if(currentTurn < players.size()){
        currentTurn++;
    }
    else{
        currentTurn = 1;
        nextRound();
    }
}

std::string Game::getNicknameOfUserPlaying(){
    for(auto player : players){
        int id = whosTurn();
        if(id == player.first){
            return player.second->getNickname();
        }
    }
}

std::string Game::generateGameID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> distrib(0, 0xFFFFFFFF);

    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << distrib(gen);

    return oss.str();
}


// moved to FinishedGameApplication
std::string Game::whoWins() {
    if (players.empty()) return "Brak graczy";

    std::shared_ptr<User> winner = nullptr;
    int maxRespect = -1;
    bool tie = false;

    for (const auto& [_, user] : players) {
        int respect = user->getRespect();
        if (respect > maxRespect) {
            winner = user;
            maxRespect = respect;
            tie = false;
        }
        else if (respect == maxRespect && winner != nullptr && user != winner) {
            tie = true;
        }
    }

    if (tie) return "Remis!";
    return "Wygral " + winner->getNickname() + " z " + std::to_string(winner->getRespect()) + " respektu!";
}




bool Game::isGameFinished(){
    return currentRound > numberOfRounds;
}

void Game::iwan(int triggeringUserId) {
    for (auto& [userId, userPtr] : players) {
        if (userId != triggeringUserId) {
            if (userPtr->getCash() > 0) {
                userPtr->decreaseCash(1);
            }
        }
    }
}



//bots

void Game::addBot(int difficulty) {
    if (numberOfPlayersJoined >= numberOfPlayers) return;

    std::string botName = User::generateBotsName();
    auto bot = std::make_shared<User>(botName, gameID, numberOfPlayersJoined, true, difficulty);

    players[bot->getUserID()] = bot;
    numberOfPlayersJoined++;
}