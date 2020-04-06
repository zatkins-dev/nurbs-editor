#ifdef __APPLE__
#include "GLFW/glfw3.h"
#else
#if !defined(GL_GLEXT_PROTOTYPES)
#define GL_GLEXT_PROTOTYPES
#endif // GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#endif