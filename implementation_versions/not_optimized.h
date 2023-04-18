#ifndef ALPHA_BLENDING_NOT_OPTIMIZED_H
#define ALPHA_BLENDING_NOT_OPTIMIZED_H

#include <assert.h>
#include <emmintrin.h>
#include <fcntl.h>      /// open()
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>   /// mmap()
#include <sys/stat.h>   /// stat()
#include <unistd.h>     /// close(), getpagesize()
#include <xmmintrin.h>
#include <SFML/Graphics.hpp>

#include "config.h"

#ifdef TIME_CHECK
static const size_t CALC_NUM = 100000;
#else 
static const size_t CALC_NUM = 1;
#endif 


void alpha_blending (void);
void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num);
const void *read_file_rdonly (const char *filename);
size_t get_file_size (const char *filename);
void alpha_blending_main (const char *front, const char *back, sf::Uint8 *result);
void copy_and_convert_bgr_bgra (const char *src, char *dst, size_t pixel_num);
void revert_and_convert_bgr_rgba (const sf::Uint8 *src, sf::Uint8 *dst, size_t height, size_t width);


#endif /* ALPHA_BLENDING_NOT_OPTIMIZED_H */
