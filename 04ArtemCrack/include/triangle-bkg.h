#ifndef TRIANGLE_BKG
#define TRIANGLE_BKG

const float MAX_AXIS_SPEED = WINDOW_HEIGHT / 1 / FPS_LIMIT;
const float FORCE = 0.04;

typedef struct Triangle {
    sf::CircleShape shape;
    float rotateSpeed;
    float vel_x, vel_y;
} Triangle_t;

int TriangleInit(Triangle_t *triangle, sf::RenderWindow *window);
int TriangleUpdate(Triangle_t *triangle, sf::RenderWindow *window);
int TriangleDraw(Triangle_t *triangle, sf::RenderWindow *window);

#endif
