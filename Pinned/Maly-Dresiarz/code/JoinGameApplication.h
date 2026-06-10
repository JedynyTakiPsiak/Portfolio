#pragma once

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WLineEdit.h>
#include <string>

class JoinGameApplication : public Wt::WApplication
{
public:
    JoinGameApplication(const Wt::WEnvironment& env, const std::string& gameId);
    void createUserUI(const Wt::WEnvironment& env);
    void showWaitingUI(const Wt::WEnvironment& env);
    void setUserCookie(const std::string& fullUserId);

private:
    std::string gameId;
    std::string nickname;
    Wt::WLineEdit* nicknameInput = nullptr;
    Wt::WText* addingUserMessage = nullptr; // Status messages (success/error)
};