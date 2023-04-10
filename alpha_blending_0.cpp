#include <stdio.h>
#include <SFML/Graphics.hpp>

#define TIME_CHECK

#include "alpha_blending.h"

int main ()
{   
    alpha_blending ();

    return 0;
}

void alpha_blending ()
{
    sf::RenderWindow window (sf::VideoMode (WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_HEADER);
    
    sf::Image askhat;
    sf::Image table;

    askhat.loadFromFile (front_img);
    table.loadFromFile (back_img);
    
    while (window.isOpen ())
    {
        sf::Image result;
        sf::Font fps_font;
        sf::Text fps_text;
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Event event;
        sf::Time elapsed_time;

        for (size_t calc_iter = 0; calc_iter < CALC_NUM; ++calc_iter)
        {
            sf::Clock clock;
            alpha_blending_main (askhat, table, &result);
            elapsed_time += clock.getElapsedTime ();
        }

        texture.loadFromImage (result);
        sprite.setTexture (texture);

        load_fps_text (&fps_text, &fps_font, fps_font_file, elapsed_time, CALC_NUM);
        
        while (window.pollEvent (event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close ();
            }
        }

        window.clear ();
        window.draw (sprite);
        window.draw (fps_text);
        window.display ();
    }
}

void alpha_blending_main (const sf::Image front_img, const sf::Image back_img, sf::Image *result)
{
    result->create (BACK_WIDTH, BACK_HEIGHT);

    for (int y = 0; y <  HEIGHT_SHIFT; ++y)
    {
        for (int x = 0; x < BACK_WIDTH; ++x)
        {
            result->setPixel (x, y, back_img.getPixel (x, y));
        }
    }

    for (int y = HEIGHT_SHIFT; y < HEIGHT_SHIFT + FRONT_HEIGHT; ++y)
    {
        int x = 0;
        for (;x < WIDTH_SHIFT; ++x)
        {
            result->setPixel (x, y, back_img.getPixel (x, y));
        }
        for ( ;x < WIDTH_SHIFT + FRONT_WIDTH; ++x)
        {
            sf::Color back_color = back_img.getPixel (x, y);
            sf::Color front_color = front_img.getPixel (x - WIDTH_SHIFT, y - HEIGHT_SHIFT);
            sf::Color result_color;
            result_color.r = (front_color.r * front_color.a + back_color.r * (255 - front_color.a)) >> 8;
            result_color.g = (front_color.g * front_color.a + back_color.g * (255 - front_color.a)) >> 8;
            result_color.b = (front_color.b * front_color.a + back_color.b * (255 - front_color.a)) >> 8;
            result_color.a = (front_color.a * front_color.a + back_color.a * (255 - front_color.a)) >> 8;
            result->setPixel (x, y, result_color);
        }
        for (; x < BACK_WIDTH; ++x)
        {
            result->setPixel (x, y, back_img.getPixel (x, y));
        }
    }

    for (int y = HEIGHT_SHIFT + FRONT_HEIGHT; y < BACK_HEIGHT; ++y)
    {
        for (int x = 0; x < BACK_WIDTH; ++x)
        {
            result->setPixel (x, y, back_img.getPixel (x, y));
        }
    }
}

void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num)
{
    font->loadFromFile (fps_file);
    
    char string[MAX_TEXT_LEN] = "";

    sprintf (string, "FPS: %lf", (double) 1 / ((double)elapsed_time.asSeconds () / calc_num));
    
    fps_text->setString (string);
    fps_text->setFont (*font);
}