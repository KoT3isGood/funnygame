#pragma once
#include "stdint.h"
#include "cvar.h"
#include "cmd.h"






void fuck(const char* why);
char* strclone(const char *format, ...);
void print_hex(const unsigned char *data, long length);
double gettime();



void common_init();
void common_deinit();
