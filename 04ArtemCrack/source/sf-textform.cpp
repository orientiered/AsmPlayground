#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SFML/Graphics.hpp>

#include "sf-textform.h"

//global lock on multiple form selection
static TextForm_t *selectedForm = NULL;

//TODO: this function is used in multiple files, remove copypaste
/// @brief convert from (0,1)^2 to (0, window.width)x(0, window.height)
static sf::Vector2f mapCoords(sf::Vector2f coords, const sf::RenderWindow *window) {
    sf::Vector2u windowSize = window->getSize();
    return sf::Vector2f(coords.x * windowSize.x, coords.y * windowSize.y);
}

void textFormCtor(TextForm_t *form, sf::RenderWindow *window, sf::Font *font, sf::Vector2f pos, sf::Vector2f size) {
    form->window = window;

    form->inputSize = 0;
    memset(form->inputStr, 0, sizeof(wchar_t) * (MAX_INPUT_LEN + 1));

    form->box.setOrigin(mapCoords(size * 0.5f, window));
    form->box.setPosition(mapCoords(pos, window));
    form->box.setSize(mapCoords(size, window));
    form->box.setOutlineThickness(5);

    form->label.setFont(*font);
    form->label.setCharacterSize(52);
    form->label.setFillColor(sf::Color(0x000000FF));
    form->label.setOrigin(mapCoords(size * 0.5f, window));
    form->label.setPosition(mapCoords(pos, window));

    form->visible = true;
}

void textFormSetVisible(TextForm_t *form, bool visible) {
    form->visible = visible;
}

void textFormUpdate(TextForm_t *form, wchar_t symbol) {
    if (!form->visible)
        return;

    if (form->isSelected) {
        if (symbol == BACKSPACE_SYMBOL) {
            if (form->inputSize > 0)
                form->inputStr[--form->inputSize] = L'\0';
        } else {
            if (form->inputSize < MAX_INPUT_LEN) {
                form->inputStr[form->inputSize++] = symbol;
                form->inputStr[form->inputSize] = L'\0';
            }
        }
    }

    form->label.setString(form->inputStr);
}

void textFormClear(TextForm_t *form) {
    form->inputSize = 0;
    form->inputStr[0] = L'\0';
    form->label.setString(form->inputStr);
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
}

void textFormDtor(TextForm_t *form) {
    // printf(L_DEBUG, 0, "Destructed form[%p]\n", form);
}
