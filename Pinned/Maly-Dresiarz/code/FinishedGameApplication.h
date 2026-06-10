#ifndef FINISHEDGAMEAPPLICATION_H
#define FINISHEDGAMEAPPLICATION_H

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <string>
#include <memory>

#include "User.h"

// Application shown when game is finished
// displays final results and winner
class FinishedGameApplication : public Wt::WApplication {
public:
    FinishedGameApplication(const Wt::WEnvironment& env, const std::string& gameId);
    ~FinishedGameApplication();

private:
    std::string gameId;
    void showFinishedGameUI(); // Main UI setup for game over screen


    void showDetailedStats(Wt::WContainerWidget* container); // Show player ranking and stats
    void createPlayerStatsCard(Wt::WContainerWidget* parent,
    std::shared_ptr<User> user, int position, bool isWinner); // Create card for each player with their final scores

    Wt::WPushButton *goBackButton = nullptr; // Button to return to main menu
};

#endif
