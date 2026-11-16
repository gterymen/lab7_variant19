#pragma once

#include <iostream>
#include <string>
#include <utility>

// RAII helper: reports creation and destruction of the object.
class ScopeLogger
{
private:
    std::string name_;

public:
    explicit ScopeLogger(std::string name)
        : name_(std::move(name))
    {
        std::cout << "[ctor] " << name_ << '\n';
    }

    ~ScopeLogger()
    {
        std::cout << "[dtor] " << name_ << '\n';
    }

    ScopeLogger(const ScopeLogger&) = delete;
    ScopeLogger& operator=(const ScopeLogger&) = delete;
};
