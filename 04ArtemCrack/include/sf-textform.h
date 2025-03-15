#ifndef SF_TEXT_FORM_H
#define SF_TEXT_FORM_H

#include <SFML/Graphics.hpp>

const size_t MAX_INPUT_LEN = 63;
const wchar_t BACKSPACE_SYMBOL = 8;

//outline colors
const sf::Color TEXTFORM_MAIN_COLOR(0xFFFFFFFF);
const sf::Color TEXTFORM_MAIN_BKG_COLOR(0x4f7e4fFF);
const sf::Color TEXTFORM_SELECTED_COLOR(0xFFFFFFFF);
const sf::Color TEXTFORM_SELECTED_BKG_COLOR(0x7e4f7eFF);
const int TEXTFORM_FONT_SIZE = 52; // in pixels

typedef struct {
    sf::RenderWindow *window;
    sf::RectangleShape box;
    sf::RectangleShape cursor;

    wchar_t inputStr[MAX_INPUT_LEN+1];
    size_t cursorPos;
    size_t inputSize;
    sf::Text label;

    bool isSelected;
    bool visible;
} TextForm_t;

// pos and size are factor of the window size
void textFormCtor(TextForm_t *form, sf::RenderWindow *window, sf::Font *font, sf::Vector2f pos, sf::Vector2f size);

void textFormSetVisible(TextForm_t *form, bool visible);

const wchar_t *textFormGetText(TextForm_t *form);
void textFormClear(TextForm_t *form);

void textFormUpdate(TextForm_t *form, wchar_t symbol);

void textFormKeyUpdate(TextForm_t *form, sf::Event::KeyEvent key);

void textFormClickEventUpdate(TextForm_t *form);

void textFormDraw(TextForm_t *form);

void textFormDtor(TextForm_t *form);

#endif
