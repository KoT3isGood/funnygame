#pragma once
typedef void(*frame_f)(float deltatime);
typedef void(*general_f)(void);

typedef struct {
  struct entity* next;
  general_f init;
  general_f deinit;
  frame_f frame;
  frame_f fixedframe;
} entity;

