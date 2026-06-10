#pragma once

#include <Wt/WApplication.h>
#include <Wt/WText.h>

class MainApplication : public Wt::WApplication {
public:
    MainApplication(const Wt::WEnvironment& env);
private:
    Wt::WText* statusMsg_;
};

