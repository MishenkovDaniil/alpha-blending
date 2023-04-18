#ifndef ALPHA_BLENDING_CONFIG_H
#define ALPHA_BLENDING_CONFIG_H


static const size_t WINDOW_WIDTH  = 800;
static const size_t FRONT_WIDTH   = 235;
static const size_t BACK_WIDTH    = 800;

static const size_t WINDOW_HEIGHT = 600;
static const size_t FRONT_HEIGHT  = 126;
static const size_t BACK_HEIGHT   = 600;
static const size_t HEIGHT_SHIFT  = (BACK_HEIGHT - FRONT_HEIGHT) / 2 - 16;
static const size_t WIDTH_SHIFT   = (BACK_WIDTH  - FRONT_WIDTH ) / 2 - 80;

static const size_t MAX_TEXT_LEN = 20;

static const int BMP_HEADER_SIZE = 0x36;

static const int OPEN_ERR = -1;

static const char *WINDOW_HEADER = "alpha";
static const char *back_img = "images&font/Table.bmp";
static const char *front_img = "images&font/AskhatCat.bmp";

static const char *fps_font_file = "images&font/fps_font.ttf";

#endif /* ALPHA_BLENDING_CONFIG_H */
