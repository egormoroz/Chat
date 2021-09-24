#pragma once

#include <iostream>

enum class LT {
    INFO,
    ERR
};

template<LT lt>
std::ostream& logger() {
    if (lt == LT::INFO) {
        std::cout << "[  INFO ] ";
    } else if (lt == LT::ERR) {
        std::cout << "[ ERROR ] ";
    }
    return std::cout;
}

