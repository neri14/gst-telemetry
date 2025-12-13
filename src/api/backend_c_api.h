#ifndef BACKEND_C_API_H
#define BACKEND_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ManagerHandle ManagerHandle;

ManagerHandle* manager_new(void);
void manager_free(ManagerHandle* handle);
void manager_init(ManagerHandle* handle, float offset);
void manager_deinit(ManagerHandle* handle);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BACKEND_C_API_H
