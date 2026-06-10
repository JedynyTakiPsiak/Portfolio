#include "GameApplication.h"
#include "Globals.h"
#include "Game.h"
#include "User.h"
#include "UserFunctions.h"

#include <Wt/WEnvironment.h>
#include <Wt/WServer.h>
#include <Wt/WTimer.h>
#include <iostream>

GameApplication::GameApplication(const Wt::WEnvironment& env, const std::string& gameId)
    : Wt::WApplication(env), gameId(gameId)
{
    useStyleSheet("assets/style.css");
    
    enableUpdates(true);
    setTitle("Mały Dresiarz - Gra #" + gameId);
    setInternalPath("/game/" + gameId, true);
    
    const std::string* cookie = env.getCookie(gameId);
    if (cookie) {
        userIdFromCookie = std::stoi(*cookie);
    }
    
    showGameUI(env);
}

GameApplication::~GameApplication() {}

void GameApplication::showGameUI(const Wt::WEnvironment& env)
{
    root()->clear();
    buildGameTable();
    
    // Timer sprawdzający koniec gry
    auto finishCheckTimer = root()->addChild(std::make_unique<Wt::WTimer>());
    finishCheckTimer->setInterval(std::chrono::seconds(1));
    finishCheckTimer->timeout().connect([=] {
        if (activeGames[gameId]->isGameFinished()) {
            doJavaScript("window.location.reload(true);");
            finishCheckTimer->stop();
        }
    });
    finishCheckTimer->start();
    
    updatePlayerSeats();
    updateGameInfo();
    setButtons();
    
    // Legacy containers for compatibility
    scoreBoardContainer = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreBoardContainer->setStyleClass("hidden");
    roundInformationContainer = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    roundInformationContainer->setStyleClass("hidden");
    buttonsContainer = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    buttonsContainer->setStyleClass("hidden");

    //bots wait 2-3 sec before making a move, cause to give time to react, what happens for others players
    botActionTimer = root()->addChild(std::make_unique<Wt::WTimer>());
    botActionTimer->setInterval(std::chrono::seconds(2));
    botActionTimer->timeout().connect([=] {
        handleBotTurn();
    });

    //checking if its bot round
    if (activeGames[gameId] && !activeGames[gameId]->isGameFinished()) {
        auto game = activeGames[gameId];
        int currentTurn = game->whosTurn();
        auto currentPlayer = game->getUserById(currentTurn);

        if (currentPlayer && currentPlayer->getIsBot()) {
            botActionTimer->start();
        }
    }

}

