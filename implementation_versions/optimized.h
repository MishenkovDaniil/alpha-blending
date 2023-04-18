#ifndef ALPHA_BLENDING_OPTIMIZED_H
#define ALPHA_BLENDING_OPTIMIZED_H

#include <assert.h>
#include <emmintrin.h>
#include <fcntl.h>      /// for open()
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>   /// for mmap()
#include <sys/stat.h>   /// for stat()
#include <smmintrin.h>
#include <unistd.h>     /// for close(), getpagesize()
#include <xmmintrin.h>
#include <SFML/Graphics.hpp>

#include "config.h"

#ifdef TIME_CHECK
static const size_t CALC_NUM = 100000;
#else 
static const size_t CALC_NUM = 1;
#endif 

static const size_t DELTA = FRONT_WIDTH % 4;
static const char ZERO = 0x80;

static const __m128i FF_MULTIPLIER  = _mm_set_epi8 (0, 0xff, 0, 0xff,0, 0xff,0, 0xff,0, 0xff,0, 0xff,0, 0xff,0, 0xff);
                                                    /// [ - ff - ff | - ff - ff | - ff - ff | - ff - ff ]
static const __m128i ALPHA_MASK     = _mm_set_epi8 (ZERO, 0x0E, 
                                                    ZERO, 0x0E, 
                                                    ZERO, 0x0E,
                                                    ZERO, 0x0E,
                                                    ZERO, 0x06,
                                                    ZERO, 0x06,
                                                    ZERO, 0x06,
                                                    ZERO, 0x06);
static const __m128i SUM_MASK       = _mm_set_epi8 (ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
                                                    0x0F, 0x0D, 0x0B, 0x09, 0x07, 0x05, 0x03, 0x01);              
static const __m128 ZERO_ARR        = _mm_set_ps1(0);

void alpha_blending (void);
void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num);
const void *read_file_rdonly (const char *filename);
size_t get_file_size (const char *filename);
void alpha_blending_main (const char *front, const char *back, char *result);

void copy_img (char *dst, const char *src, size_t num_of_pixels, size_t pixel_len);
void revert_and_convert_bgr_rgba (const char *bgr_img, char *rgba_img);
void revert_and_convert_bgra_rgba (const char *src, char *dst, size_t height, size_t width);

#endif /* ALPHA_BLENDING_OPTIMIZED_H */
