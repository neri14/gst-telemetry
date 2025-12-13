#include "manager.h"

#include <iostream>

namespace telemetry {

Manager::Manager() {
    // Constructor implementation (if needed)
    log.info("Manager created");
};

Manager::~Manager() {
    // Destructor implementation (if needed)
    log.info("Manager destroyed");
};

void Manager::init(float offset) {
    // Initialization code using the offset
    log.info("Manager initialized with offset {}", offset);
}

void Manager::deinit() {
    // Deinitialization code
    log.info("Manager deinitialized");
}

} // namespace telemetry