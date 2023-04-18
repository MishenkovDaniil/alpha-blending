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

// #define TIME_CHECK

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


int main ()
{   
    alpha_blending ();
    
    return 0;
}

void alpha_blending ()
{
    sf::RenderWindow window (sf::VideoMode (WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_HEADER);
    
    const char *front_main = (const char *)read_file_rdonly (front_img);
    const char *back_main  = (const char *)read_file_rdonly (back_img);
          char *back       = (char *)calloc (BACK_HEIGHT  * BACK_WIDTH  * 4, sizeof (char));
          char *front      = (char *)calloc (FRONT_HEIGHT * FRONT_WIDTH * 4, sizeof (char));
          char *result_arr = (char *)calloc (BACK_HEIGHT  * BACK_WIDTH  * 4, sizeof (char));
    assert (back);
    assert (front);
    assert (back_main);
    assert (front_main);
    assert (result_arr);

    front_main += BMP_HEADER_SIZE;
    back_main  += BMP_HEADER_SIZE;

    revert_and_convert_bgr_rgba (back_main, back);
    revert_and_convert_bgra_rgba (front_main, front, FRONT_HEIGHT, FRONT_WIDTH);
    copy_img (result_arr, back, BACK_HEIGHT * BACK_WIDTH, 4);
    
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
            alpha_blending_main (front, back, result_arr);
            elapsed_time += clock.getElapsedTime ();
        }
        
        result.create (BACK_WIDTH, BACK_HEIGHT, (const sf::Uint8 *)result_arr);
        texture.loadFromImage (result);
        sprite.setTexture (texture);
        
        while (window.pollEvent (event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close ();
            }
        }

        load_fps_text (&fps_text, &fps_font, fps_font_file, elapsed_time, CALC_NUM);

        window.clear ();
        window.draw (sprite);
        // window.draw (fps_text);
        window.display ();
    }

    free (back);
    free (front);
    free (result_arr);
}

void alpha_blending_main (const char *front, const char *back, char *result)
{
    for (size_t y = HEIGHT_SHIFT; y < HEIGHT_SHIFT + FRONT_HEIGHT; ++y)
    {
        size_t idx = y * BACK_WIDTH * 4 + WIDTH_SHIFT * 4;

        for (size_t x = WIDTH_SHIFT; x < WIDTH_SHIFT + FRONT_WIDTH - DELTA; x += 4, front += 16, idx += 16)
        {
            __m128i front_10 = _mm_loadu_si128 ((__m128i *)&(*front));
            __m128i back_10  = _mm_loadu_si128 ((__m128i *)&(back[idx]));                     
                                                                                            
            __m128i front_32 = (__m128i) _mm_movehl_ps (ZERO_ARR, (__m128)front_10);      /// [ - - - - | - - - - | a3 b3 g3 r3 | a2 b2 g2 r2]
            __m128i back_32  = (__m128i) _mm_movehl_ps (ZERO_ARR, (__m128)back_10);


            front_10 = _mm_cvtepu8_epi16 (front_10);                                            /// [ - a1 - b1 | - g1 - r1 | - a0 - b0 | - g0 - r0 ]       
            
            front_32 = _mm_cvtepu8_epi16 (front_32);                                            /// [ - a3 - b3 | - g3 - r3 | - a2 - b2 | - g2 - r2 ]  

            back_10 = _mm_cvtepu8_epi16 (back_10);
            back_32 = _mm_cvtepu8_epi16 (back_32);
            
            __m128i alpha_multiplier_10 = _mm_shuffle_epi8 (front_10, ALPHA_MASK);               /// [ - a1 - a1 | - a1 - a1 | - a0 - a0 | - a0 - a0 ]    
            __m128i alpha_multiplier_32 = _mm_shuffle_epi8 (front_32, ALPHA_MASK);               /// [ - a3 - a3 | - a3 - a3 | - a2 - a2 | - a2 - a2 ]                     

            __m128i  bk_alpha_multiplier_10 = _mm_sub_epi16 (FF_MULTIPLIER, alpha_multiplier_10); /// [ - ba1 - ba1| . .  . . | - ba0 - ba0| . . . .  ]
            __m128i  bk_alpha_multiplier_32 = _mm_sub_epi16 (FF_MULTIPLIER, alpha_multiplier_32); /// [ - ba3 - ba3| . .  . . | - ba2 - ba2| . . . .  ]

            front_10 = _mm_mullo_epi16 (front_10, alpha_multiplier_10);                          /// fr *= a;
            front_32 = _mm_mullo_epi16 (front_32, alpha_multiplier_32);                          /// FR *= A;

            back_10 = _mm_mullo_epi16 (back_10, bk_alpha_multiplier_10);                         /// bk *= (255 - a);
            back_32 = _mm_mullo_epi16 (back_32, bk_alpha_multiplier_32);                         /// BK *= (255 - A);

            __m128i sum_10 = _mm_adds_epi16 (front_10, back_10);                                  /// [A1 B1 | G1 R1 | A0 B0 | G0 R0]
            __m128i sum_32 = _mm_adds_epi16 (front_32, back_32);                                  /// [A3 B3 | G3 R3 | A2 B2 | G2 R2]
            
            sum_10 = _mm_shuffle_epi8 (sum_10, SUM_MASK);                                        /// [- - - - | - - - - | A1 B1 G1 R1 | A0 B0 G0 R0]
            sum_32 = _mm_shuffle_epi8 (sum_32, SUM_MASK);                                        /// [- - - - | - - - - | A3 B3 G3 R3 | A2 B2 G2 R2]
            
            __m128i color = (__m128i) _mm_movelh_ps ((__m128)sum_10, (__m128)sum_32);            /// [A3 B3 G3 R3 | A2 B2 G2 R2 | A1 B1 G1 R1 | A0 B0 G0 R0]

            _mm_storeu_si128 ((__m128i *)&(result[idx]), color);
        }  
        for (size_t iter = 0; iter < DELTA; ++iter, idx += 16)
        {
            unsigned char fr_alpha = front[3];
            unsigned char bk_alpha = 0xff - fr_alpha;

            result[idx]     = ((unsigned char)*front++ * fr_alpha + (unsigned char)back[idx]     * bk_alpha) >> 8;
            result[idx + 1] = ((unsigned char)*front++ * fr_alpha + (unsigned char)back[idx + 1] * bk_alpha) >> 8;
            result[idx + 2] = ((unsigned char)*front++ * fr_alpha + (unsigned char)back[idx + 2] * bk_alpha) >> 8;
            ++front;
        }
    }
}

