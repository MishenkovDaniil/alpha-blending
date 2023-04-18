#ifndef ALPHA_BLENDING_H
#define ALPHA_BLENDING_H

#include <stdio.h>
#include <SFML/Graphics.hpp>

#include "config.h"

#ifdef TIME_CHECK
static const size_t CALC_NUM = 10000;
#else 
static const size_t CALC_NUM = 1;
#endif 

void alpha_blending (void);
void alpha_blending_main (const sf::Image front_img, const sf::Image back_img, sf::Image *result);
void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num);

#endif /* ALPHA_BLENDING_H */