void GameApplication::buildGameTable()
{
    // Main wrapper
    gameWrapper = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    gameWrapper->setStyleClass("game-wrapper");
    
    // Game table (main container)
    gameTable = gameWrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    gameTable->setStyleClass("game-table");
    
    // Game info bar (round, turn info)
    gameInfoBar = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    gameInfoBar->setStyleClass("game-info-bar");
    
    roundInfo = gameInfoBar->addWidget(std::make_unique<Wt::WText>("Runda <span class='round-number'>1</span>"));
    roundInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    roundInfo->setStyleClass("round-info");
    
    turnInfo = gameInfoBar->addWidget(std::make_unique<Wt::WText>("Ruch gracza: <span class='current-player'>Ładowanie...</span>"));
    turnInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    turnInfo->setStyleClass("turn-info");
    
    // Player seats
    seatTop = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    seatTop->setStyleClass("player-seat seat-top");
    
    seatRight = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    seatRight->setStyleClass("player-seat seat-right");
    
    seatBottom = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    seatBottom->setStyleClass("player-seat seat-bottom");
    
    seatLeft = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    seatLeft->setStyleClass("player-seat seat-left");
    
    // Center table (for last action)
    centerTable = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    centerTable->setStyleClass("center-table");
    
    lastActionContainer = centerTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    lastActionContainer->setStyleClass("last-action");
    
    actionPlayer = lastActionContainer->addWidget(std::make_unique<Wt::WText>(""));
    actionPlayer->setStyleClass("action-player");
    
    actionTitle = lastActionContainer->addWidget(std::make_unique<Wt::WText>("Oczekiwanie na ruch..."));
    actionTitle->setStyleClass("action-title");
    
    actionDescription = lastActionContainer->addWidget(std::make_unique<Wt::WText>(""));
    actionDescription->setStyleClass("action-description");
    
    // Actions bar
    actionsBar = gameTable->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsBar->setStyleClass("actions-bar");


    // Rules button + container
    auto rulesButton = root()->addWidget(std::make_unique<Wt::WPushButton>("📖 Zasady Gry"));
    rulesButton->setStyleClass("rules-fab-btn btn-extortion"); // używa stylu od przycisku "haracz"

    auto rulesOverlayContainer = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    rulesOverlayContainer->setStyleClass("game-rules-overlay");
    rulesOverlayContainer->hide(); // Zasady -> ukryty domyślnie, pokazuje sie po kliknieciu przycisku

    auto rulesModal = rulesOverlayContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    rulesModal->setStyleClass("rules-modal");

    auto closeBtn = rulesOverlayContainer->addWidget(std::make_unique<Wt::WPushButton>("✖"));
    closeBtn->setStyleClass("rules-close-btn");

    rulesButton->clicked().connect([=]() {
        rulesOverlayContainer->show();
    });

    closeBtn->clicked().connect([=]() {
        rulesOverlayContainer->hide();
    });

    // Rules text
    const std::string zasady_html = R"(
<div class='rules-title'>Zasady gry <span style='color:#7209B7;'>Mały Dresiarz</span></div>
<div class='rules-content'>
<b style='color:#4361EE;'>Cel gry:</b> <span style='color:#FFD43B;'>Zostań najbardziej szanowanym dresiarzem w dzielnicy!</span><br><br>
<b>Rozgrywka:</b>
<ul>
 <li><b style='color:#845ef7;'>Gra toczy się w turach.</b> Każdy gracz wykonuje swój ruch po kolei.</li>
 <li>Za zdobyte <span style='color:#ff6b6b;font-weight:bold;'>Szacun</span>, <span style='color:#51cf66;font-weight:bold;'>Kasa</span>, <span style='color:#FFD43B;font-weight:bold;'>BMW</span> i <span style='color:#845ef7;font-weight:bold;'>Haracz</span> zdobywasz punkty.</li>
 <li>Liczba tur: <b style='color:#FFD43B;'>10</b>. Wygrywa gracz z największą ilością <b style='color:#ff6b6b;'>Szacunku</b>.</li>
</ul><br>
<b>Akcje (do wyboru co turę):</b>
<ul>
  <li><b style='color:#845ef7;'>⭐ LANS</b>: +1 <span style='color:#ff6b6b;'>Szacun</span></li>
  <li><b style='color:#51cf66;'>🔨 DO ROBOTY</b>: +2 <span style='color:#51cf66;'>Kasa</span></li>
  <li><b style='color:#ff6b6b;'>💰 HARACZ</b>: -4 <span style='color:#51cf66;'>Kasa</span> (jednorazowo), a potem co nową turę +1 <span style='color:#51cf66;'>Kasa</span>.</li>
  <li><b style='color:#FFD43B;'>🚗 BMW</b>: Każda tura z BMW wymaga -2 <span style='color:#51cf66;'>Kasa</span>, lecz wtedy daje +3 <span style='color:#ff6b6b;'>Szacun</span>. Jeśli nie masz kasy na tankowanie, tracisz -2 <span style='color:#ff6b6b;'>Szacun</span>.</li>
  <li><b style='color:#339af0;'>💪 IWAN</b>: Wzywasz Iwana, każdy inny gracz traci -1 <span style='color:#51cf66;'>Kasa</span>. Ty nic nie tracisz.</li>
</ul>
<div class='rules-tips'>
<b>Pozostałe zasady:</b><br>
- Zasoby nigdy nie mogą być ujemne.<br>
- Nie możesz wykonać <span style='color:#ff6b6b;font-weight:bold;'>Haracz</span> jeśli masz mniej niż 4 <span style='color:#51cf66;'>Kasa</span>.<br>
- Haracz i BMW są stałe do końca gry.<br>
- Kolejność faz w turze: najpierw dostajesz kasę z haraczy, potem tankowanie BMW (lub strata szacunku), potem AKCJA.<br>
- Wygrywa osoba z największą ilością <span style='color:#ff6b6b;'>Szacun</span>. Pozostałe zasoby nie są brane pod uwagę.</div>
</div>
)";

    auto rulesText = rulesModal->addWidget(std::make_unique<Wt::WText>(zasady_html));
    rulesText->setTextFormat(Wt::TextFormat::UnsafeXHTML);


}