void copy_img (char *dst, const char *src, size_t num_of_pixels, size_t pixel_len)
{
    assert (dst && src);
    
    for (int iter = 0; iter < num_of_pixels * pixel_len; ++iter)
    {
        *dst++ = *src++;
    }
}

void revert_and_convert_bgr_rgba (const char *bgr_img, char *rgba_img)
{
    assert (bgr_img && rgba_img);
    
    for (int y = BACK_HEIGHT - 1; y >= 0; --y)
    {
        for (int x = 0; x < BACK_WIDTH; ++x)               /// BGR colors into RGBA colors
        {
            rgba_img[(y*BACK_WIDTH + x) * 4 + 2] = *bgr_img++; 
            rgba_img[(y*BACK_WIDTH + x) * 4 + 1] = *bgr_img++; 
            rgba_img[(y*BACK_WIDTH + x) * 4 + 0] = *bgr_img++;
            rgba_img[(y*BACK_WIDTH + x) * 4 + 3] = 0xff;
        }
    }
}

void revert_and_convert_bgra_rgba (const char *src, char *dst, size_t height, size_t width)
{
    assert (src && dst);
    
    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            dst[(y*width + x) * 4 + 2] = *src++; 
            dst[(y*width + x) * 4 + 1] = *src++; 
            dst[(y*width + x) * 4 + 0] = *src++; 
            dst[(y*width + x) * 4 + 3] = *src++;
        }
    }
}

const void *read_file_rdonly (const char *filename)
{
    assert (filename);
    
    int file_descr = open (filename, O_RDONLY);
    if (file_descr == OPEN_ERR)
    {
        perror ("open() failed:");
        return nullptr;
    }

    size_t filesize = get_file_size (filename);
    if (!filesize)
        return nullptr;
    // int ps = getpagesize ();
    // printf ("%d\n", ps);
    void *buffer = mmap (nullptr, filesize, PROT_READ, MAP_SHARED, file_descr, 0);

    if (buffer == MAP_FAILED)
    {
        perror ("mmap() failed.\n");
    }

    if (close (file_descr) == -1)
    {
        printf ("close() failed, file \"%s\".\n", filename);
        return nullptr;
    }

    return buffer;
}

size_t get_file_size (const char *filename)
{
    assert (filename);
    
    struct stat buf = {};

    if (stat (filename, &buf) == -1)
    {
        perror ("stat() failed.\n");

        return 0;
    }

    return buf.st_size;
}

void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num)
{
    assert (fps_text && font && fps_file);
    
    font->loadFromFile (fps_file);
    
    char string[MAX_TEXT_LEN] = "";

    sprintf (string, "FPS: %lf", (double) 1 / ((double)elapsed_time.asSeconds () / calc_num));
    
    fps_text->setString (string);
    fps_text->setFont (*font);
}