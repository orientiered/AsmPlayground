#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <globals.h>
#include <sf-textform.h>
#include <triangle-bkg.h>

enum EXIT_CODES {
    NO_FILE_EXIT = 2,
    OPEN_ERROR_EXIT = 3,
    ADDR_ERROR_EXIT = 4,
    WRITE_ERROR_EXIT = 5,
    CLOSE_ERROR_EXIT = 6,
    BAD_HASH_EXIT = 7
};

typedef struct BytePatch {
    size_t addr;
    uint8_t replacement;
} BytePatch_t;


BytePatch_t PatchTable[] = {
    {0x1, 0x29}
};

const size_t PatchSize = sizeof(PatchTable) / sizeof(PatchTable[0]);

const char * const CONFIG_NAME = "patch.cfg";

int main(int argc, char *argv[]) {
    sf::Font font;
    if (!font.loadFromFile("assets/Monomakh-Regular.ttf")) {
        fprintf(stderr, "! No font found, working in console version\n");
    }

    FILE *config = fopen(CONFIG_NAME, "r+");
    uint64_t originalHash = 0u, patchedHash = 0u;
    bool hasConfig = true;
    if (!config) {
        printf("! No config found, assuming all file choices are correct\n");
        config = fopen(CONFIG_NAME, "w");
        hasConfig = false;
    } else {
        fscanf(config, "%ju %ju", &originalHash, &patchedHash);
    }

    if (argc == 1) {
        fprintf(stderr, "No file selected\n");
        return NO_FILE_EXIT;
    }

    const char *fileName = argv[1];
    FILE *file = fopen(fileName, "rb+");
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", fileName);
        return OPEN_ERROR_EXIT;
    }

    fseek(file, 0, SEEK_END);
    long int fileLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *fileContents = (uint8_t *) calloc(fileLength, 1);
    fread(fileContents, fileLength, 1, file);
    uint64_t fileHash = memHash(fileContents, fileLength);
    printf("! File hash: %#jX\n", fileHash);

    if (!hasConfig) {
        fprintf(config, "%ju\n", fileHash);
    } else {
        if (fileHash == originalHash)
            printf("! Originality of the file confirmed!\n");
        else if (fileHash == patchedHash)
            printf("! Warning: this program has already been patched\n");
        else {
           fprintf(stderr, "This file is not original. If you want to patch this file, delete %s\n", CONFIG_NAME);
           fclose(config);
           fclose(file);
           free(fileContents);
           return BAD_HASH_EXIT;
        }
    }


    printf("! Opened %s\n! Starting patching...\n", fileName);

    for (size_t byte_idx = 0; byte_idx < PatchSize; byte_idx++) {
        size_t addr = PatchTable[byte_idx].addr;
        if (addr >= fileLength) {
            fprintf(stderr, "! Error: Invalid address %zu\n", addr);
            return ADDR_ERROR_EXIT;
        }

        fileContents[addr] = PatchTable[byte_idx].replacement;
    }

    printf("! Patching complete. Saving changes...\n");
    uint64_t newHash = memHash(fileContents, fileLength);
    printf("! Hash after patching: %#jX\n", newHash);

    if (!hasConfig)
        fprintf(config, "%ju\n", newHash);
    else if (newHash != patchedHash) {
        fprintf(stderr, "Something went wrong: hash of patched program doesn't match with reference hash\n");
        return BAD_HASH_EXIT;
    }

    fseek(file, 0, SEEK_SET);
    fwrite(fileContents, fileLength, 1, file);
    free(fileContents);
    if (fclose(file) == EOF) {
        fprintf(stderr, "Failed to close file %s\n", fileName);
        return CLOSE_ERROR_EXIT;
    }
    if (fclose(config) == EOF) {
        fprintf(stderr, "Failed to close file %s\n", CONFIG_NAME);
        return CLOSE_ERROR_EXIT;
    }

    printf("! Done \n$ Your executable is patched! $\n");

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(FPS_LIMIT);
    sf::Text text(L"Взламываю файл", font);

    const int SHAPES_COUNT = 15;
    Triangle_t triangles[SHAPES_COUNT];
    for (int i = 0; i < SHAPES_COUNT; i++) {
        TriangleInit(&triangles[i], &window);
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

            if (event.type == sf::Event::TextEntered) {
                textFormUpdate(&form, event.key.code);
                for (int i = 0; i < SHAPES_COUNT; i++) {
                    TriangleInit(&triangles[i], &window);
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

    return 0;
}
