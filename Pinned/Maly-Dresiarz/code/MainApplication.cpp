#include "MainApplication.h"
#include "Game.h"
#include "Globals.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>
#include <Wt/WBreak.h>
#include <Wt/WString.h>
#include <Wt/WComboBox.h>


MainApplication::MainApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env) 
{
    useStyleSheet("assets/style.css");
    
    setTitle("Mały Dresiarz - Menu Główne");
    
    // Main container
    auto container = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("main-container slide-in");
    
    // Game title
    auto title = container->addWidget(std::make_unique<Wt::WText>("🎮 MAŁY DRESIARZ"));
    title->setStyleClass("menu-title");
    
    // Description
    auto description = container->addWidget(std::make_unique<Wt::WText>(
        "Zostań najbardziej szanowanym dresiarzem w dzielnicy!"
        "<br/>Gra turowa dla 2-4 graczy z elementami strategii."
    ));
    description->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    description->setStyleClass("menu-description");
    
    // Player count container
    auto playerCountContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    playerCountContainer->setStyleClass("player-count-container");

    // Top row - labels
    auto labelsRow = playerCountContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    labelsRow->setStyleClass("labels-row");

    auto playerCountLabel = labelsRow->addWidget(std::make_unique<Wt::WText>("Liczba graczy (1-4):"));
    playerCountLabel->setStyleClass("player-count-label");

    auto botCountLabel = labelsRow->addWidget(std::make_unique<Wt::WText>("Liczba botów (0-3):"));
    botCountLabel->setStyleClass("bot-count-label");

    // Second row - controls
    auto controlsRow = playerCountContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    controlsRow->setStyleClass("controls-row");

    auto playerspinBox = controlsRow->addWidget(std::make_unique<Wt::WSpinBox>());
    playerspinBox->setStyleClass("player-count-input");
    playerspinBox->setMinimum(1);
    playerspinBox->setMaximum(4);
    playerspinBox->setValue(1);

    auto botspinBox = controlsRow->addWidget(std::make_unique<Wt::WSpinBox>());
    botspinBox->setStyleClass("player-count-input");
    botspinBox->setMinimum(0);
    botspinBox->setMaximum(3);
    botspinBox->setValue(3);

    // Third row - difficulty
    auto difficultyRow = playerCountContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    difficultyRow->setStyleClass("difficulty-row");

    auto difficultyLabel = difficultyRow->addWidget(std::make_unique<Wt::WText>("Poziom trudności botów:"));
    difficultyLabel->setStyleClass("difficulty-label");

    auto difficultyCombo = difficultyRow->addWidget(std::make_unique<Wt::WComboBox>());
    difficultyCombo->setStyleClass("difficulty-input");
    difficultyCombo->addItem("Łatwy");
    difficultyCombo->addItem("Normalny");
    difficultyCombo->addItem("Trudny");
    difficultyCombo->setCurrentIndex(0); // Default value

    // Update logic for spinboxes
    playerspinBox->changed().connect([=] {
        if (playerspinBox->value() < 1) playerspinBox->setValue(1);
        if (playerspinBox->value() > 4) playerspinBox->setValue(4);
        // always value of players (real + bot)  == 4
        if (botspinBox->value() + playerspinBox->value() != 4) {
            botspinBox->setValue(4 - playerspinBox->value());
        }
        });

    botspinBox->changed().connect([=] {
        if (botspinBox->value() < 0) botspinBox->setValue(0);
        if (botspinBox->value() > 3) botspinBox->setValue(3);
        if (botspinBox->value() + playerspinBox->value() != 4) {
            playerspinBox->setValue(4 - botspinBox->value());
        }
        });

    // Create game button
    auto button = container->addWidget(std::make_unique<Wt::WPushButton>("🚀 STWÓRZ GRĘ"));
    button->setStyleClass("main-btn");
    
    // Status message
    statusMsg_ = container->addWidget(std::make_unique<Wt::WText>());
    statusMsg_->setStyleClass("join-message");
    statusMsg_->hide();
    
    // Game info
    auto infoText = container->addWidget(std::make_unique<Wt::WText>(
        "💡 <strong>Zasady gry:</strong><br/>"
        "• Gra trwa 10 rund<br/>"
        "• W każdej rundzie wybierasz jedną z 5 akcji<br/>"
        "• Celem jest zdobycie największej ilości Szacunku<br/>"
        "• Zarządzaj Kasą, BMW, Haraczem i Szacunkiem<br/>"
        "<strong>Powodzenia, dresiarz! </strong>😎"
    ));
    infoText->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    infoText->setStyleClass("menu-description");
    
    // Button click handler
    button->clicked().connect([=] {
        int totalPlayers = playerspinBox->value() + botspinBox->value();
        auto newGame = std::make_shared<Game>(totalPlayers);

        //adding bots
        int difficulty = difficultyCombo->currentIndex(); // 0 = easy, 1 = normal, 2 = hard
        newGame->setBotDifficultyLevel(difficulty);

        for (int i = 0; i < botspinBox->value(); i++) {
            newGame->addBot(difficulty);
        }

        activeGames[newGame->getGameID()] = newGame;
        
        std::string gameUrl = "/maly_dresiarz/game/" + newGame->getGameID();
        statusMsg_->show();
        statusMsg_->setText("✅ Gra została stworzona dla " + std::to_string(playerspinBox->value()) 
            + " graczy i " + std::to_string(botspinBox->value()) + " botów!");
        
        button->setDisabled(true);
        
        auto timer = container->addChild(std::make_unique<Wt::WTimer>());
        timer->setInterval(std::chrono::seconds(1));
        timer->timeout().connect([=] {
            redirect(gameUrl);
        });
        timer->start();
    });
}

// moved from GameApplication.cpp
std::map<std::string, std::shared_ptr<Game>> activeGames;