#include <stdio.h>
#include <SFML/Graphics.hpp>

#define TIME_CHECK

#include "alpha_blending.h"
#include "../../mean_square_deviation/msd.h"
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
        sf::Time elapsed_time_1;

        result.loadFromFile (back_img);
        double values[CALC_NUM] = {0};

        for (size_t calc_iter = 0; calc_iter < CALC_NUM; ++calc_iter)
        {
            sf::Clock clock;
            
            alpha_blending_main (askhat, table, &result);

            elapsed_time_1 = clock.getElapsedTime ();
            elapsed_time += elapsed_time_1;
            values[calc_iter] = (double)1 / (double)elapsed_time_1.asSeconds ();
        }

        texture.loadFromImage (result);
        sprite.setTexture (texture);

        load_fps_text (&fps_text, &fps_font, fps_font_file, elapsed_time, CALC_NUM);
        
        double avg = (double) 1 / ((double)elapsed_time.asSeconds () / CALC_NUM);
        printf ("%lf\ng", msd (values, CALC_NUM, avg));
        
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
    for (int y = HEIGHT_SHIFT; y < HEIGHT_SHIFT + FRONT_HEIGHT; ++y)
    {
        for (int x = WIDTH_SHIFT; x < WIDTH_SHIFT + FRONT_WIDTH; ++x)
        {
            sf::Color back_color = back_img.getPixel (x, y);
            sf::Color front_color = front_img.getPixel (x - WIDTH_SHIFT, y - HEIGHT_SHIFT);
            sf::Color result_color;

            unsigned int alpha = front_color.a;
            unsigned int bk_alpha = 255 - alpha;

            result_color.r = (front_color.r * alpha + back_color.r * bk_alpha) >> 8;
            result_color.g = (front_color.g * alpha + back_color.g * bk_alpha) >> 8;
            result_color.b = (front_color.b * alpha + back_color.b * bk_alpha) >> 8;
            result_color.a = (front_color.a * alpha + back_color.a * bk_alpha) >> 8;
            result->setPixel (x, y, result_color);
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