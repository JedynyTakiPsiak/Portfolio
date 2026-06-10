#include "User.h"

#include <memory>
#include <string>
#include <random>
#include <vector>

User::User(std::string nickname, std::string gameID, int currentUserNumber, bool isBot, int difficulty)
    : nickname(nickname), gameID(gameID), numberOfClicks(0), isBot(isBot), difficultyLevel(difficulty)
{
    generateFullUserID(currentUserNumber, gameID);
}

User::~User() {}

void User::generateFullUserID(int userNumber, std::string gameID) {
    generateUserID(userNumber);
    fullUserID = gameID + "_" + std::to_string(userID);
}

void User::generateUserID(int userNumber) {
    userID = userNumber + 1;
}

std::string User::getFullUserID() const {
    return fullUserID;
}

int User::getUserID() const {
    return userID;
}

std::string User::getNickname() const {
    return nickname;
}

std::string User::getGameID() const {
    return gameID;
}

int User::getCash() const {
    return cash;
}

int User::getRespect() const {
    return respect;
}

int User::getBMW() const {
    return BMW;
}

int User::getExtortion() const {
    return extortion;
}

//testing
void User::incrementNumberOfClicks(){
    numberOfClicks++;
}

int User::getNumberOfClicks() const{
    return numberOfClicks;
}

void User::decreaseCash(int amount) {
    if (cash >= amount) cash -= amount;
    else cash = 0;
}


//bots
static const std::vector<std::string> polishNames = {
    "Jan", "Anna", "Piotr", "Mariusz", "Krzysztof", "Kasia",
    "Andrzej", "Magdalena", "Tomasz", "Grzegorz", "Jakub",
    "Marcin", "Maria", "Ewa", "Adam", "Janusz", "Grazyna"
};

std::string User::generateBotsName() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, polishNames.size() - 1);

    return "(Bot) " + polishNames[distrib(gen)];
}

std::string User::decideBotAction(int currentRound) const {
    // check if its real user
    if (!isBot) return "";

    // Boty nie patrza na innych wartosci, poniewaz nie ma wiekszego celu w takim dzialaniu.
    // Koncowo wychodzi ze prosta taktyka uzycia Do Roboty co runde + 1 BMW == najwieksza ilosc Szacunku.
    // Dodatkowo w ostatniej rundzie procz Lans czy Iwan (aby komus przeszkodzic jesli jestesmy pierwsi w rundzie),
    // nie daje kompletnie nic innego niz puste wartosc. Poniewaz obchodzi nas tylko liczba Szacunku.

    // Jeden problem przy 2 botach w grze jest taki, ze bysmy mogli zrobic aby zawsze uzywal Iwana.
    // Powoduje to iz zaden gracz nigdy nie bedzie miec hajsu $, aby uzyc BMW czy Do Roboty. 
    // Wiec sa zmuszeni do uzycia Lans. Co pozwala nam opoznic ich w rozwoju Szacunku o 1-2 rundy.
    // Dajac nam automatycznie przewage nie do wygrania.
    switch (difficultyLevel) {
        case 0: {// difficulty easy
            // Losowanie miedzy kazda akcja, z szansa 20%
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, 4);
            int liczba = distrib(gen);
            switch (liczba) {
            case 0: return "work";
            case 1: return "extortion";
            case 2: return "showoff";
            case 3: return "steal";
            case 4: return "iwan";
            default: return "work";
            }
        }
        case 1: {//difficulty normal
            // Metoda uzyskania sredniej ilosci Szacunku.
            // Zarazem przeszkadzamy innym graczom jak rowniez uzyskujemy sredni wynik.
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, 2);
            int liczba = distrib(gen);
            static const std::vector<std::string> akcja = {
                "showoff", "steal", "iwan"
            };
            switch (currentRound) {
                case 1: return "work";
                case 2: return "work";
                case 3: return (cash >= 4) ? "extortion" : "iwan";
                case 4: return akcja[liczba]; // losowa akcja 
                case 5: return "steal";
                case 6: return "work";
                case 7: return (cash >= 2) ? akcja[liczba] : "work"; // jesli mamy 2 hajsu na nastepna runde dla BMW to losowa akcja
                case 8: return (cash >= 2) ? "showoff" : "iwan";
                case 9: return (cash >= 3) ? akcja[liczba] : "work";
                case 10: return "iwan";
                default: return "work";
            }
        }
        case 2: {//difficulty hard
            // Metoda aby uzyskac jak najwieksza ilosc Szacun w 10 rund,
            // Jesli nikt nie uzyje Iwana, to uzyskamy zawsze 25 Szacun.
            // Jesli jednak ktos lub wiecej uzyje to bedziemy zawsze stratni owa runde,
            // lecz rowniez analogicznie, on rowniez nie otrzyma z Iwana nic.
            // Wiec aby pokonac ta taktyke ktos albo musi sie poswiecic i uzyc Iwana,
            // albo uzyc Haraczu tak aby miec 19 Szacunku, ale zarazem uzyc 1-2x Iwana.
            switch (currentRound) {
                case 1: return "work";
                case 2: return "steal";
                case 3: case 4: case 5: case 6: case 7: case 8: case 9: return "work";
                case 10: return "showoff";
                default: return "work";
            }
        }
        default: 
            return "work";
    }
}