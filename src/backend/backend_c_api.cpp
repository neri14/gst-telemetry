#include "api/backend_c_api.h"
#include "manager.h"

extern "C" {

ManagerHandle* manager_new(void) {
    return reinterpret_cast<ManagerHandle*>(new telemetry::Manager());
}

void manager_free(ManagerHandle* handle) {
    delete reinterpret_cast<telemetry::Manager*>(handle);
}

void manager_init(ManagerHandle* handle, float offset) {
    reinterpret_cast<telemetry::Manager*>(handle)->init(offset);
}

void manager_deinit(ManagerHandle* handle) {
    reinterpret_cast<telemetry::Manager*>(handle)->deinit();
}

} // extern "C"
