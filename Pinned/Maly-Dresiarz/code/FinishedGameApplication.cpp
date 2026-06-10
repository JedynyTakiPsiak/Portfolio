#include "FinishedGameApplication.h"
#include "Globals.h"
#include "Game.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>
#include <iostream>
#include <vector>
#include <algorithm>

FinishedGameApplication::FinishedGameApplication(const Wt::WEnvironment& env, const std::string& gameId)
    : Wt::WApplication(env), gameId(gameId)
{
    useStyleSheet("assets/style.css");
    
    setTitle("Gra zakończona - Mały Dresiarz");
    showFinishedGameUI();
}

FinishedGameApplication::~FinishedGameApplication() {}

// Main function to build the game over screen
void FinishedGameApplication::showFinishedGameUI() {
    root()->clear();
    
    auto container = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("main-container game-over-container fade-in");
    
    // Game over title
    auto title = container->addWidget(std::make_unique<Wt::WText>("🏆 GRA ZAKOŃCZONA"));
    title->setStyleClass("game-over-title");
    
    // Game info message
    auto gameInfo = container->addWidget(std::make_unique<Wt::WText>(
        "Gra <strong>#" + gameId + "</strong> dobiegła końca! </br> Czas na finalne statystyki!"
    ));
    gameInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    gameInfo->setStyleClass("menu-description");
    
    if (activeGames.count(gameId)) {
        showDetailedStats(container); // Show winner and all player stats

        // Auto-cleanup timer
        auto cleanupTimer = root()->addChild(std::make_unique<Wt::WTimer>());
        cleanupTimer->setInterval(std::chrono::minutes(60));
        cleanupTimer->timeout().connect([gameId = this->gameId] {
            if (activeGames.count(gameId)) {
                activeGames.erase(gameId);
                std::cout << "Game " << gameId << " auto-deleted after timeout.\n";
            }
        });
        cleanupTimer->start();
    } else {
        // Error if game not found
        auto errorMsg = container->addWidget(std::make_unique<Wt::WText>("❌ Nie można znaleźć wyników gry."));
        errorMsg->setStyleClass("join-message error-message");
    }
    
    // Thank you message
    auto thankYou = container->addWidget(std::make_unique<Wt::WText>(
        "Dziękujemy za grę!<br/>Mamy nadzieję, że się podobała! 😎"
    ));
    thankYou->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    thankYou->setStyleClass("thank-you-message");
    
    // Button to create new game / return to lobby
    goBackButton = container->addWidget(std::make_unique<Wt::WPushButton>("🎮 STWÓRZ NOWĄ GRĘ"));
    goBackButton->setStyleClass("main-btn");
    
    goBackButton->clicked().connect([=] {
        redirect("/maly_dresiarz/");
    });
}

// Shows detailed statistics for all players sorted by score
void FinishedGameApplication::showDetailedStats(Wt::WContainerWidget* container) {
    auto game = activeGames[gameId];
    const auto& players = game->getPlayers();

    // pomysł na rozwiązanie z stackoverflow
    std::vector<std::pair<int, std::shared_ptr<User>>> playersList;
    for (const auto& [userId, user] : players) {
        playersList.push_back({ userId, user });
    }

    std::sort(playersList.begin(), playersList.end(),
        [](const auto& a, const auto& b) {
            return a.second->getRespect() > b.second->getRespect();
        });

    // okresl pozycje z uwzglednieniem remisow
    std::vector<int> positions;
    int currentPos = 1;

    for (size_t i = 0; i < playersList.size(); i++) {
        if (i > 0 && playersList[i].second->getRespect() != playersList[i - 1].second->getRespect()) {
            currentPos = i + 1;
        }
        positions.push_back(currentPos);
    }

    // layout na statystyki
    auto statsContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    statsContainer->setStyleClass("final-stats-container");

    // pokaz kazdego gracza
    for (size_t i = 0; i < playersList.size(); i++) {
        auto user = playersList[i].second;
        int position = positions[i];
        bool isWinner = (position == 1);

        createPlayerStatsCard(statsContainer, user, position, isWinner);
    }
}

// Creates individual player card showing final stats
void FinishedGameApplication::createPlayerStatsCard(Wt::WContainerWidget* parent,
    std::shared_ptr<User> user, int position, bool isWinner)
{
    auto card = parent->addWidget(std::make_unique<Wt::WContainerWidget>());
    std::string cardClass = "stats-card";
    if (isWinner) {
        cardClass += " winner-card";
    }
    card->setStyleClass(cardClass);

    // Pozycje i nazwy gracza
    auto header = card->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("stats-header");

    auto positionBadge = header->addWidget(std::make_unique<Wt::WText>());
    if (isWinner) {
        positionBadge->setText("🥇 " + std::to_string(position) + "#");
    } else if (position == 2) {
        positionBadge->setText("🥈 " + std::to_string(position) + "#");
    } else if (position == 3) {
        positionBadge->setText("🥉 " + std::to_string(position) + "#");
    } else {
        positionBadge->setText(std::to_string(position) + "#");
    }
    positionBadge->setStyleClass("position-badge");

    auto playerName = header->addWidget(std::make_unique<Wt::WText>(user->getNickname()));
    playerName->setStyleClass("stats-player-name");

    // Statystyki gracza
    auto statsGrid = card->addWidget(std::make_unique<Wt::WContainerWidget>());
    statsGrid->setStyleClass("stats-grid");

    // Szacunek - wynik glowny
    auto respectStat = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    respectStat->setStyleClass("stat-box respect-main");
    auto respectIcon = respectStat->addWidget(std::make_unique<Wt::WText>("👑"));
    respectIcon->setStyleClass("stat-icon");
    auto respectLabel = respectStat->addWidget(std::make_unique<Wt::WText>("Szacun"));
    respectLabel->setStyleClass("stat-label");
    auto respectValue = respectStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getRespect())));
    respectValue->setStyleClass("stat-value main-stat");

    // Inne stastyki typu - kasa
    auto cashStat = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    cashStat->setStyleClass("stat-box");
    auto cashIcon = cashStat->addWidget(std::make_unique<Wt::WText>("💰"));
    cashIcon->setStyleClass("stat-icon");
    auto cashLabel = cashStat->addWidget(std::make_unique<Wt::WText>("Kasa"));
    cashLabel->setStyleClass("stat-label");
    auto cashValue = cashStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getCash())));
    cashValue->setStyleClass("stat-value");

    // BMW
    auto bmwStat = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    bmwStat->setStyleClass("stat-box");
    auto bmwIcon = bmwStat->addWidget(std::make_unique<Wt::WText>("🚗"));
    bmwIcon->setStyleClass("stat-icon");
    auto bmwLabel = bmwStat->addWidget(std::make_unique<Wt::WText>("BMW"));
    bmwLabel->setStyleClass("stat-label");
    auto bmwValue = bmwStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getBMW())));
    bmwValue->setStyleClass("stat-value");

    // Haracz
    auto extortionStat = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    extortionStat->setStyleClass("stat-box");
    auto extortionIcon = extortionStat->addWidget(std::make_unique<Wt::WText>("⚡"));
    extortionIcon->setStyleClass("stat-icon");
    auto extortionLabel = extortionStat->addWidget(std::make_unique<Wt::WText>("Haracz"));
    extortionLabel->setStyleClass("stat-label");
    auto extortionValue = extortionStat->addWidget(std::make_unique<Wt::WText>(std::to_string(user->getExtortion())));
    extortionValue->setStyleClass("stat-value");
}