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

    // for (int i = 0; i < 0x36; ++i)
    // {
    //     *back++ = *back_main++;
    // }
    // for (int i = 0; i < 600*800; ++i)
    // {
    //     for (int j = 0; j < 3; ++j)
    //     {
    //         *back++ = *back_main++;
    //     }
    //     *back = 0xff;
    //     back++;
    // }
    // back = back_start;

    // back += BMP_HEADER_SIZE;
    // front += BMP_HEADER_SIZE;

    for (int i = 0; i < BACK_WIDTH * BACK_HEIGHT; ++i)
    {
        result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 2 - i * 4] = (sf::Uint8) (back[i*4]);
        result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 3 - i * 4] = (sf::Uint8) (back[i*4 + 1]);
        result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 4  -i * 4] = (sf::Uint8) (back[i*4 + 2]);
        result_arr[BACK_WIDTH * BACK_HEIGHT * 4 - 1 - i * 4] = 0xff;
        // back += 4;
    }

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
            //result[first_pixel + 2] = ((unsigned char)*front++ * (unsigned char)fr_alpha + (unsigned char)result[first_pixel + 2] * (unsigned char)(255 - fr_alpha)) >> 8;
            //result[first_pixel + 1] = ((unsigned char)*front++ * (unsigned char)fr_alpha + (unsigned char)result[first_pixel + 1] * (unsigned char)(255 - fr_alpha)) >> 8;
            // result[first_pixel] = ((unsigned char)*front++ * (unsigned char)fr_alpha + (unsigned char)result[first_pixel + 2] * (unsigned char)(255 - fr_alpha)) >> 8;
            ++front;
            ++back;
        }
        back += WIDTH_SHIFT * 4;
    }
}

// void alpha_blending_main (const char *front, const char *back, char *result)
// {
//     result[0] = 0x42;
//     result[1] = 0x4d;
//     result[2] = 0x38;
//     result[3] = 0x4c;
//     result[4] = 0x1d;
//     result[5] = 0x00;
//     result[6] = 0x00;
//     result[7] = 0x00;
//     result[8] = 0x00;
//     result[9] = 0x00;
//     result[10] = 0x36;
//     result[11] = 0x00;
//     result[12] = 0x00;
//     result[13] = 0x00;
//     result[14] = 0x28;
//     result[15] = 0x00;
//     result[16] = 0x00;
//     result[17] = 0x00;
//     result[18] = 0x20;
//     result[19] = 0x03;
//     result[20] = 0x00;
//     result[21] = 0x00;
//     result[22] = 0x58;
//     result[23] = 0x02;
//     result[24] = 0x00;
//     result[25] = 0x00;
//     result[26] = 0x01;
//     result[27] = 0x00;
//     result[28] = 0x20;
//     result[29] = 0x00;
//     result[30] = 0x00;
//     result[31] = 0x00;
//     result[32] = 0x00;
//     result[33] = 0x00;
//     result[34] = 0x02;
//     result[35] = 0xf9;
//     result[36] = 0x15;
//     result[37] = 0x00;
//     result[38] = 0x12;
//     result[39] = 0x0b;
//     result[40] = 0x00;
//     result[41] = 0x00;
//     result[42] = 0x12;
//     result[43] = 0x0b;
//     result[44] = 0x00;
//     result[45] = 0x00;
//     result[46] = 0x00;
//     result[47] = 0x00;
//     result[48] = 0x00;
//     result[49] = 0x00;
//     result[50] = 0x00;
//     result[51] = 0x00;
//     result[52] = 0x00;
//     result[53] = 0x00;
//     result += 54;
//     back   += 54;
//     front  += 54; 
//     // for (int i = 0; i < 54; ++i)
//     // {
//     //     *result++ = *front++;
//     //     back++;
//     // }

//     for (int y = 0; y <  HEIGHT_SHIFT; ++y)
//     {
//         for (int x = 0; x < BACK_WIDTH; ++x)
//         {
//             for (int i = 0; i < 4; ++i)
//             {
//                 *result++ = *back++;
//             }
//         }
//     }

//     for (int y = HEIGHT_SHIFT; y < HEIGHT_SHIFT + FRONT_HEIGHT; ++y)
//     {
//         int x = 0;
//         for (;x < WIDTH_SHIFT; ++x)
//         {
//             for (int i = 0; i < 4; ++i)
//             {
//                 *result++ = *back++;
//             }
//         }
//         for ( ;x < WIDTH_SHIFT + FRONT_WIDTH; ++x)
//         {
//             unsigned int fr_alpha = *(front + 3);
//             for (int i = 0; i < 4; ++i)
//             {
//                 (*(result + i)) = (char)(((unsigned int )((unsigned int)(*(front + i)) * fr_alpha + (unsigned int)(*(back + i)) * (unsigned int)(255 - fr_alpha))) >> 8);
//             }
//             // *(result + 3) = 0xff;
//             result += 4;
//             front += 4;
//             back += 4;
//         }
//         for (; x < BACK_WIDTH; ++x)
//         {
//             for (int i = 0; i < 4; ++i)
//             {
//                 *result++ = *back++;
//             }
//         }
//     }

//     for (int y = HEIGHT_SHIFT + FRONT_HEIGHT; y < BACK_HEIGHT; ++y)
//     {
//         for (int x = 0; x < BACK_WIDTH; ++x)
//         {
//             for (int i = 0; i < 4; ++i)
//             {
//                 *result++ = *back++;
//             }
//         }
//     }
// }

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

// void *create_file (const char *filename, size_t filesize)
// {
//     int file_descr = open (filename, O_RDWR);
//     if (file_descr == OPEN_ERR)
//     {
//         perror ("open() failed:");
//         return nullptr;
//     }
//     char *ggg = (char *)calloc (filesize, sizeof (char));
//     write (file_descr, ggg, filesize);
//     free (ggg);

//     void *buffer = mmap (nullptr, filesize, PROT_WRITE, MAP_SHARED, file_descr, 0);

//     if (buffer == MAP_FAILED)
//     {
//         perror ("mmap() failed.\n");
//     }

//     if (close (file_descr) == -1)
//     {
//         printf ("close() failed, file \"%s\".\n", filename);
//         return nullptr;
//     }

//     return buffer;
// }

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


// read arrays of 2 images
// fr = [a3, r3, g3, b3, ..., a0, r0, g0, b0] -->
// FR = [- - - - - - - -, a3, r3, g3, b3, ..., b2]-->
// fr = [- a1 - r1, - g1 - b1, ...] --}
//                                      -->
// FR = [- a3 - r3, - g3 - b3, ...] --}
// a = [- a1 - a1, - a1 - a1, - a0 - a0, - a0 - a0] --}
//                                                      -->
// A = [- a3 - a3, - a3 - a3, - a2 - a2, - a2 - a2] --}
// shuffle a (with moveA mask) --}
//                                  -->
// shuffle b (with moveA mask) --}
// fr *= a (back *= (255 -a))   --}
//                                  -->
// FR *= A                      --}
// ... 
// sum = [- - - -, - - - -, A1 R1 G1 B1, ...] --}
//                                                  -->
// SUM = [- - - -, - - - -, A3 R3 G3 B3, ...] --}
// COLOR = [A3 R3 G3 B3, ..., A0 R0 G0 B0]


// 
// 
// 
// color = (color_front * a + color_b * (255 - a) ) /255
// put colour in new array
// print new image