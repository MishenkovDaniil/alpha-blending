#define TIME_CHECK

#include "not_optimized.h"
#include "../root_mean_square/rms.h"

int main ()
{   
    alpha_blending ();
    
    return 0;
}

void alpha_blending ()
{
    sf::RenderWindow window (sf::VideoMode (WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_HEADER);
    
    sf::Uint8 *result_arr = (sf::Uint8 *)calloc (BACK_HEIGHT*BACK_WIDTH*4, sizeof (sf::Uint8));
    
    const char *front      = (const char *)read_file_rdonly (front_img);
    const char *back_main  = (const char *)read_file_rdonly (back_img);

    char *back  = (char *)calloc (600*800*4, sizeof (char));

    copy_and_convert_bgr_bgra (back_main + BMP_HEADER_SIZE, back, BACK_HEIGHT * BACK_WIDTH);
    revert_and_convert_bgr_rgba ((const sf::Uint8 *)(back_main + BMP_HEADER_SIZE), result_arr, BACK_HEIGHT, BACK_WIDTH);

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

        double values[CALC_NUM] = {0};

        for (size_t calc_iter = 0; calc_iter < CALC_NUM; ++calc_iter)
        {
            sf::Clock clock;

            alpha_blending_main (front + BMP_HEADER_SIZE, back, result_arr);

            elapsed_time_1 = clock.getElapsedTime ();
            elapsed_time += elapsed_time_1;
            values[calc_iter] = (double)1 / (double)elapsed_time_1.asSeconds ();
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
        
        double avg = (double) 1 / ((double)elapsed_time.asSeconds () / CALC_NUM);
        printf ("%lf\ng", rms (values, CALC_NUM, avg));

        window.clear ();
        window.draw (sprite);
        window.draw (fps_text);
        window.display ();
    }
    free (result_arr);
    free (back);
}

void alpha_blending_main (const char *front, const char *back, sf::Uint8 *result)
{
    back += (BACK_HEIGHT - FRONT_HEIGHT - HEIGHT_SHIFT) * BACK_WIDTH * 4;

    for (int y = HEIGHT_SHIFT + FRONT_HEIGHT - 1; y >= HEIGHT_SHIFT; --y)
    {
        back += WIDTH_SHIFT * 4;//(BACK_WIDTH - FRONT_WIDTH - WIDTH_SHIFT) * 4;

        for (int x = WIDTH_SHIFT; x < WIDTH_SHIFT + FRONT_WIDTH; ++x)
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
        back += (BACK_WIDTH - FRONT_WIDTH - WIDTH_SHIFT) * 4;//WIDTH_SHIFT * 4;
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

void revert_and_convert_bgr_rgba (const sf::Uint8 *src, sf::Uint8 *dst, size_t height, size_t width)
{
    for (int y = 0; y < height; ++y)
    {
        size_t src_idx = y * width * 3;
        size_t dst_idx = (height - y - 1) * width * 4;

        for (int x = 0; x < width; ++x)
        {
            size_t cur_src_idx = src_idx + x*3;
            size_t cur_dst_idx = (height - y - 1) * width * 4 + x * 4;

            dst[cur_dst_idx + 2] = src[cur_src_idx++];
            dst[cur_dst_idx + 1] = src[cur_src_idx++];
            dst[cur_dst_idx + 0] = src[cur_src_idx];
            dst[cur_dst_idx + 3] = 0xff;
        }
    } 
}