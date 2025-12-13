#ifndef MANAGER_H
#define MANAGER_H

#include "backend/utils/logging/logger.h"

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    void init(float offset);
    void deinit();

private:
    utils::logging::Logger log{"manager"};
};

} // namespace telemetry

#endif // MANAGER_H