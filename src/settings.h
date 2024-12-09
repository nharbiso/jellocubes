#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 1;
    float farPlane = 1;
    bool perPixelFilter1 = false;
    bool perPixelFilter2 = false;
    bool perPixelFilter3 = false;
    bool kernelBasedFilter1 = false;
    bool kernelBasedFilter2 = false;
    bool kernelBasedFilter3 = false;
    bool extraCredit1 = false;
    bool extraCredit2 = false;
    bool extraCredit3 = false;
    bool extraCredit4 = false;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

extern int bounds;

#endif // SETTINGS_H
