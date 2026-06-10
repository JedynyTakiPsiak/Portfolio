#ifndef USER_H
#define USER_H

#include <string>
#include <memory>


class User
{
    public:
        //testing smth, if real player need full options of arguments
        User(std::string nickname, std::string gameID, int currentUserNumber) 
            : User(nickname, gameID, currentUserNumber, false, 0) {}

        User(std::string nickname, std::string gameID, int currentUserNumber, bool isBot, int difficulty = 0);
        virtual ~User();

        void decreaseCash(int amount);

        // getters
        int getUserID() const;
        std::string getFullUserID() const;
        std::string getNickname() const;
        std::string getGameID() const;
        int getCash() const;
        int getRespect() const;
        int getBMW() const;
        int getExtortion() const;

        // friend functions
        friend void showOff(User& user);
        friend void work(User& user);
        friend void addExtortion(User& user);
        friend void collectExtortion(User& user);
        friend void stealBMW(User& user);
        friend void useBMW(User& user);
        

        //testing
        int getNumberOfClicks() const;
        void incrementNumberOfClicks();


        // bots
        bool getIsBot() const { return isBot; }
        int getDifficultyLevel() const { return difficultyLevel; }
        static std::string generateBotsName();
        std::string decideBotAction (int currentRound) const;

    protected:

    private:
        std::string nickname;
        int userID;
        std::string fullUserID; // <gameID>_<ordinal number>
        std::string gameID;

        void generateFullUserID(int userNumber, std::string);
        void generateUserID(int userNumber);

        //testing
        int numberOfClicks;
        int cash = 0;
        int respect = 0;
        int BMW = 0;
        int extortion = 0;

        // bots
        bool isBot = false;
        int difficultyLevel = 0; // 0 = easy, 1 = normal, 2 = hard
};

#endif
