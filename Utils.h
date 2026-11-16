#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

template <typename T>
T readValue(const std::string& message, T minValue, T maxValue) {
    while (true) {
        std::cout << message;

        std::string line;
        if (!std::getline(std::cin, line)) std::exit(0);

        std::stringstream input(line);
        T value;
        char extra;

        if ((input >> value) && !(input >> extra) &&
            value >= minValue && value <= maxValue) {
            return value;
        }

        std::cout << "Enter a value from " << minValue
                  << " to " << maxValue << ".\n";
    }
}

std::string readLine(const std::string& message);
