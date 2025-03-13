#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Graphics/Text.hpp>
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

#include <NetworkAudio.h>

void handleFilePatch(TextForm_t *form, sf::Text& text);

int main() {
    sf::Font font;
    sf::SoundBuffer keyboardSoundBuffer;
    if (!keyboardSoundBuffer.loadFromFile("assets/keyboard_sound.ogg") ) {
        printf("Sound has not been loaded\n");
    }
    sf::Sound keyboardSound;
    keyboardSound.setBuffer(keyboardSoundBuffer);


    if (!font.loadFromFile("assets/Tektur.ttf")) {
        fprintf(stderr, "! No font found :(\n");
        return 1;
    }

    loadConfig(CONFIG_NAME);

    // printf("! Opened %s\n! Starting patching...\n", fileName);

    NetworkOggAudio radio;
    radio.open(keygen_FM);
    radio.play();
    radio.setVolume(100.f);
    printf("volume = %f\n", radio.getVolume());
    printf("is playing = %d\n", radio.getStatus() == sf::SoundStream::Playing);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(FPS_LIMIT);
    sf::Text text(L"Введите имя файла", font);
    text.setFillColor(sf::Color::White);
    text.setCharacterSize(75);
    text.setPosition(sf::Vector2f(WINDOW_WIDTH * 0.05f, WINDOW_HEIGHT * 0.7f));

    const int SHAPES_COUNT = 15;
    Triangle_t triangles[SHAPES_COUNT];
    sf::Vector2f baseDirection(MAX_AXIS_SPEED, MAX_AXIS_SPEED);
    sf::Color baseColor(128, 5, 10);
    for (int i = 0; i < SHAPES_COUNT; i++) {
        TriangleInit(&triangles[i], &window, baseDirection, baseColor );
    }
    TextForm_t form;
    textFormCtor(&form, &window, &font, sf::Vector2f(0.5f, 0.9f), sf::Vector2f(0.95f, 0.15f) );
    textFormSetVisible(&form, 1);

    // system("vlc assets/keygen_fm.ogg.m3u");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                textFormClickEventUpdate(&form);
            }

            if (event.type == sf::Event::KeyPressed) {
                textFormKeyUpdate(&form, event.key);
                // try to patch file with fiven name
                if (event.key.code == sf::Keyboard::Enter) {
                    handleFilePatch(&form, text);
                    // TriangleReverseSpeed(triangles, SHAPES_COUNT);
                    baseDirection = -baseDirection;
                }
            }

            if (event.type == sf::Event::TextEntered) {
                textFormUpdate(&form, event.text.unicode);
                keyboardSound.play();
                for (int i = 0; i < SHAPES_COUNT; i++) {
                    baseDirection = vecRotate(baseDirection, PI / 180);
                    baseColor.r += 15;
                    baseColor.g += 10;
                    baseColor.b += 5;
                    TriangleSetSpeedColor(&triangles[i], baseDirection, baseColor );
                }
            }
        }

        window.clear();
        for (int i = 0; i < SHAPES_COUNT; i++) {
            TriangleUpdate(&triangles[i], &window);
            TriangleDraw(&triangles[i], &window);
        }
        textFormDraw(&form);
        window.draw(text);

        window.display();
    }


    closeConfig();
    return 0;
}

void handleFilePatch(TextForm_t *form, sf::Text& text) {
    const wchar_t *fileName = textFormGetText(form);
    if (loadFile(fileName) != SUCCESS_EXIT) {
        text.setFillColor(sf::Color::Red);
        text.setString(L"Не могу открыть файл");
    } else {
        enum EXIT_CODES code = checkHashBeforePatch();
        if (code == ALREADY_PATCHED_EXIT) {
            text.setFillColor(sf::Color::Cyan);
            text.setString(L"Файл уже пропатчен");
            closeFile();
        }
        else if (code == BAD_HASH_EXIT) {
            text.setFillColor(sf::Color::Red);
            text.setString(L"Таблетка не предназначена для этого файла");
            closeFile();
        } else {

            patchCode(PatchTable, PatchSize);
            checkHashAfterPatch();
            closeFile();
            text.setFillColor(sf::Color::Green);
            text.setString(L"Всё готово. Йоу");
        }
    }
}
