#include "manager.h"

#include <iostream>

namespace telemetry {

Manager::Manager() {
    // Constructor implementation (if needed)
    std::cout << "Manager created\n";
};

Manager::~Manager() {
    // Destructor implementation (if needed)
    std::cout << "Manager destroyed\n";
};

void Manager::init(float offset) {
    // Initialization code using the offset
    std::cout << std::format("Manager initialized with offset {}\n", offset);
}

void Manager::deinit() {
    // Deinitialization code
    std::cout << "Manager deinitialized\n";
}

} // namespace telemetry