void GameApplication::setPlayerSeatState(Wt::WContainerWidget* seat, int playerId, bool isActive, bool isEmpty)
{
    seat->clear();

    std::string originalClass = seat->styleClass().toUTF8();
    std::string baseClass = "player-seat";

    if (originalClass.find("seat-top") != std::string::npos) baseClass += " seat-top";
    else if (originalClass.find("seat-right") != std::string::npos) baseClass += " seat-right";
    else if (originalClass.find("seat-bottom") != std::string::npos) baseClass += " seat-bottom";
    else if (originalClass.find("seat-left") != std::string::npos) baseClass += " seat-left";

    if (isEmpty) {
        seat->setStyleClass(baseClass + " empty");
        auto emptyText = seat->addWidget(std::make_unique<Wt::WText>("Wolne miejsce"));
        emptyText->setStyleClass("player-name");
        return;
    }

    if (isActive) {
        seat->setStyleClass(baseClass + " active");
    }
    else {
        seat->setStyleClass(baseClass);
    }
    
    if (!activeGames.count(gameId)) return;
    
    auto user = activeGames[gameId]->getUserById(playerId);
    if (!user) return;
    
    // Player info section
    auto playerInfo = seat->addWidget(std::make_unique<Wt::WContainerWidget>());
    playerInfo->setStyleClass("player-info");
    
    auto name = playerInfo->addWidget(std::make_unique<Wt::WText>(user->getNickname()));
    name->setStyleClass("player-name");
    
    // Player stats section
    auto stats = seat->addWidget(std::make_unique<Wt::WContainerWidget>());
    stats->setStyleClass("player-stats");

    // Szacun
    auto respectStat = stats->addWidget(std::make_unique<Wt::WContainerWidget>());
    respectStat->setStyleClass("stat-item stat-respect");
    auto respectLabel = respectStat->addWidget(std::make_unique<Wt::WText>("Szacun"));
    respectLabel->setStyleClass("stat-label");
    auto respectValue = respectStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getRespect())));
    respectValue->setStyleClass("stat-value");

    // Kasa
    auto cashStat = stats->addWidget(std::make_unique<Wt::WContainerWidget>());
    cashStat->setStyleClass("stat-item stat-cash");
    auto cashLabel = cashStat->addWidget(std::make_unique<Wt::WText>("Kasa"));
    cashLabel->setStyleClass("stat-label");
    auto cashValue = cashStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getCash())));
    cashValue->setStyleClass("stat-value");

    // BMW
    auto bmwStat = stats->addWidget(std::make_unique<Wt::WContainerWidget>());
    bmwStat->setStyleClass("stat-item stat-bmw");
    auto bmwLabel = bmwStat->addWidget(std::make_unique<Wt::WText>("BMW"));
    bmwLabel->setStyleClass("stat-label");
    auto bmwValue = bmwStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getBMW())));
    bmwValue->setStyleClass("stat-value");

    // Haracz
    auto extortionStat = stats->addWidget(std::make_unique<Wt::WContainerWidget>());
    extortionStat->setStyleClass("stat-item stat-extortion");
    auto extortionLabel = extortionStat->addWidget(std::make_unique<Wt::WText>("Haracz"));
    extortionLabel->setStyleClass("stat-label");
    auto extortionValue = extortionStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getExtortion())));
    extortionValue->setStyleClass("stat-value");
}

void GameApplication::updatePlayerSeats()
{
    if (!activeGames.count(gameId)) return;
    
    auto game = activeGames[gameId];
    const auto& players = game->getPlayers();
    int currentTurn = game->whosTurn();
    
    // Array of seats in order: top, right, bottom, left
    Wt::WContainerWidget* seats[4] = {seatTop, seatRight, seatBottom, seatLeft};
    
    // Fill seats with players
    int seatIndex = 0;
    for (const auto& [playerId, user] : players) {
        if (seatIndex < 4) {
            bool isActive = (playerId == currentTurn);
            setPlayerSeatState(seats[seatIndex], playerId, isActive, false);
            seatIndex++;
        }
    }
    
    // Fill remaining seats as empty
    for (int i = seatIndex; i < 4; i++) {
        setPlayerSeatState(seats[i], 0, false, true);
    }
}

