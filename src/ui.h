#ifndef __UI_H__
#define __UI_H__

#include <GLFW/glfw3.h>

#include "app.h"
#include "world.h"

GLFWwindow *ui_init(App *app, World *world);
void ui_exit(GLFWwindow *window);


#endif
