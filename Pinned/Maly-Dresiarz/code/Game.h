#ifndef GAME_H
#define GAME_H

#include <string>
#include <random>
#include <vector>
#include <memory>
#include <map>


#include "User.h"

// Main game logic class -> manages players, turns, rounds and game state
class Game {
    public:
        Game(int numberOfPlayers, int numberOfRounds = 10); //change number of rounds
        virtual ~Game();

        // Basic game info getters
        std::string getGameID() const;
        int getNumberOfPlayers() const;
        int getNumberOfPlayersJoined() const;
        int getNumberOfRounds() const;
        int getCurrentRound() const;

        // Core game mechanics
        void processStartOfTurn();
        void iwan(int triggeringUserId);
        void nextRound();
        void nextTurn();
        int whosTurn();
        bool isGameFinished();

        std::string whoWins();

        // Player management
        void addPlayer(std::shared_ptr<User> user);
        bool arePlayersAccepted();
        bool isNicknameTaken(const std::string& nickname) const;
        std::shared_ptr<User> getUserById(int userId);
        std::map<int, std::shared_ptr<User>>& getPlayers();
        std::string getNicknameOfUserPlaying();

        //testing
        int getCounter() { return counter; };
        void incrementCounter() {counter++;};

        //bots
        void setBotDifficultyLevel(int level) { botDifficultyLevel = level; }
        int getBotDifficultyLevel() const { return botDifficultyLevel; }
        void addBot(int difficulty);

    protected:

    private:
        int numberOfPlayers;
        int numberOfPlayersJoined;
        int numberOfRounds;
        int currentRound;
        int currentTurn;
        std::string gameID;
        std::map<int, std::shared_ptr<User>> players;

        std::string generateGameID();

        //testing
        int counter = 0;

        // bots
        int botDifficultyLevel = 0;
};

#endif
