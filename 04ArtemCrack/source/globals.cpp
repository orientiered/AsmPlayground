#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <globals.h>
#include <math.h>

float getRandom(float lower, float upper) {
    float rnd_01 = (float) rand() / RAND_MAX;
    return lower + rnd_01 * (upper - lower);
}

int getRandomInt(int lower, int upper) {
    return lower + rand() % (upper - lower);
}

// DJB2 hash //link
uint64_t memHash(const void *arr, size_t len) {
    if (!arr) return 0x1DED0BEDBAD0C0DE;
    uint64_t hash = 5381;
    const unsigned char *carr = (const unsigned char*)arr;
    while (len--)
        hash = ((hash << 5) + hash) + *carr++;
        //hash = 33*hash + c
    return hash;
}

sf::Vector2f normalize(sf::Vector2f vec) {
    float norm = vecLength(vec);
    vec.x /= norm;
    vec.y /= norm;
    return vec;
}

// return rotated clockwise vector by angle
sf::Vector2f vecRotate(sf::Vector2f vec, float angle) {
    // x' = cos(a) * x - sin(a) * y
    // y' = sin(a) * x + cos(a) * y
    return sf::Vector2f(vec.x * cos(angle) - vec.y * sin(angle),
                        vec.x * sin(angle) + vec.y * cos(angle)  );
}

float vecLength(sf::Vector2f& vec) {
    return sqrt(vec.x * vec.x + vec.y * vec.y);

}
