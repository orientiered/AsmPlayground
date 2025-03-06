#include <SFML/Graphics/Glyph.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Clipboard.hpp>
#include "sf-textform.h"

//global lock on multiple form selection
static TextForm_t *selectedForm = NULL;

//TODO: this function is used in multiple files, remove copypaste
/// @brief convert from (0,1)^2 to (0, window.width)x(0, window.height)
static sf::Vector2f mapCoords(sf::Vector2f coords, const sf::RenderWindow *window) {
    sf::Vector2u windowSize = window->getSize();
    return sf::Vector2f(coords.x * windowSize.x, coords.y * windowSize.y);
}

static void moveCursorByGlyph(TextForm_t *form, bool right) {
    if (right && form->cursorPos == form->inputSize) return;
    if (!right && form->cursorPos == 0) return;

    wchar_t symbol = form->inputStr[form->cursorPos - (!right)];
    sf::Glyph glyph = form->label.getFont()->getGlyph(
        (uint32_t) symbol,
        TEXTFORM_FONT_SIZE,
        false
    );

    sf::Vector2f moveOffset(glyph.advance, 0);
    form->cursor.move(right ? moveOffset : -moveOffset);
    form->cursorPos += right ? 1 : -1;

}

static void moveCursorToStart(TextForm_t *form) {
    form->cursor.setPosition(form->label.getPosition());
    form->cursorPos = 0;
}
static void moveCursorToEnd(TextForm_t *form) {
    for (size_t pos = form->cursorPos; pos < form->inputSize; pos++) {
        moveCursorByGlyph(form, 1);
    }
}

// inserts character at cursorPos, moves cursor by one character right
// doesn't update label
static void insertCharacter(TextForm_t *form, wchar_t symbol) {
    if (form->inputSize == MAX_INPUT_LEN - 1) return;

    form->inputSize++;
    form->inputStr[form->inputSize] = L'\0';
    // moving part of the string to insert symbol
    for (size_t pos = form->inputSize - 1; pos > form->cursorPos; pos--) {
        form->inputStr[pos] = form->inputStr[pos - 1];
    }


    // writing symbol and moving cursor
    form->inputStr[form->cursorPos] = symbol;
    moveCursorByGlyph(form, 1);

}

// erases character at cursorPos - 1, moves cursor by one character left
static void eraseCharacter(TextForm_t *form) {
    if (form->inputSize == 0 || form->cursorPos == 0) return;

    // moving cursor
    // this needs to be done first, because character will be overwritten;
    moveCursorByGlyph(form, 0);

    // moving part of the string to erased symbol
    for (size_t pos = form->cursorPos; pos < form->inputSize; pos++) {
        form->inputStr[pos] = form->inputStr[pos + 1];
    }


    form->inputSize--;
    form->inputStr[form->inputSize] = L'\0';

}

void textFormCtor(TextForm_t *form, sf::RenderWindow *window, sf::Font *font, sf::Vector2f pos, sf::Vector2f size) {
    form->window = window;

    form->inputSize = 0;
    form->cursorPos = 0;
    memset(form->inputStr, 0, sizeof(wchar_t) * (MAX_INPUT_LEN + 1));

    form->box.setOrigin(mapCoords(size * 0.5f, window));
    form->box.setPosition(mapCoords(pos, window));
    form->box.setSize(mapCoords(size, window));
    form->box.setFillColor(TEXTFORM_MAIN_BKG_COLOR);
    form->box.setOutlineThickness(5);

    form->label.setFont(*font);
    form->label.setCharacterSize(TEXTFORM_FONT_SIZE);
    form->label.setFillColor(sf::Color(0x000000FF));
    form->label.setOrigin(sf::Vector2f(0, (float)TEXTFORM_FONT_SIZE/2));
    form->label.setPosition(mapCoords(pos, window) -
                            sf::Vector2f(0.45f*mapCoords(size, window).x, 0) );

    form->cursor.setOrigin(sf::Vector2f(0, (float)TEXTFORM_FONT_SIZE/2));
    form->cursor.setSize(sf::Vector2f(1, (float)TEXTFORM_FONT_SIZE));
    form->cursor.setPosition(mapCoords(pos, window) -
                            sf::Vector2f(0.45f*mapCoords(size, window).x, 0));
    form->cursor.setOutlineThickness(1);

    form->visible = true;
}

void textFormSetVisible(TextForm_t *form, bool visible) {
    form->visible = visible;
}

void textFormKeyUpdate(TextForm_t *form, sf::Event::KeyEvent key) {
    if (!form->visible)
        return;

    if (form->isSelected) {
        // arrows navigation
        if (key.code == sf::Keyboard::Left) {
            if (key.control)
                moveCursorToStart(form);
            else
                moveCursorByGlyph(form, 0);
        } else if (key.code == sf::Keyboard::Right) {
            if (key.control)
                moveCursorToEnd(form);
            else
                moveCursorByGlyph(form, 1);
        }


        // Ctrl + V
        if (key.code == sf::Keyboard::V && key.control) {
            sf::String pastedString = sf::Clipboard::getString();
            const uint32_t *stringData = pastedString.getData();
            size_t fullStringLen = pastedString.getSize() + 1;
            memcpy(form->inputStr, stringData, fullStringLen * sizeof(wchar_t));

            form->inputSize = fullStringLen - 1;
            moveCursorToEnd(form);
            form->label.setString(pastedString);
        }

        // Backspace and Ctrl + Backspace

        if (key.code == sf::Keyboard::BackSpace) {
            eraseCharacter(form);
            // Ctrl+Bksp erases all from cursor to start of the string
            if (key.control) {
                while (form->cursorPos > 0)
                    eraseCharacter(form);
            }
        }
    }
}


void textFormUpdate(TextForm_t *form, wchar_t symbol) {
    if (!form->visible)
        return;

    if (form->isSelected) {
        if (form->inputSize < MAX_INPUT_LEN && iswprint(symbol)) {
            insertCharacter(form, symbol);
        }
    }

    form->label.setString(form->inputStr);
}

void textFormClear(TextForm_t *form) {
    form->inputSize = 0;
    form->inputStr[0] = L'\0';
    form->label.setString(form->inputStr);
    moveCursorToStart(form);
}

const wchar_t *textFormGetText(TextForm_t *form) {
    return form->inputStr;
}

void textFormClickEventUpdate(TextForm_t *form) {
    if (!form->visible) return;

    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(*form->window));
    bool mouseInBox = form->box.getGlobalBounds().contains(mousePos);
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    if (!mousePressed) return;

    if (form->isSelected && !mouseInBox) {
        form->isSelected = false;
        form->box.setOutlineColor(TEXTFORM_MAIN_COLOR);
        form->box.setFillColor(TEXTFORM_MAIN_BKG_COLOR);
    } else if (!form->isSelected && mouseInBox) {
        form->isSelected = true;
        form->box.setOutlineColor(TEXTFORM_SELECTED_COLOR);
        form->box.setFillColor(TEXTFORM_SELECTED_BKG_COLOR);
    }
}

void textFormDraw(TextForm_t *form) {
    if (!form->visible) return;

    form->window->draw(form->box);
    form->window->draw(form->label);
    form->window->draw(form->cursor);
}

void textFormDtor(TextForm_t *form) {
    // printf(L_DEBUG, 0, "Destructed form[%p]\n", form);
}