void GameApplication::updateGameInfo()
{
    if (!activeGames.count(gameId)) return;
    
    auto game = activeGames[gameId];
    int currentRound = game->getCurrentRound();
    std::string currentPlayerName = game->getNicknameOfUserPlaying();
    
    roundInfo->setText("Runda <span class='round-number'>" + std::to_string(currentRound) + "</span>");
    roundInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);

    //check if its current player move
    bool isCurrentPlayerTurn = (userIdFromCookie == game->whosTurn());

    if (isCurrentPlayerTurn) {
        turnInfo->setText("Ruch gracza <span class='current-player'> Twój ruch</span>");
    } else {
        turnInfo->setText("Ruch gracza <span class='current-player'>" + currentPlayerName + "</span>");
    }
    turnInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);
}

void GameApplication::showLastAction(const std::string& actionName, const std::string& playerName, const std::string& description)
{
    actionTitle->setText(actionName);

    if (playerName.find("(Bot) ") == 0) {
        actionPlayer->setText(playerName + " wykonał </br>");
    } else {
        actionPlayer->setText("Gracz " + playerName + " wykonał </br>");
    }
    actionPlayer->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    lastActionContainer->addStyleClass("show");

    //removing to made space for players card
    //actionPlayer->setText("<br/>Gracz " + playerName + " ");
    //actionDescription->setText(description);
    
    //// Hide after 3 seconds
    //auto hideTimer = lastActionContainer->addChild(std::make_unique<Wt::WTimer>());
    //hideTimer->setInterval(std::chrono::seconds(3));
    //hideTimer->timeout().connect([=] {
    //    clearLastAction();
    //});
    //hideTimer->start();
}

//void GameApplication::clearLastAction()
//{
//    if (lastActionContainer) {
//        lastActionContainer->removeStyleClass("show");
//        actionTitle->setText("Oczekiwanie na ruch...");
//        actionPlayer->setText("");
//        actionDescription->setText("");
//    }
//}

