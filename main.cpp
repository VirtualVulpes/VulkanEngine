#include <iostream>

#include "engine.h"

int main() {
    try {
        Engine engine{};
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
