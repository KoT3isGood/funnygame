#pragma once
#include "window.h"
#include "render.h"
#include "stdbool.h"

void client_init();
void client_frame();
bool client_shouldrun();
void client_deinit();
