#ifndef SETTINGS_H
#define SETTINGS_H

enum class Integrator {
    EULER,
    RK4
};

struct Settings {
    float nearPlane = 0.1;
    float farPlane = 100;

    int bounds = 4;
    double dt = 1; // simulation timestep (ms)
    double kElastic = 500; // Hook's elasticity coefficient for all springs except collision springs
    double dElastic = 0.25; // Dampening coefficient for all springs except collision springs
    double kCollision = 10000; // Hook's elasticity coefficient for collision springs
    double dCollision = 0.25; // Dampening coefficient collision springs
    double mass = 0.01; // mass of each node (equal for all nodes)
    double gravity = 100; // gravity (acceleration downwards)
    Integrator integrator = Integrator::RK4;

    bool textureMappingEnabled = false;
    bool transparentCube = false;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
