/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


// Font class
// Created 15/7/01
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "GfxPrimitives.h"


char Fontstr[256] = {" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~~"};//\161\162\163\164\165\166\167\168\169\170\171\172\173\174\175\176\177\178\179\180\181\182\183\184\185\186\187\188\189\190\191\192\193\194\195\196\197\198\199\200\201\202\203\204\205\206\207\208\209\210\211\212\213\214\215\216\217\218\219\220\221\222\223\224\225\226\227\228\229\230\231\232\233\234\235\236\237\238\239\240\241\242\243\244\245\246\247\248\249\250\251\252\253\254\255"};
size_t Fontstr_len = strlen(Fontstr);


///////////////////
// Load a font
int CFont::Load(const std::string& fontname, bool _colour, int _width)
{
	LOAD_IMAGE(bmpFont,fontname);

	Colour = _colour;
	Width = _width;

	if(!bmpFont) return false;

	bmpWhite = gfxCreateSurface(bmpFont->w,bmpFont->h);
	bmpGreen = gfxCreateSurface(bmpFont->w,bmpFont->h);

	// Calculate the font width for each character
	CalculateWidth();

	PreCalculate(bmpWhite,ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpWhite->format));
	PreCalculate(bmpGreen,ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpGreen->format));


		// Pre-calculate some colours
	f_pink = ConvertColor(tLX->clPink,SDL_GetVideoSurface()->format,bmpFont->format);
	f_blue = ConvertColor(tLX->clHeading,SDL_GetVideoSurface()->format,bmpFont->format);//SDL_MapRGB(bmpFont->format,0,0,255);
	f_white = ConvertColor(tLX->clNormalLabel,SDL_GetVideoSurface()->format,bmpFont->format);//MakeColour(255,255,255);
	f_green = ConvertColor(tLX->clChatText,SDL_GetVideoSurface()->format,bmpFont->format);//MakeColour(0,255,0);

	// Must do this after PreCalculate
	SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, f_pink);

	return true;
}


///////////////////
// Shutdown the font
void CFont::Shutdown(void)
{
	if(bmpWhite)  {
		SDL_FreeSurface(bmpWhite);
		bmpWhite = NULL;
	}
	if(bmpGreen)  {
		SDL_FreeSurface(bmpGreen);
		bmpGreen = NULL;
	}
}


///////////////////
// Calculate Widths
void CFont::CalculateWidth(void)
{
	unsigned int n;
	Uint32 pixel;
	int i,j;
	int a,b;

	// Lock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);

	Uint32 blue = SDL_MapRGB(bmpFont->format,0,0,255);

	for(n=0;n<Fontstr_len;n++) {
		a=n*Width;
		for(j=0;j<bmpFont->h;j++) {
			for(i=a,b=0;b<Width;i++,b++) {

				pixel = GetPixel(bmpFont,i,j);
				if(pixel == blue) {
					FontWidth[n] = b;
					break;
				}
			}
		}
	}


	// Unlock the surface
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);
}


///////////////////
// Precalculate a font's colour
void CFont::PreCalculate(SDL_Surface *bmpSurf, Uint32 colour)
{
	Uint32 pixel;
	int x,y;

	DrawRectFill(bmpSurf,0,0,bmpSurf->w,bmpSurf->h,tLX->clPink);
	SDL_BlitSurface(bmpFont,NULL,bmpSurf,NULL);


	// Replace black with the appropriate colour
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_LockSurface(bmpSurf);

	for(y=0;y<bmpSurf->h;y++) {
		for(x=0;x<bmpSurf->w;x++) {
			pixel = GetPixel(bmpSurf,x,y);

			if(pixel == 0)
				PutPixel(bmpSurf,x,y,colour);
		}
	}


	// Unlock the surface
	if(SDL_MUSTLOCK(bmpSurf))
		SDL_UnlockSurface(bmpSurf);

	SDL_SetColorKey(bmpSurf, SDL_SRCCOLORKEY, tLX->clPink);
}


///////////////////
// Set to true, if this is an outline font
void CFont::SetOutline(int Outline) {
	OutlineFont = Outline;
}

///////////////////
// Is this an outline font?
int CFont::IsOutline(void) {
	return OutlineFont;
}

//////////////////
// Draw a text
void CFont::Draw(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt,...) {
	va_list arg;

	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawAdv(dst,x,y,99999,col,std::string(buf));
}

void CFont::Draw(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	DrawAdv(dst,x,y,99999,col,txt);
}


