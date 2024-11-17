#ifndef __UI_H__
#define __UI_H__

#include <GLFW/glfw3.h>

#include "app.h"
#include "world.h"

void ui_init(App *app, World *world);
void ui_exit(GLFWwindow *window);

void gui_init(App *app);
int gui_draw(App *app, World *world);
void gui_exit(struct nk_glfw *gui);
#endif
