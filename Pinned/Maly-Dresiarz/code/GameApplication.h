#pragma once

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>
#include <string>
#include <map>
#include <memory>

class GameApplication : public Wt::WApplication
{
public:
    GameApplication(const Wt::WEnvironment& env, const std::string& gameId);
    ~GameApplication();

    void setTestText(std::string nickname);
    void refreshPage();

private:
    void showGameUI(const Wt::WEnvironment& env);
    void buildGameTable();
    void setButtons();
    void setScoreBoard();
    void setRoundInformation();
    void everyRoundRepeat();
    void updatePlayerSeats();
    void updateGameInfo();
    void showLastAction(const std::string& actionName, const std::string& playerName, const std::string& description);
    void clearLastAction(); //testing
    void setPlayerSeatState(Wt::WContainerWidget* seat, int playerId, bool isActive, bool isEmpty = false);
    
    std::string gameId;
    std::string nickname;
    int userIdFromCookie = -1;
    bool isGameFinished = false;

    // Layout containers
    Wt::WContainerWidget* gameWrapper = nullptr;
    Wt::WContainerWidget* gameTable = nullptr;
    Wt::WContainerWidget* gameInfoBar = nullptr;
    Wt::WContainerWidget* centerTable = nullptr;
    Wt::WContainerWidget* lastActionContainer = nullptr;
    Wt::WContainerWidget* actionsBar = nullptr;

    // Player seats
    Wt::WContainerWidget* seatTop = nullptr;
    Wt::WContainerWidget* seatRight = nullptr;
    Wt::WContainerWidget* seatBottom = nullptr;
    Wt::WContainerWidget* seatLeft = nullptr;

    // Game info elements
    Wt::WText* roundInfo = nullptr;
    Wt::WText* turnInfo = nullptr;

    // Last action elements
    Wt::WText* actionTitle = nullptr;
    Wt::WText* actionPlayer = nullptr;
    Wt::WText* actionDescription = nullptr;

    // Action buttons
    Wt::WPushButton* workButton = nullptr;
    Wt::WPushButton* extortionButton = nullptr;
    Wt::WPushButton* showOffButton = nullptr;
    Wt::WPushButton* stealButton = nullptr;
    Wt::WPushButton* iwanButton = nullptr;

    // Legacy elements (for compatibility)
    Wt::WText* gameStatusMessage = nullptr;
    Wt::WText* testText = nullptr;
    Wt::WContainerWidget* scoreBoardContainer = nullptr;
    Wt::WContainerWidget* roundInformationContainer = nullptr;
    Wt::WContainerWidget* buttonsContainer = nullptr;

    //bots
    void handleBotTurn();
    Wt::WTimer* botActionTimer = nullptr;
};