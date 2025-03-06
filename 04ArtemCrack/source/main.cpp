#include <SFML/Window/Keyboard.hpp>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <math.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <globals.h>
#include <sf-textform.h>
#include <triangle-bkg.h>
#include <patcher.h>

int main() {
    sf::Font font;
    if (!font.loadFromFile("assets/Tektur.ttf")) {
        fprintf(stderr, "! No font found :(\n");
        return 1;
    }

    loadConfig(CONFIG_NAME);

    // printf("! Opened %s\n! Starting patching...\n", fileName);


    // printf("! Done \n$ Your executable is patched! $\n");

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(FPS_LIMIT);
    sf::Text text(L"Взламываю файл", font);

    const int SHAPES_COUNT = 15;
    Triangle_t triangles[SHAPES_COUNT];
    sf::Vector2f baseDirection(MAX_AXIS_SPEED, MAX_AXIS_SPEED);
    sf::Color baseColor(128, 5, 10);
    for (int i = 0; i < SHAPES_COUNT; i++) {
        TriangleInit(&triangles[i], &window, baseDirection, baseColor );
    }
    TextForm_t form;
    textFormCtor(&form, &window, &font, sf::Vector2f(0.5f, 0.8f), sf::Vector2f(0.3f, 0.2f) );
    textFormSetVisible(&form, 1);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                textFormClickEventUpdate(&form);
            }

            if (event.type == sf::Event::KeyPressed) {
                textFormKeyUpdate(&form, event.key);
                if (event.key.code == sf::Keyboard::Enter) {
                    const wchar_t *fileName = textFormGetText(&form);
                    if (loadFile(fileName) != SUCCESS_EXIT) {
                        text.setString(L"Не могу открыть файл");
                    } else {
                        enum EXIT_CODES code = checkHashBeforePatch();
                        if (code == ALREADY_PATCHED_EXIT) {
                            text.setString(L"Файл уже пропатчен");
                            closeFile();
                        }
                        else if (code == BAD_HASH_EXIT) {
                            text.setString(L"Таблетка не предназначена для этого файла");
                            closeFile();
                        } else {

                            patchCode(PatchTable, PatchSize);
                            checkHashAfterPatch();
                            closeFile();
                            text.setString(L"Всё готово. Йоу");
                        }
                    }
                }
            }

            if (event.type == sf::Event::TextEntered) {
                textFormUpdate(&form, event.text.unicode);

                for (int i = 0; i < SHAPES_COUNT; i++) {
                    baseDirection = vecRotate(baseDirection, PI / 180);
                    baseColor += sf::Color(10, 10, 10);
                    TriangleSetSpeedColor(&triangles[i], baseDirection, baseColor );
                }
            }
        }

        window.clear();
        for (int i = 0; i < SHAPES_COUNT; i++) {
            TriangleUpdate(&triangles[i], &window);
            TriangleDraw(&triangles[i], &window);
        }
        window.draw(text);
        textFormDraw(&form);
        window.display();
    }


    closeConfig();

    return 0;
}
