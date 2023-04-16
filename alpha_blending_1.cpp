#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include <SFML/Graphics.hpp>

#include <sys/mman.h>   //mmap()
#include <fcntl.h>      //open()
#include <sys/stat.h>   //stat()
// #include <sys/types.h>
 #include <unistd.h>  //close(), getpagesize()

// 424d 36 4c 1d 00 00 00 00 00 36 00 00 00 28 00
// 0000 2003 0000 5802 0000 0100 2000 0000 0000 02f9 1500 120b 0000 120b 0000 0000 0000 0000 0000 

#ifdef TIME_CHECK
static const size_t CALC_NUM = 100;
#else 
static const size_t CALC_NUM = 1;
#endif 

static const size_t MAX_TEXT_LEN = 20;

static const size_t WINDOW_WIDTH  = 800;
static const size_t FRONT_WIDTH   = 235;
static const size_t BACK_WIDTH    = 800;

static const size_t WINDOW_HEIGHT = 600;
static const size_t FRONT_HEIGHT  = 126;
static const size_t BACK_HEIGHT   = 600;
static const size_t HEIGHT_SHIFT  = (BACK_HEIGHT - FRONT_HEIGHT) / 2 - 16;
static const size_t WIDTH_SHIFT   = (BACK_WIDTH  - FRONT_WIDTH ) / 2 - 80;

static const int OPEN_ERR = -1;
static const int BMP_HEADER_SIZE = 0x36;

static const char *WINDOW_HEADER = "alpha";
static const char *back_img = "Table.bmp";
static const char *front_img = "AskhatCat.bmp";

static const char *fps_font_file = "fps_font.ttf";

void alpha_blending (void);
void *create_file (const char *filename, size_t filesize);
void load_fps_text (sf::Text *fps_text, sf::Font *font, const char *fps_file, sf::Time elapsed_time, const size_t calc_num);
const void *read_file_rdonly (const char *filename);
size_t get_file_size (const char *filename);
void alpha_blending_main (const char *front, const char *back, sf::Uint8 *result);
void copy_and_convert_bgr_bgra (const char *src, char *dst, size_t pixel_num);
void copy_and_convert_bgr_rgba (const sf::Uint8 *src, sf::Uint8 *dst, size_t pixel_num);

int main ()
{   
    alpha_blending ();
    
    return 0;
}

void alpha_blending ()
{
    sf::RenderWindow window (sf::VideoMode (WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_HEADER);
    
    sf::Uint8 *result_arr = (sf::Uint8 *)calloc (600*800*4, sizeof (sf::Uint8));
    
    const char *front = (const char *)read_file_rdonly (front_img);
    const char *back_main  = (const char *)read_file_rdonly (back_img);

    char *back  = (char *)calloc (600*800*4, sizeof (char));
    // char *back_start = back;

    copy_and_convert_bgr_bgra (back_main + BMP_HEADER_SIZE, back, BACK_HEIGHT * BACK_WIDTH);

    copy_and_convert_bgr_rgba ((const sf::Uint8 *)(back_main + BMP_HEADER_SIZE), result_arr, BACK_HEIGHT * BACK_WIDTH);

    // for (int i = 0; i < BACK_WIDTH * BACK_HEIGHT; ++i)
    // {
    //     result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 2 - i * 4] = (sf::Uint8) (back[i*4]);
    //     result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 3 - i * 4] = (sf::Uint8) (back[i*4 + 1]);
    //     result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 4  -i * 4] = (sf::Uint8) (back[i*4 + 2]);
    //     result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 1 - i * 4] = 0xff;
    // }

    while (window.isOpen ())
    {
        sf::Image result;
        sf::Font fps_font;
        sf::Text fps_text;
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Event event;

        sf::Clock clock;
        alpha_blending_main (front + BMP_HEADER_SIZE, back, result_arr);
        sf::Time elapsed_time = clock.getElapsedTime ();

        result.create (800, 600, (const sf::Uint8 *)result_arr);
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
        window.draw (fps_text);
        window.display ();
    }
    free (result_arr);
    free (back);
}

void copy_and_convert_bgr_bgra (const char *src, char *dst, size_t pixel_num)
{
    for (int i = 0; i < pixel_num; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            *dst++ = *src++;
        }
        *dst++ = 0xff;
    }
}

void copy_and_convert_bgr_rgba (const sf::Uint8 *src, sf::Uint8 *dst, size_t pixel_num)
{
    for (int i = 0; i < pixel_num; ++i)
    {
        dst[pixel_num * 4 - 2 - i * 4] = src[i*3];
        dst[pixel_num * 4 - 3 - i * 4] = src[i*3 + 1];
        dst[pixel_num * 4 - 4  -i * 4] = src[i*3 + 2];
        dst[pixel_num * 4 - 1 - i * 4] = 0xff;
    }
}


void alpha_blending_main (const char *front, const char *back, sf::Uint8 *result)
{
    back += (BACK_HEIGHT - FRONT_HEIGHT - HEIGHT_SHIFT) * BACK_WIDTH * 4;

    for (int y = HEIGHT_SHIFT + FRONT_HEIGHT - 1; y >= HEIGHT_SHIFT; --y)
    {
        back += (BACK_WIDTH - FRONT_WIDTH - WIDTH_SHIFT) * 4;

        for (int x = WIDTH_SHIFT + FRONT_WIDTH - 1; x >= WIDTH_SHIFT; --x)
        {
            unsigned char fr_alpha = front[3];
            unsigned char bk_alpha = 0xff - fr_alpha;

            int first_pixel = (y * BACK_WIDTH + x) * 4;

            result[first_pixel + 2] = ((unsigned char)*front++ * fr_alpha + (unsigned char)*back++ * bk_alpha) >> 8;
            result[first_pixel + 1] = ((unsigned char)*front++ * fr_alpha + (unsigned char)*back++ * bk_alpha) >> 8;
            result[first_pixel]     = ((unsigned char)*front++ * fr_alpha + (unsigned char)*back++ * bk_alpha) >> 8;

            ++front;
            ++back;
        }
        back += WIDTH_SHIFT * 4;
    }
}

const void *read_file_rdonly (const char *filename)
{
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
    font->loadFromFile (fps_file);
    
    char string[MAX_TEXT_LEN] = "";

    sprintf (string, "FPS: %lf", (double) 1 / ((double)elapsed_time.asSeconds () / calc_num));
    
    fps_text->setString (string);
    fps_text->setFont (*font);
}