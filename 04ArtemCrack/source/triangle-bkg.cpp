#include <stdio.h>
#include <stdlib.h>

#include <SFML/Graphics.hpp>
#include <globals.h>
#include <triangle-bkg.h>

int TriangleInit(Triangle_t *triangle, sf::RenderWindow *window) {
    if (!triangle || !window) {
        return -1;
    }

    const int width = window->getSize().x;
    const int height = window->getSize().y;

    float pos_x = getRandom(0, width),
          pos_y = getRandom(0, height);

    triangle->shape.setPosition(pos_x, pos_y);
    triangle->shape.setPointCount(3);
    float radius = getRandom(40, 70);
    triangle->shape.setRadius(radius);
    triangle->shape.setOrigin(sf::Vector2f(radius, radius));
    triangle->shape.setRotation(getRandom(0, 2*PI));
    triangle->shape.setFillColor( sf::Color(128, getRandomInt(0, 55), getRandomInt(0, 55)) );

    triangle->rotateSpeed = getRandom(0, 360 / 15);
    triangle->vel_x = getRandom(MAX_AXIS_SPEED / 10, MAX_AXIS_SPEED);
    triangle->vel_y = getRandom(MAX_AXIS_SPEED / 10, MAX_AXIS_SPEED);

    return 0;
}

int TriangleUpdate(Triangle_t *triangle, sf::RenderWindow *window) {
    if (!triangle)
        return -1;

    // bounds check
    sf::Vector2f pos = triangle->shape.getPosition();
    if (pos.x > WINDOW_WIDTH) pos.x = 0;
    if (pos.x < 0) pos.x = WINDOW_WIDTH;

    if (pos.y > WINDOW_HEIGHT) pos.y = 0;
    if (pos.y < 0) pos.y = WINDOW_HEIGHT;

    triangle->shape.setPosition(pos);

    // mouse circular force
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    if (mousePressed) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        sf::Vector2f mousePosf = sf::Vector2f(mousePos);
        sf::Vector2f rVec = pos - mousePosf;
        sf::Vector2f ForceVec(-rVec.y, rVec.x);
        ForceVec = normalize(ForceVec) * FORCE;
        triangle->vel_x += ForceVec.x;
        triangle->vel_y += ForceVec.y;
        // printf("Mouse pressed, v = %f %f\n", triangle->vel_x, triangle->vel_y);
    }

    // moving and rotating
    triangle->shape.rotate(triangle->rotateSpeed);
    triangle->shape.move(triangle->vel_x, triangle->vel_y);

    // adding some noise
    triangle->vel_x += getRandom(- MAX_AXIS_SPEED/100, MAX_AXIS_SPEED/100);
    triangle->vel_y += getRandom(- MAX_AXIS_SPEED/100, MAX_AXIS_SPEED/100);


    return 0;
}

int TriangleDraw(Triangle_t *triangle, sf::RenderWindow *window) {
    if (!triangle || !window)
        return -1;
    window->draw(triangle->shape);

    return 0;
}