void GameApplication::setButtons() {
    actionsBar->clear();
    
    // Create buttons
    workButton = actionsBar->addWidget(std::make_unique<Wt::WPushButton>("🔨 Do roboty"));
    workButton->setStyleClass("action-btn btn-work");
    
    extortionButton = actionsBar->addWidget(std::make_unique<Wt::WPushButton>("💰 Haracz"));
    extortionButton->setStyleClass("action-btn btn-extortion");
    
    showOffButton = actionsBar->addWidget(std::make_unique<Wt::WPushButton>("⭐ Lans"));
    showOffButton->setStyleClass("action-btn btn-showoff");
    
    stealButton = actionsBar->addWidget(std::make_unique<Wt::WPushButton>("🚗 BMW"));
    stealButton->setStyleClass("action-btn btn-steal");
    
    iwanButton = actionsBar->addWidget(std::make_unique<Wt::WPushButton>("💪 Iwan"));
    iwanButton->setStyleClass("action-btn btn-iwan");
    
    // Check if game is finished
    if (activeGames[gameId]->isGameFinished()) {
        workButton->setDisabled(true);
        extortionButton->setDisabled(true);
        showOffButton->setDisabled(true);
        stealButton->setDisabled(true);
        iwanButton->setDisabled(true);
        return;
    }
    
    // Check if its players turn
    bool isPlayersTurn = (userIdFromCookie == activeGames[gameId]->whosTurn());
    workButton->setDisabled(!isPlayersTurn);
    extortionButton->setDisabled(!isPlayersTurn);
    showOffButton->setDisabled(!isPlayersTurn);
    stealButton->setDisabled(!isPlayersTurn);
    iwanButton->setDisabled(!isPlayersTurn);
    
    // Special case for extortion, need 4 cash
    if (isPlayersTurn) {
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (user && user->getCash() < 4) {
            extortionButton->setDisabled(true);
        }
    }
    
    // Connect button handlers
    workButton->clicked().connect([=] {
        if (!isPlayersTurn) return;
        
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (!user) return;
        
        activeGames[gameId]->processStartOfTurn();
        work(*user);
        activeGames[gameId]->nextTurn();
        
        Wt::WServer::instance()->postAll([gameId = this->gameId, nickname = user->getNickname()] {
            if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
                if (app->gameId == gameId) {
                    app->updatePlayerSeats();
                    app->updateGameInfo();
                    app->setButtons();
                    app->showLastAction("🔨 DO ROBOTY", nickname, "+2 Kasa");

                    //bots handler
                    auto game = activeGames[gameId];
                    if (game && !game->isGameFinished()) {
                        int nextTurn = game->whosTurn();
                        auto nextPlayer = game->getUserById(nextTurn);
                        if (nextPlayer && nextPlayer->getIsBot()) {
                            app->botActionTimer->start();
                        }
                    }

                    //game update
                    app->triggerUpdate();
                }
            }
        });
    });
    
    extortionButton->clicked().connect([=] {
        if (!isPlayersTurn) return;
        
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (!user || user->getCash() < 4) return;
        
        activeGames[gameId]->processStartOfTurn();
        addExtortion(*user);
        activeGames[gameId]->nextTurn();
        
        Wt::WServer::instance()->postAll([gameId = this->gameId, nickname = user->getNickname()] {
            if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
                if (app->gameId == gameId) {
                    app->updatePlayerSeats();
                    app->updateGameInfo();
                    app->setButtons();
                    app->showLastAction("💰 HARACZ", nickname, "-4 Kasa, +1 Kasa na każdą turę");

                    //bots handler
                    auto game = activeGames[gameId];
                    if (game && !game->isGameFinished()) {
                        int nextTurn = game->whosTurn();
                        auto nextPlayer = game->getUserById(nextTurn);
                        if (nextPlayer && nextPlayer->getIsBot()) {
                            app->botActionTimer->start();
                        }
                    }

                    //game update
                    app->triggerUpdate();
                }
            }
        });
    });
    
    showOffButton->clicked().connect([=] {
        if (!isPlayersTurn) return;
        
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (!user) return;
        
        activeGames[gameId]->processStartOfTurn();
        showOff(*user);
        activeGames[gameId]->nextTurn();
        
        Wt::WServer::instance()->postAll([gameId = this->gameId, nickname = user->getNickname()] {
            if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
                if (app->gameId == gameId) {
                    app->updatePlayerSeats();
                    app->updateGameInfo();
                    app->setButtons();
                    app->showLastAction("⭐ LANS", nickname, "+1 Szacun");

                    //bots handler
                    auto game = activeGames[gameId];
                    if (game && !game->isGameFinished()) {
                        int nextTurn = game->whosTurn();
                        auto nextPlayer = game->getUserById(nextTurn);
                        if (nextPlayer && nextPlayer->getIsBot()) {
                            app->botActionTimer->start();
                        }
                    }

                    //game update
                    app->triggerUpdate();
                }
            }
        });
    });
    
    stealButton->clicked().connect([=] {
        if (!isPlayersTurn) return;
        
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (!user) return;
        
        activeGames[gameId]->processStartOfTurn();
        stealBMW(*user);
        activeGames[gameId]->nextTurn();
        
        Wt::WServer::instance()->postAll([gameId = this->gameId, nickname = user->getNickname()] {
            if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
                if (app->gameId == gameId) {
                    app->updatePlayerSeats();
                    app->updateGameInfo();
                    app->setButtons();
                    app->showLastAction("🚗 BMW", nickname, "+1 BMW (wymaga paliwa)");

                    //bots handler
                    auto game = activeGames[gameId];
                    if (game && !game->isGameFinished()) {
                        int nextTurn = game->whosTurn();
                        auto nextPlayer = game->getUserById(nextTurn);
                        if (nextPlayer && nextPlayer->getIsBot()) {
                            app->botActionTimer->start();
                        }
                    }

                    //game update
                    app->triggerUpdate();
                }
            }
        });
    });
    
    iwanButton->clicked().connect([=] {
        if (!isPlayersTurn) return;
        
        auto user = activeGames[gameId]->getUserById(userIdFromCookie);
        if (!user) return;
        
        activeGames[gameId]->processStartOfTurn();
        activeGames[gameId]->iwan(userIdFromCookie);
        activeGames[gameId]->nextTurn();
        
        Wt::WServer::instance()->postAll([gameId = this->gameId, nickname = user->getNickname()] {
            if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
                if (app->gameId == gameId) {
                    app->updatePlayerSeats();
                    app->updateGameInfo();
                    app->setButtons();
                    app->showLastAction("💪 IWAN", nickname, "Wszyscy inni gracze tracą -1 Kasa");

                    //bots handler
                    auto game = activeGames[gameId];
                    if (game && !game->isGameFinished()) {
                        int nextTurn = game->whosTurn();
                        auto nextPlayer = game->getUserById(nextTurn);
                        if (nextPlayer && nextPlayer->getIsBot()) {
                            app->botActionTimer->start();
                        }
                    }

                    //game update
                    app->triggerUpdate();
                }
            }
        });
    });
}

