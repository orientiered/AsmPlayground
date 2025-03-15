#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
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

const int SHAPES_COUNT = 15;

typedef struct windowLayout_t {
    sf::Font font;                          ///< Font for all text
    sf::SoundBuffer keyboardSoundBuffer;    ///< Sound buffer for keyboard presses
    sf::Sound keyboardSound;                ///< Sound that is played when keyboard is pressed

    sf::Text statusText;                    ///< Text for displaying info about cracking
    TextForm_t form;                        ///< Text field
    NetworkOggAudio radio;                  ///< Object that plays audio in Ogg from internet

    Triangle_t triangles[SHAPES_COUNT];     ///< Array with spinning and moving triangles
    sf::Vector2f baseDirection;             ///< Average triangles moving direction
    sf::Color    baseColor;                 ///< Average triangles color

    sf::RenderWindow window;                ///< Window for rendering

    bool hasConfig;
} windowLayout_t;

const char * const FONT_PATH = "assets/Tektur.ttf";
const char * const KEYBOARD_SOUND_PATH = "assets/keyboard_sound.ogg";
const wchar_t * const DEFAULT_STATUS_TEXT = L"Введите имя файла";
const wchar_t * const NO_CONFIG_STATUS_TEXT = L"Нет конфига: проверка хеша отключена";

// Return 0 on success, 1 on error
int initLayout(windowLayout_t *layout);

void layoutDrawLoop(windowLayout_t *layout);

int main() {

    enum EXIT_CODES configStatus = loadConfig(CONFIG_NAME);

    windowLayout_t layout; // this struct contains multiple class object, so default initialization is not feasible
    layout.hasConfig = (configStatus == NO_CONFIG_MODE) ? false
                                                        : true;

    if (initLayout(&layout)) {
        fprintf(stderr, "Layout initialization failed\n");
        closeConfig();
        return 1;
    }

    layoutDrawLoop(&layout);

    closeConfig();
    return 0;
}

void updateTrianglesOnTextEntered(Triangle_t *triangles, sf::Vector2f *baseDirection, sf::Color *baseColor) {
    for (int i = 0; i < SHAPES_COUNT; i++) {
        *baseDirection = vecRotate(*baseDirection, PI / 180);
        baseColor->r += 15;
        baseColor->g += 10;
        baseColor->b += 5;
        TriangleSetSpeedColor(&triangles[i], *baseDirection, *baseColor);
    }
}

void layoutDrawLoop(windowLayout_t *layout) {
    while (layout->window.isOpen())
    {
        sf::Event event;
        while (layout->window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                layout->window.close();
                break;
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                textFormClickEventUpdate(&layout->form);
            }

            if (event.type == sf::Event::KeyPressed) {
                textFormKeyUpdate(&layout->form, event.key);
                // try to patch file with fiven name
                if (event.key.code == sf::Keyboard::Enter) {
                    handleFilePatch(&layout->form, layout->statusText);
                    // TriangleReverseSpeed(triangles, SHAPES_COUNT);
                    layout->baseDirection = -layout->baseDirection;
                }
            }

            if (event.type == sf::Event::TextEntered) {
                textFormUpdate(&layout->form, event.text.unicode);
                layout->keyboardSound.play();

                updateTrianglesOnTextEntered(layout->triangles, &layout->baseDirection, &layout->baseColor);
            }
        }

        layout->window.clear();
        for (int i = 0; i < SHAPES_COUNT; i++) {
            TriangleUpdate(&layout->triangles[i], &layout->window);
            TriangleDraw(&layout->triangles[i], &layout->window);
        }
        textFormDraw(&layout->form);
        layout->window.draw(layout->statusText);

        layout->window.display();
    }
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

int initLayout(windowLayout_t *layout) {
    //loading font
    if (!layout->font.loadFromFile(FONT_PATH)) {
        fprintf(stderr, "! No font found :(\n");
        return 1;
    }

    //initializing window
    layout->window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    layout->window.setFramerateLimit(FPS_LIMIT);

    // initializing status text
    layout->statusText.setFont(layout->font);
    if (layout->hasConfig)
        layout->statusText.setString(DEFAULT_STATUS_TEXT);
    else
        layout->statusText.setString(NO_CONFIG_STATUS_TEXT);

    layout->statusText.setFillColor(sf::Color::White);
    layout->statusText.setCharacterSize(75);
    layout->statusText.setPosition(sf::Vector2f(WINDOW_WIDTH * 0.05f, WINDOW_HEIGHT * 0.7f));

    // initializing text form
    textFormCtor(&layout->form, &layout->window, &layout->font, sf::Vector2f(0.5f, 0.9f), sf::Vector2f(0.95f, 0.15f) );
    textFormSetVisible(&layout->form, 1);

    // initiazling background triangles animation
    layout->baseDirection = sf::Vector2f(MAX_AXIS_SPEED, MAX_AXIS_SPEED);
    layout->baseColor     = sf::Color(128, 5, 10);
    for (int i = 0; i < SHAPES_COUNT; i++) {
        TriangleInit(&layout->triangles[i], &layout->window, layout->baseDirection, layout->baseColor );
    }

    //loading sound
    if (!layout->keyboardSoundBuffer.loadFromFile(KEYBOARD_SOUND_PATH) ) {
        printf("Sound has not been loaded\n");
    }
    layout->keyboardSound.setBuffer(layout->keyboardSoundBuffer);

    // initializing radio
    layout->radio.open(keygen_FM);
    layout->radio.play();

    return 0;
}
