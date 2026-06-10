#include "JoinGameApplication.h"
#include "Globals.h"
#include "Game.h"
#include "User.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>
#include <Wt/Http/Cookie.h>

JoinGameApplication::JoinGameApplication(const Wt::WEnvironment& env, const std::string& gameId)
    : Wt::WApplication(env), gameId(gameId)
{
    useStyleSheet("assets/style.css");
    
    setTitle("Dołącz do gry #" + gameId);
    setInternalPath("/game/" + gameId, true);
    
    const std::string* cookie = env.getCookie(gameId);
    if (!cookie) {
        createUserUI(env);
    } else {
        showWaitingUI(env);
    }
}

void JoinGameApplication::showWaitingUI(const Wt::WEnvironment& env)
{
    auto container = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("main-container join-container slide-in");
    
    auto title = container->addWidget(std::make_unique<Wt::WText>("🎮 MAŁY DRESIARZ"));
    title->setStyleClass("join-title");
    
    auto message = container->addWidget(std::make_unique<Wt::WText>(
        "✅ Już dołączyłeś do gry!<br/>Oczekiwanie na innych graczy..."
    ));
    message->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    message->setStyleClass("waiting-message");
    
    auto urlContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    urlContainer->setStyleClass("join-url");
    
    auto urlLabel = urlContainer->addWidget(std::make_unique<Wt::WText>("🔗 Udostępnij znajomym:"));
    urlLabel->setStyleClass("nickname-label");
    
    auto urlText = urlContainer->addWidget(std::make_unique<Wt::WText>(
        env.urlScheme() + "://" + env.hostName() + "/maly_dresiarz/game/" + gameId
    ));
    urlText->setStyleClass("join-message");
}

void JoinGameApplication::createUserUI(const Wt::WEnvironment& env)
{
    root()->clear();

    auto container = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("main-container join-container slide-in");

    auto title = container->addWidget(std::make_unique<Wt::WText>("🎮 MAŁY DRESIARZ"));
    title->setStyleClass("join-title");

    auto gameInfo = container->addWidget(std::make_unique<Wt::WText>(
        "Dołączasz do gry <strong>#" + gameId + "</strong>"
    ));
    gameInfo->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    gameInfo->setStyleClass("menu-description");

    auto urlContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    urlContainer->setStyleClass("join-url");

    std::string ipAdresGry = env.urlScheme() + "://" + env.hostName() + "/maly_dresiarz/game/" + gameId;
    std::string shareText = "🔗 Kliknij, skopiuj i udostępnij znajomym:";

    auto urlLabel = urlContainer->addWidget(std::make_unique<Wt::WText>(shareText));
    urlLabel->setStyleClass("nickname-label");

    auto urlText = urlContainer->addWidget(std::make_unique<Wt::WText>(ipAdresGry));

    auto urlCopied = urlContainer->addWidget(std::make_unique<Wt::WText>());
    urlCopied->setStyleClass("urlCopied");
    urlCopied->hide();

    urlContainer->clicked().connect([=] {
        urlLabel->hide();
        urlText->hide();

        urlCopied->setText("Pomyślnie skopiowano!");
        urlCopied->show();
        urlContainer->doJavaScript("navigator.clipboard.writeText('" + ipAdresGry + "');");

        auto hidetimer = urlContainer->addChild(std::make_unique<Wt::WTimer>());
        hidetimer->setInterval(std::chrono::milliseconds(3000));
        hidetimer->timeout().connect([=] {
            urlCopied->setText("");
            urlCopied->hide();
            urlLabel->show();
            urlText->show();
            hidetimer->stop();
        });
        hidetimer->start();
    });

    
    // Nickname input
    auto nicknameLabel = container->addWidget(std::make_unique<Wt::WText>("👤 Jak chcesz się nazywać?"));
    nicknameLabel->setStyleClass("nickname-label");
    
    nicknameInput = container->addWidget(std::make_unique<Wt::WLineEdit>());
    nicknameInput->setStyleClass("nickname-input");
    nicknameInput->setPlaceholderText("Wpisz swój nick...");
    
    addingUserMessage = container->addWidget(std::make_unique<Wt::WText>());
    addingUserMessage->setStyleClass("join-message");
    addingUserMessage->hide();
    
    auto button = container->addWidget(std::make_unique<Wt::WPushButton>("🚀 DOŁĄCZ DO GRY"));
    button->setStyleClass("main-btn");
    
    button->clicked().connect([=] {
        auto currentGame = activeGames[gameId];
        std::string nickname = nicknameInput->text().toUTF8();
        
        addingUserMessage->setText("");
        addingUserMessage->show();
        
        if (nickname.empty()) {
            addingUserMessage->setText("❌ Musisz podać nick!");
            addingUserMessage->setStyleClass("join-message error-message");
            return;
        }
        
        if (currentGame->arePlayersAccepted()) {
            if (currentGame->isNicknameTaken(nickname)) {
                addingUserMessage->setText("❌ Nazwa gracza już istnieje!");
                addingUserMessage->setStyleClass("join-message error-message");
                return;
            }
            
            auto user = std::make_shared<User>(nickname, gameId, currentGame->getNumberOfPlayersJoined());
            setUserCookie(user->getFullUserID());
            currentGame->addPlayer(user);
            
            button->setDisabled(true);
            nicknameInput->setDisabled(true);
            
            addingUserMessage->setText("✅ Pomyślnie dołączono! Oczekiwanie na graczy...");
            addingUserMessage->setStyleClass("join-message");
            
            auto waitTimer = container->addChild(std::make_unique<Wt::WTimer>());
            waitTimer->setInterval(std::chrono::seconds(1));
            waitTimer->timeout().connect([=] {
                if (currentGame->getNumberOfPlayersJoined() >= currentGame->getNumberOfPlayers()) {
                    doJavaScript("window.location.reload(true);");
                    waitTimer->stop();
                }
            });
            waitTimer->start();
        } else {
            container->clear();
            
            auto errorContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
            errorContainer->setStyleClass("main-container");
            
            auto errorTitle = errorContainer->addWidget(std::make_unique<Wt::WText>("BRAK MIEJSC!"));
            errorTitle->setStyleClass("join-title");
            
            auto errorMsg = errorContainer->addWidget(std::make_unique<Wt::WText>(
                "Niestety, w tej grze nie ma już wolnych miejsc.<br/>Przekierowywanie do menu głównego..."
            ));
            errorMsg->setTextFormat(Wt::TextFormat::UnsafeXHTML);
            errorMsg->setStyleClass("waiting-message");
            
            auto timer = errorContainer->addChild(std::make_unique<Wt::WTimer>());
            timer->setInterval(std::chrono::seconds(3));
            timer->timeout().connect([this] {
                redirect("/maly_dresiarz/");
            });
            timer->start();
        }
    });
}

void JoinGameApplication::setUserCookie(const std::string& fullUserId)
{
    std::string gameId = fullUserId.substr(0, 8);
    std::string userId = fullUserId.substr(9);
    Wt::Http::Cookie cookie(gameId, userId);
    cookie.setMaxAge(std::chrono::seconds(60 * 60 * 24));
    setCookie(cookie);
}