#ifndef MANAGER_C_API_H
#define MANAGER_C_API_H

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

#endif // MANAGER_C_API_H