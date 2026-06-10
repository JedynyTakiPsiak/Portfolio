#include "InvalidGameApplication.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>

InvalidGameApplication::InvalidGameApplication(const Wt::WEnvironment& env, const std::string& gameId)
    : Wt::WApplication(env) 
{
    useStyleSheet("assets/style.css");
    
    setTitle("Błąd - Mały Dresiarz");
    
    auto container = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("main-container slide-in");
    
    auto title = container->addWidget(std::make_unique<Wt::WText>("❌ BŁĄD"));
    title->setStyleClass("game-over-title");
    
    auto message = container->addWidget(std::make_unique<Wt::WText>(
        "Gra <strong>#" + gameId + "</strong> nie istnieje lub została usunięta."
    ));
    message->setTextFormat(Wt::TextFormat::UnsafeXHTML);
    message->setStyleClass("menu-description");
    
    auto redirectMessage = container->addWidget(std::make_unique<Wt::WText>(
        "🔄 Przekierowywanie do menu głównego za 3 sekundy..."
    ));
    redirectMessage->setStyleClass("waiting-message");
    
    auto timer = container->addChild(std::make_unique<Wt::WTimer>());
    timer->setInterval(std::chrono::seconds(3));
    timer->timeout().connect([this] {
        redirect("/maly_dresiarz/");
    });
    timer->start();
}