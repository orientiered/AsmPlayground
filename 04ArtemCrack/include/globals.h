#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

const int WINDOW_HEIGHT = 1080;
const int WINDOW_WIDTH  = 1920;
const int FPS_LIMIT = 60;

const char * const WINDOW_TITLE = "Great Patcher Title";
const float PI = 3.1415;

uint64_t memHash(const void *arr, size_t len);

float getRandom(float lower, float upper);
int getRandomInt(int lower, int upper);
sf::Vector2f normalize(sf::Vector2f vec);


#endif
