#include <proto-include.h>
#include <SDL2/SDL.h>

#include "types.h"
#include "ttf_render.h"
#include "menu.h"

/******************************************************************************************************************************************************/
/* TTF functions to load and convert fonts                                                                                                             */
/******************************************************************************************************************************************************/

static int ttf_inited = 0;

static FT_Library freetype;
static FT_Face face[4];
static int f_face[4] = {0, 0, 0, 0};

int TTFLoadFont(int set, const char * path, void * from_memory, int size_from_memory)
{
   
    if(!ttf_inited)
        FT_Init_FreeType(&freetype);
    ttf_inited = 1;

    f_face[set] = 0;

    if(path) {
        if(FT_New_Face(freetype, path, 0, &face[set])<0) return -1;
    } else {
        if(FT_New_Memory_Face(freetype, from_memory, size_from_memory, 0, &face[set])) return -1;
        }

    f_face[set] = 1;

    return 0;
}

/* function to render the character
chr : character from 0 to 255
bitmap: u8 bitmap passed to render the character character (max 256 x 256 x 1 (8 bits Alpha))
*w : w is the bitmap width as input and the width of the character (used to increase X) as output
*h : h is the bitmap height as input and the height of the character (used to Y correction combined with y_correction) as output
y_correction : the Y correction to display the character correctly in the screen

*/
/*
*/
// constructor dinamico de fuentes 32 x 32

typedef struct ttf_dyn {
    u32 ttf;
    SDL_Texture *text[2];
    u32 r_use;
    u16 y_start;
    u16 width;
    u16 height;
    u16 flags;

} ttf_dyn;

#define MAX_CHARS 1600
#define TEX_SZ 64

static ttf_dyn ttf_font_datas[MAX_CHARS];

static u32 r_use= 0;

float Y_ttf = 0.0f;
float Z_ttf = 0.0f;

static int Win_X_ttf = 0;
static int Win_Y_ttf = 0;
static int Win_W_ttf = SCREEN_WIDTH;
static int Win_H_ttf = SCREEN_HEIGHT;


static u32 Win_flag = 0;

void set_ttf_window(int x, int y, int width, int height, u32 mode)
{
    Win_X_ttf = x;
    Win_Y_ttf = y;
    Win_W_ttf = width;
    Win_H_ttf = height;
    Win_flag = mode;
    Y_ttf = 0.0f;
    Z_ttf = 0.0f;
   
}

void init_ttf_table(void)
{
    r_use = 0;
    memset(ttf_font_datas, 0, sizeof(ttf_font_datas));
}

static void DrawBox_ttf(float x, float y, float z, float w, float h, u32 rgba)
{
    SDL_FRect rect = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };

    SDL_SetRenderDrawColor(renderer, RGBA_R(rgba), RGBA_G(rgba), RGBA_B(rgba), RGBA_A(rgba));
    SDL_RenderFillRectF(renderer, &rect);
/*
    tiny3d_SetPolygon(TINY3D_QUADS);
    
   
    tiny3d_VertexPos(x    , y    , z);
    tiny3d_VertexColor(rgba);

    tiny3d_VertexPos(x + w, y    , z);

    tiny3d_VertexPos(x + w, y + h, z);

    tiny3d_VertexPos(x    , y + h, z);

    tiny3d_End();
*/
}

static SDL_Texture* create_texture(const u8* bitmap, u32 rgba)
{
    u32 buf[TEX_SZ * TEX_SZ];

    for (int i = 0; i < TEX_SZ*TEX_SZ; i++)
        buf[i] = bitmap[i] ? ((rgba & 0xFFFFFF00) | bitmap[i]) : 0;

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*) buf, TEX_SZ, TEX_SZ, 32, 4 * TEX_SZ, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    SDL_Texture* sdl_tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(sdl_tex, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);

    return (sdl_tex);
}

static void DrawTextBox_ttf(SDL_Texture* bitmap, float x, float y, float z, float w, float h, u32 rgba, float tx, float ty)
{
    SDL_FRect dest = {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };

	SDL_SetTextureAlphaMod(bitmap, RGBA_A(rgba));
    SDL_RenderCopyF(renderer, bitmap, NULL, &dest);
}

