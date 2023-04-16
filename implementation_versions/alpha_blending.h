#ifndef ALPHA_BLENDING_H
#define ALPHA_BLENDING_H

#include <stdio.h>
#include <SFML/Graphics.hpp>

#ifdef TIME_CHECK
static const size_t CALC_NUM = 1000;
#else 
static const size_t CALC_NUM = 1;
#endif 

static const size_t WINDOW_WIDTH  = 800;
static const size_t FRONT_WIDTH   = 235;
static const size_t BACK_WIDTH    = 800;

static const size_t WINDOW_HEIGHT = 600;
static const size_t FRONT_HEIGHT  = 126;
static const size_t BACK_HEIGHT   = 600;
static const size_t HEIGHT_SHIFT  = (BACK_HEIGHT - FRONT_HEIGHT) / 2 - 16;
static const size_t WIDTH_SHIFT   = (BACK_WIDTH  - FRONT_WIDTH ) / 2 - 80;

static const size_t MAX_TEXT_LEN = 20;

static const int OPEN_ERR = -1;

static const char *WINDOW_HEADER = "alpha";
static const char *back_img = "images&font/Table.bmp";
static const char *front_img = "images&font/AskhatCat.bmp";

static const char *fps_font_file = "images&font/fps_font.ttf";

void alpha_blending (void);
void alpha_blending_main (const sf::Image front_img, const sf::Image back_img, sf::Image *result);
void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num);

#endif /* ALPHA_BLENDING_H */