void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, char *fmt,...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);
	
	DrawAdv(dst, x, y, max_w, col, std::string(buf));
}

///////////////////
// Draw a font (advanced)
void CFont::DrawAdv(SDL_Surface *dst, int x, int y, int max_w, Uint32 col, const std::string& txt) {
	int pos=0;
	short l;
	Uint32 pixel;
	int i,j;
	int w;
	int a,b;
	int length = Fontstr_len;


	// Clipping rectangle
	SDL_Rect rect = dst->clip_rect;

	int	top = rect.y;
	int left = rect.x;
	int right = rect.x + rect.w;
	int bottom = rect.y + rect.h;


	// Lock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont))
		SDL_LockSurface(bmpFont);


	Uint32 col2 = (Uint32)col;

	pos=0;
	for(std::string::const_iterator p = txt.begin(); p != txt.end(); p++) {
		l = *p - 32;

		// Line break
		if (*p == '\n')  {
			y += bmpFont->h+3;
			pos = 0;
			continue;
		}

		// Maximal width overflowed
		// TODO: doesn't support multiline texts, but it's faster...
		if (pos > max_w)
			break;

        // Ignore unkown characters
        if(l >= length || l < 0)
            continue;

		w=0;
		a=l*Width;

		if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, tLX->clPink);
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
			pos+=FontWidth[l];
			continue;
		}

		if (!OutlineFont)  {
			if (col2 == f_white)  {
				DrawImageAdv(dst,bmpWhite,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			} else if (!col2) {
				DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			} else if( col2 == f_green) {
				DrawImageAdv(dst,bmpGreen,a,0,x+pos,y,FontWidth[l],bmpFont->h);
				pos+=FontWidth[l];
				continue;
			}
		}

		/*if(!Colour) {
			SDL_SetColorKey(bmpFont, SDL_SRCCOLORKEY, tLX->clPink);
			DrawImageAdv(dst,bmpFont,a,0,x+pos,y,FontWidth[l],bmpFont->h);
		}
		else */{
			Uint8 *src = (Uint8 *)bmpFont->pixels + a * bmpFont->format->BytesPerPixel;
			Uint8 *p;
			int bpp = bmpFont->format->BytesPerPixel;
			for(j=0;j<bmpFont->h;j++) {
				p = src;
				for(i=a,b=0;b<FontWidth[l];i++,b++,p+=bpp) {

					// Clipping
					if(x+pos+b < left)
						continue;
					if(y+j < top)
						break;

					if(y+j >= bottom)
						break;
					if(x+pos+b >= right)
						break;

					pixel = GetPixelFromAddr(p,bpp);

					if(pixel == f_pink)
						continue;

					if(pixel == 0 && OutlineFont)  {
							PutPixel(dst,x+pos+b,y+j,0);
							continue;
					}


					if(Colour)
						PutPixel(dst,x+pos+b,y+j,col);
				}
				src+= bmpFont->pitch;
			}
		}

		pos+=FontWidth[l];
	}


	// Unlock the surfaces
	if(SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
	if(SDL_MUSTLOCK(bmpFont))
		SDL_UnlockSurface(bmpFont);

}


///////////////////
// Calculate the width of a string of text
int CFont::GetWidth(const std::string& buf) {
	int length = 0;
	short l;
	
	// Calculate the length of the text
	size_t fontstrlen = Fontstr_len;
	for(std::string::const_iterator p = buf.begin(); p != buf.end(); p++) {
		l = *p - 32;
		if(l <= 0 || (ushort)l >= fontstrlen)
			continue;

		length += FontWidth[l];
	}

	return length;
}


void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = x-length/2;
	Draw(dst,pos,y,col,txt);
}

///////////////////
// Draws the text in centre alignment
void CFont::DrawCentre(SDL_Surface *dst, int x, int y, Uint32 col, char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawCentre(dst, x,y,col, std::string(buf));
}


void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, const std::string& txt) {
	int length = GetWidth(txt);
	int pos = MAX(min_x, x-length/2);
	DrawAdv(dst,pos,y,max_w,col,txt);
}

///////////////////
// Draw's the text in centre alignment
void CFont::DrawCentreAdv(SDL_Surface *dst, int x, int y, int min_x, int max_w, Uint32 col, char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	static char buf[512];
	vsnprintf(buf, sizeof(buf),fmt, arg);
	fix_markend(buf);
	va_end(arg);

	DrawCentreAdv(dst, x, y, min_x, max_w, col, std::string(buf));
}
