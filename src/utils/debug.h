#ifndef DEBUG_H
#define DEBUG_H

#include <GL/glew.h>
#include <iostream>

inline void glErrorCheck(const char* filename, int line) {
    GLenum errNum = glGetError();
    while (errNum != GL_NO_ERROR) {
        std::cout << "Encountered error in " << filename << " on line " << line << ": " << gluErrorString(errNum) << " (" << errNum << ")" << std::endl;
        errNum = glGetError();
    }
}

#define glErrorCheck(glCall) glCall; glErrorCheck(__FILE__, __LINE__)

#endif // DEBUG_H