// Legacy functions for compatibility
// testing
void GameApplication::setTestText(std::string nickname)
{
    showLastAction("RUCH WYKONANY", nickname, "Gracz wykonał akcję");
}

void GameApplication::refreshPage()
{
    doJavaScript("setTimeout(function() { window.location.reload(true); }, 300);");
}

void GameApplication::setScoreBoard()
{
    updatePlayerSeats();
}

void GameApplication::setRoundInformation()
{
    updateGameInfo();
}

void GameApplication::everyRoundRepeat()
{
    auto user = activeGames[gameId]->getUserById(userIdFromCookie);
    if (user) {
        collectExtortion(*user);
        useBMW(*user);
    }
}

//bots
void GameApplication::handleBotTurn() {
    if (!activeGames.count(gameId)) return;
    
    auto game = activeGames[gameId];
    if (game->isGameFinished()) return;

    int currentTurn = game->whosTurn();
    auto currentPlayer = game->getUserById(currentTurn);

    if (!currentPlayer || !currentPlayer->getIsBot()) {
        botActionTimer->stop();
        return;
    }

    //bots action
    std::string botAction = currentPlayer->decideBotAction(game->getCurrentRound());

    game->processStartOfTurn();

    if (botAction == "work") {
        work(*currentPlayer);
        showLastAction("🔨 DO ROBOTY", currentPlayer->getNickname(), "+2 Kasa");
    } else if (botAction == "extortion" && currentPlayer->getCash() >= 4) {
        addExtortion(*currentPlayer);
        showLastAction("💰 HARACZ", currentPlayer->getNickname(), "-4 Kasa, +1 Kasa na każdą turę");
    } else if (botAction == "showoff") {
        showOff(*currentPlayer);
        showLastAction("⭐ LANS", currentPlayer->getNickname(), "+1 Szacun");
    } else if (botAction == "steal") {
        stealBMW(*currentPlayer);
        showLastAction("🚗 BMW", currentPlayer->getNickname(), "+1 BMW (wymaga paliwa)");
    } else if (botAction == "iwan") {
        game->iwan(currentPlayer->getUserID());
        showLastAction("💪 IWAN", currentPlayer->getNickname(), "Wszyscy inni gracze tracą -1 Kasa");
    } else { //backup move if something breaks
        work(*currentPlayer);
        showLastAction("🔨 DO ROBOTY", currentPlayer->getNickname(), "+2 Kasa");
    }
    
    game->nextTurn();

    // UI update
    Wt::WServer::instance()->postAll([gameId = this->gameId] {
        if (auto app = dynamic_cast<GameApplication*>(Wt::WApplication::instance())) {
            if (app->gameId == gameId) {
                app->updatePlayerSeats();
                app->updateGameInfo();
                app->setButtons();

                //check if next player is also bot
                auto game = activeGames[gameId];
                if (game && !game->isGameFinished()) {
                    int nextTurn = game->whosTurn();
                    auto nextPlayer = game->getUserById(nextTurn);

                    if (nextPlayer && nextPlayer->getIsBot()) {
                        app->botActionTimer->start();
                    } else {
                        app->botActionTimer->stop();
                    }
                }

                app->triggerUpdate();
            }
        }
    });

    botActionTimer->stop();
}