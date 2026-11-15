#ifndef TTF_RENDER_H
#define TTF_RENDER_H


int TTFLoadFont(int set, const char * path, void * from_memory, int size_from_memory);


// reset the glyph cache
void init_ttf_table(void);

// define window mode and size

#define WIN_AUTO_LF 1
#define WIN_SKIP_LF 2
#define WIN_DOUBLE_LF  4

void set_ttf_window(int x, int y, int width, int height, u32 mode);

extern float Y_ttf;
extern float Z_ttf;

// display UTF8 string int posx/posy position. With color 0 don't display and refresh/calculate the width. 
// color is the character color and sw/sh the width/height of the characters

int display_ttf_string(int posx, int posy, const char *string, u32 color, u32 bkcolor, int sw, int sh, int (*DrawIcon)(int, int, char));
int width_ttf_string(const char *string, int sw, int sh);

#endif
