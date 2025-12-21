#include "api/backend_c_api.h"
#include "manager.h"

extern "C" {

ManagerHandle* manager_new(void) {
    return reinterpret_cast<ManagerHandle*>(new telemetry::Manager());
}

void manager_free(ManagerHandle* handle) {
    delete reinterpret_cast<telemetry::Manager*>(handle);
}

int manager_init(ManagerHandle* handle, float offset, char* track, char* layout) {
    bool ok = reinterpret_cast<telemetry::Manager*>(handle)->init(offset, track, layout);
    return ok ? 0 : -1;
}

int manager_deinit(ManagerHandle* handle) {
    bool ok = reinterpret_cast<telemetry::Manager*>(handle)->deinit();
    return ok ? 0 : -1;
}

} // extern "C"
