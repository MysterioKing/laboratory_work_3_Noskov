#include "Utils.h"

std::string readLine(const std::string& message) {
    std::string value;

    while (value.find_first_not_of(" \t") == std::string::npos) {
        std::cout << message;
        if (!std::getline(std::cin, value)) std::exit(0);

        if (value.find_first_not_of(" \t") == std::string::npos)
            std::cout << "Value cannot be empty.\n";
    }

    return value;
}
