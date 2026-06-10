#pragma once

#include <Wt/WApplication.h>
#include <string>

class InvalidGameApplication : public Wt::WApplication {
public:
    InvalidGameApplication(const Wt::WEnvironment& env, const std::string& gameId);
};