#define TTF_UX 30 * (TEX_SZ /32)
#define TTF_UY 24 * (TEX_SZ /32)


int display_ttf_string(int posx, int posy, const char *string, u32 color, u32 bkcolor, int sw, int sh, int (*DrawIcon_cb)(int, int, char))
{
    int l,n, m, ww, ww2;
    u8 colorc;
    u32 ttf_char;
    u8 *ustring = (u8 *) string;

    int lenx = 0;

    while(*ustring) {

        if(posy >= Win_H_ttf) break;

        if(*ustring == ' ') {posx += sw>>1; ustring++; continue;}

        if(*ustring & 128) {
            m = 1;

            if((*ustring & 0xf8)==0xf0) { // 4 bytes
                ttf_char = (u32) (*(ustring++) & 3);
                m = 3;
            } else if((*ustring & 0xE0)==0xE0) { // 3 bytes
                ttf_char = (u32) (*(ustring++) & 0xf);
                m = 2;
            } else if((*ustring & 0xE0)==0xC0) { // 2 bytes
                ttf_char = (u32) (*(ustring++) & 0x1f);
                m = 1;
            } else {ustring++;continue;} // error!

             for(n = 0; n < m; n++) {
                if(!*ustring || (*ustring & 0xc0) != 0x80) { // error!
                    ttf_char = ' ';
                    break;
                }
                ttf_char = (ttf_char <<6) |((u32) (*(ustring++) & 63));
             }
           
            if((n != m) && !*ustring) break;
        
        } else ttf_char = (u32) *(ustring++);


        if(Win_flag & WIN_SKIP_LF) {
            if(ttf_char == '\r' || ttf_char == '\n') ttf_char=' ';
        } else {
            if(Win_flag & WIN_DOUBLE_LF) {
                if(ttf_char == '\r') {if(posx > lenx) lenx = posx; posx = 0;continue;}
                if(ttf_char == '\n') {posy += sh;continue;}
            } else {
                if(ttf_char == '\n') {if(posx > lenx) lenx = posx; posx = 0;posy += sh;continue;}
            }
        }

        if ((ttf_char < 32) && DrawIcon_cb) {
            int resx = DrawIcon_cb(posx, posy, (char) ttf_char);
            if (resx > 0) {
                posx += resx;
                continue;
            }
            else 
                ttf_char='?';
        }

        // search ttf_char
        if(ttf_char < 128) n= ttf_char;
        else {
            m= 0;
            int rel=0;
            
            for(n= 128; n < MAX_CHARS; n++) {
                if(!(ttf_font_datas[n].flags & 1)) m= n;
                
                if((ttf_font_datas[n].flags & 3)==1) {
                    int trel= r_use - ttf_font_datas[n].r_use;
                    if(m==0) {m= n;rel = trel;}
                    else if(rel > trel) {m= n;rel = trel;}

                }
                if(ttf_font_datas[n].ttf == ttf_char) break;
            }

            if(m==0) m = 128;

        }

        if(n >= MAX_CHARS) {ttf_font_datas[m].flags = 0; l= m;} else l=n;

        // building the character
        if(!(ttf_font_datas[l].flags & 1)) { 

            if(f_face[0]) FT_Set_Pixel_Sizes(face[0], TTF_UX, TTF_UY);
            if(f_face[1]) FT_Set_Pixel_Sizes(face[1], TTF_UX, TTF_UY);
            if(f_face[2]) FT_Set_Pixel_Sizes(face[2], TTF_UX, TTF_UY);
            if(f_face[3]) FT_Set_Pixel_Sizes(face[3], TTF_UX, TTF_UY);

            FT_GlyphSlot slot = NULL;

            u8 bitmap[TEX_SZ * TEX_SZ];
            memset(bitmap, 0, TEX_SZ * TEX_SZ);

            ///////////

            FT_UInt index;

            if(f_face[0] && (index = FT_Get_Char_Index(face[0], ttf_char))!=0 
                && !FT_Load_Glyph(face[0], index, FT_LOAD_RENDER )) slot = face[0]->glyph;
            else if(f_face[1] && (index = FT_Get_Char_Index(face[1], ttf_char))!=0 
                && !FT_Load_Glyph(face[1], index, FT_LOAD_RENDER )) slot = face[1]->glyph;
            else if(f_face[2] && (index = FT_Get_Char_Index(face[2], ttf_char))!=0 
                && !FT_Load_Glyph(face[2], index, FT_LOAD_RENDER )) slot = face[2]->glyph;
            else if(f_face[3] && (index = FT_Get_Char_Index(face[3], ttf_char))!=0 
                && !FT_Load_Glyph(face[3], index, FT_LOAD_RENDER )) slot = face[3]->glyph;
            else ttf_char = 0;

            if(ttf_char!=0) {
                ww = ww2 = 0;

                int y_correction = TTF_UY - 1 - slot->bitmap_top;
                if(y_correction < 0) y_correction = 0;

                ttf_font_datas[l].flags = 1;
                ttf_font_datas[l].y_start = y_correction;
                ttf_font_datas[l].height = slot->bitmap.rows;
                ttf_font_datas[l].width = slot->bitmap.width;
                ttf_font_datas[l].ttf = ttf_char;
                

                for(n = 0; n < slot->bitmap.rows; n++) {
                    if(n >= TEX_SZ) break;
                    for (m = 0; m < slot->bitmap.width; m++) {

                        if(m >= TEX_SZ) continue;
                        
                        colorc = (u8) slot->bitmap.buffer[ww + m];
                        
                        if(colorc) bitmap[m + ww2] = colorc; //(colorc<<8) | 0xfff;
                    }
                
                ww2 += TEX_SZ;

                ww += slot->bitmap.width;
                }
                ttf_font_datas[l].text[0] = create_texture(bitmap, 0x000000FF);
                ttf_font_datas[l].text[1] = create_texture(bitmap, 0xFFFFFFFF);
            }
            else continue;
        }

        // displaying the character
        ttf_font_datas[l].flags |= 2; // in use
        ttf_font_datas[l].r_use = r_use;

        if((Win_flag & WIN_AUTO_LF) && (posx + (ttf_font_datas[l].width * sw / TEX_SZ) + 1) > Win_W_ttf) {
            posx = 0;
            posy += sh;
        }

        u32 ccolor = color;
        u32 cx =(ttf_font_datas[l].width * sw / TEX_SZ) + 1;

        // skip if out of window
        if((posx + cx) > Win_W_ttf || (posy + sh) > Win_H_ttf ) ccolor = 0;

        if(ccolor) {
//            tiny3d_SetTextureWrap(0, tiny3d_TextureOffset(bitmap), 32, 32, 32 * 2,
//                TINY3D_TEX_FORMAT_A4R4G4B4, TEXTWRAP_CLAMP, TEXTWRAP_CLAMP, TEXTURE_LINEAR);

            if (bkcolor != 0) DrawBox_ttf((float) (Win_X_ttf + posx), (float) (Win_Y_ttf + posy) + ((float) ttf_font_datas[l].y_start * sh) * 0.03125f,
            Z_ttf, (float) sw, (float) sh, bkcolor);
            DrawTextBox_ttf(ttf_font_datas[l].text[((color & 0xFFFFFF00) != 0)], (float) (Win_X_ttf + posx), (float) (Win_Y_ttf + posy) + ((float) ttf_font_datas[l].y_start * sh) * 0.015625f,
            Z_ttf, (float) sw, (float) sh, color, 0.99f, 0.99f);
        }

        posx+= cx;
    }

    Y_ttf = (float) posy + sh;

    if(posx < lenx) posx = lenx;
    return posx;
}

int width_ttf_string(const char *string, int sw, int sh)
{
    return (display_ttf_string(0, 0, string, 0, 0, sw, sh, NULL));
}
