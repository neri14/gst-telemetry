#ifndef MANAGER_H
#define MANAGER_H

namespace telemetry {

class Manager {
public:
    Manager();
    ~Manager();

    void init(float offset);
    void deinit();

private:
};

} // namespace telemetry

#endif // MANAGER_H