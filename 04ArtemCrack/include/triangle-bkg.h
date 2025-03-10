#ifndef TRIANGLE_BKG
#define TRIANGLE_BKG

#include <globals.h>
#include <SFML/Graphics.hpp>

const float MAX_AXIS_SPEED = WINDOW_HEIGHT / 1 / FPS_LIMIT;
const float FORCE = 0.04;

typedef struct Triangle {
    sf::CircleShape shape;
    float rotateSpeed;
    float vel_x, vel_y;
} Triangle_t;

int TriangleInit(Triangle_t *triangle, sf::RenderWindow *window, sf::Vector2f baseDirection, sf::Color baseColor);

int TriangleSetSpeedColor(Triangle_t *triangle,
                          sf::Vector2f baseDirection,    // main direction for triangles movement
                          sf::Color baseColor );         // average color of triangles)

int TriangleReverseSpeed(Triangle_t *triangle, size_t count);

int TriangleUpdate(Triangle_t *triangle, sf::RenderWindow *window);
int TriangleDraw(Triangle_t *triangle, sf::RenderWindow *window);

#endif
