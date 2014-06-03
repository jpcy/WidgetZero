/*
The MIT License (MIT)

Copyright (c) 2014 Jonathan Young

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <SDL.h>
#include <widgetzero/wzsdl2.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace wz {

struct RendererPrivate
{
	SDL_Renderer *renderer;
	stbtt_fontinfo font;
	float fontHeight;
	uint8_t *fontFileBuffer;
	static const size_t nGlyphs = 256;
	stbtt_bakedchar glyphs[nGlyphs];
	SDL_Surface *glyphAtlasSurface;
	SDL_Texture *glyphAtlasTexture;
};

Renderer::Renderer(SDL_Renderer *renderer)
{
	p = new RendererPrivate;
	p->renderer = renderer;
}

Renderer::~Renderer()
{
	delete p;
}

SDL_Renderer *Renderer::get()
{
	return p->renderer;
}

std::string Renderer::initialize(const char *fontFilename, float fontHeight)
{
	// Read font file into buffer.
	FILE *f = fopen(fontFilename, "rb");

	if (!f)
	{
		std::ostringstream ss;
		ss << "Error loading font '" << fontFilename << "'";
		return ss.str();
	}

	fseek(f, 0, SEEK_END);
	size_t length = (size_t)ftell(f);
	fseek(f, 0, SEEK_SET);
	p->fontFileBuffer = (uint8_t *)malloc(length);
	size_t bytesRead = fread(p->fontFileBuffer, 1, length, f);
	fclose(f);

    if (bytesRead != length)
    {
		std::ostringstream ss;
		ss << "Error reading font file " << fontFilename << ". Tried to read " << length << " bytes, got " << bytesRead << ".";
		return ss.str();
    }

	// Create glyph atlas and set a greyscale palette.	
	p->glyphAtlasSurface = SDL_CreateRGBSurface(0, 512, 512, 8, 0, 0, 0, 0);

	if (!p->glyphAtlasSurface)
	{
		return SDL_GetError();
	}

	SDL_Palette palette;
	SDL_Color colors[256];
	palette.ncolors = 256;
	palette.colors = colors;
	
	for (int i = 0; i < palette.ncolors; i++)
	{
		colors[i].r = colors[i].g = colors[i].b = colors[i].a = i;
	}
	
	SDL_SetSurfacePalette(p->glyphAtlasSurface, &palette);

	// Create font and bake glyphs to the atlas.
	stbtt_InitFont(&p->font, p->fontFileBuffer, stbtt_GetFontOffsetForIndex(p->fontFileBuffer, 0));
	stbtt_BakeFontBitmap(p->fontFileBuffer, 0, fontHeight, (unsigned char *)p->glyphAtlasSurface->pixels, p->glyphAtlasSurface->w, p->glyphAtlasSurface->h, 0, p->nGlyphs, p->glyphs);
	p->fontHeight = fontHeight;

	// Create a texture from the glyph atlas.
	p->glyphAtlasTexture = SDL_CreateTextureFromSurface(p->renderer, p->glyphAtlasSurface);

	if (!p->glyphAtlasTexture)
	{
		return SDL_GetError();
	}

	SDL_SetTextureBlendMode(p->glyphAtlasTexture, SDL_BLENDMODE_BLEND);
	return std::string();
}

void Renderer::textPrintf(int x, int y, TextAlignment halign, TextAlignment valign, uint8_t r, uint8_t g, uint8_t b, const char *format, ...)
{
	static char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	SDL_SetTextureColorMod(p->glyphAtlasTexture, r, g, b);
	float cursorX = (float)x;
	float cursorY = (float)y;

	if (halign == TA_CENTER || halign == TA_RIGHT)
	{
		int width, height;
		measureText(buffer, &width, &height);

		if (halign == TA_CENTER)
		{
			cursorX -= width / 2.0f;
		}
		else if (halign == TA_RIGHT)
		{
			cursorX -= width;
		}
	}

	if (valign == TA_TOP || valign == TA_CENTER)
	{
		float scale = stbtt_ScaleForMappingEmToPixels(&p->font, p->fontHeight);
		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&p->font, &ascent, &descent, &lineGap);

		if (valign == TA_TOP)
		{
			cursorY += ascent * scale;
		}
		else if (valign == TA_CENTER)
		{
			cursorY += (ascent + descent) * scale / 2.0f;
		}
	}

	for (size_t i = 0; i < strlen(buffer); i++)
	{
		stbtt_bakedchar *g = &p->glyphs[buffer[i]];

		SDL_Rect src;
		src.x = g->x0;
		src.y = g->y0;
		src.w = g->x1 - g->x0;
		src.h = g->y1 - g->y0;

		SDL_Rect dest;
		dest.x = (int)(cursorX + g->xoff);
		dest.y = (int)(cursorY + g->yoff);
		dest.w = src.w;
		dest.h = src.h;

		SDL_RenderCopy(p->renderer, p->glyphAtlasTexture, &src, &dest);
		cursorX += g->xadvance;
	}
}

void Renderer::measureText(const char *text, int *width, int *height)
{
	float total = 0;

	for (size_t i = 0; i < strlen(text); i++)
	{
		stbtt_bakedchar *g = &p->glyphs[text[i]];
		total += g->xadvance;
	}

	if (width)
	{
		*width = (int)total;
	}

	if (height)
	{
		*height = (int)p->fontHeight;
	}
}

//------------------------------------------------------------------------------

static void DrawWidget(wzWidget *widget, wzRect clip)
{
	((Widget *)wz_widget_get_metadata(widget))->draw(clip);
}

void Widget::setPosition(int x, int y)
{
	wz_widget_set_position_args(getWidget(), x, y);
}

void Widget::setRect(int x, int y, int w, int h)
{
	wzRect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	wz_widget_set_rect(getWidget(), rect);
}

void Widget::clipReset()
{
	SDL_Rect rect;
	rect.x = rect.y = rect.w = rect.h = 0;
	SDL_RenderSetClipRect(renderer_->get(), &rect);
}

void Widget::clipToRect(wzRect rect)
{
	SDL_RenderSetClipRect(renderer_->get(), (const SDL_Rect *)&rect);
}

bool Widget::clipToRectIntersection(wzRect rect1, wzRect rect2)
{
	wzRect intersection;

	if (!SDL_IntersectRect((const SDL_Rect *)&rect1, (const SDL_Rect *)&rect2, (SDL_Rect *)&intersection))
	{
		return false;
	}

	SDL_RenderSetClipRect(renderer_->get(), (const SDL_Rect *)&intersection);
	return true;
}

//------------------------------------------------------------------------------

static void DrawDockIcon(wzRect rect, void *metadata)
{
	Desktop *desktop = (Desktop *)metadata;
	desktop->drawDockIcon(rect);
}

static void DrawDockPreview(wzRect rect, void *metadata)
{
	Desktop *desktop = (Desktop *)metadata;
	desktop->drawDockPreview(rect);
}

Desktop::Desktop(Renderer *renderer)
{
	desktop_ = wz_desktop_create();
	renderer_ = renderer;
	wz_widget_set_metadata((wzWidget *)desktop_, this);
	wz_desktop_set_draw_dock_icon_callback(desktop_, DrawDockIcon, this);
	wz_desktop_set_draw_dock_preview_callback(desktop_, DrawDockPreview, this);
}

Desktop::~Desktop()
{
	wz_widget_destroy((wzWidget *)desktop_);
}

void Desktop::setSize(int w, int h)
{
	wz_widget_set_size_args((wzWidget *)desktop_, w, h);
}

void Desktop::mouseMove(int x, int y, int dx, int dy)
{
	wz_desktop_mouse_move(desktop_, x, y, dx, dy);
}

void Desktop::mouseButtonDown(int button, int x, int y)
{
	wz_desktop_mouse_button_down(desktop_, button, x, y);
}

void Desktop::mouseButtonUp(int button, int x, int y)
{
	wz_desktop_mouse_button_up(desktop_, button, x, y);
}

void Desktop::mouseWheelMove(int x, int y)
{
	wz_desktop_mouse_wheel_move(desktop_, x, y);
}

void Desktop::draw()
{
	wz_desktop_draw(desktop_);
	clipReset();
}

void Desktop::drawDockIcon(wzRect rect)
{
	clipReset();
	SDL_SetRenderDrawBlendMode(renderer_->get(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer_->get(), 64, 64, 64, 128);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);
	SDL_SetRenderDrawBlendMode(renderer_->get(), SDL_BLENDMODE_NONE);
}

void Desktop::drawDockPreview(wzRect rect)
{
	clipReset();
	SDL_SetRenderDrawBlendMode(renderer_->get(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer_->get(), 0, 0, 128, 64);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);
	SDL_SetRenderDrawBlendMode(renderer_->get(), SDL_BLENDMODE_NONE);
}

//------------------------------------------------------------------------------

Window::Window(Widget *parent, char *title)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(title_, title);
	window_ = wz_window_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)window_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_window_set_border_size(window_, 4);

	// Calculate header height based on label text plus padding.
	renderer_->measureText(title_, &size.w, &size.h);
	size.h += 6;
	wz_window_set_header_height(window_, size.h);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Window::draw(wzRect clip)
{
	clipReset();
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)window_);
	
	// Background.
	SDL_SetRenderDrawColor(renderer_->get(), 255, 255, 255, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border.
	const int borderSize = wz_window_get_border_size(window_);
	SDL_SetRenderDrawColor(renderer_->get(), 128, 128, 128, 255);

	// Border top.
	wzRect borderRect = rect;
	borderRect.h = borderSize;
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&borderRect);

	// Border bottom.
	borderRect.y = rect.y + rect.h - borderSize;
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&borderRect);

	// Border left.
	borderRect = rect;
	borderRect.w = borderSize;
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&borderRect);

	// Border right.
	borderRect.x = rect.x + rect.w - borderSize;
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&borderRect);

	// Header.
	wzRect headerRect = wz_window_get_header_rect(window_);

	if (headerRect.w > 0 && headerRect.h > 0)
	{
		clipToRect(headerRect);
		SDL_SetRenderDrawColor(renderer_->get(), 255, 232, 166, 255);
		SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&headerRect);
		renderer_->textPrintf(headerRect.x + 10, headerRect.y + headerRect.h / 2, Renderer::TA_LEFT, Renderer::TA_CENTER, 0, 0, 0, title_);
	}
}

//------------------------------------------------------------------------------

Button::Button(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);

	// Calculate size based on label text plus padding.
	renderer_->measureText(label_, &size.w, &size.h);
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Button::Button(wzButton *button, const char *label)
{
	wzWidget *widget = (wzWidget *)button;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	strcpy(label_, label);
	button_ = button;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
}

wzRect Button::getRect()
{
	return wz_widget_get_absolute_rect((wzWidget *)button_);
}

void Button::draw(wzRect clip)
{
	clipToRect(clip);

	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)button_);
	const bool hover = wz_widget_get_hover((wzWidget *)button_);
	const bool pressed = wz_button_is_pressed(button_);

	// Background.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 127, 194, 229, 255);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 218, 218, 218, 255);
	}

	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border.
	if (pressed && hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&rect);

	// Label.
	renderer_->textPrintf(rect.x + rect.w / 2, rect.y + rect.h / 2, Renderer::TA_CENTER, Renderer::TA_CENTER, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

Checkbox::Checkbox(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	button_ = wz_button_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)button_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_button_set_set_behavior(button_, WZ_BUTTON_SET_BEHAVIOR_TOGGLE);

	// Calculate size.
	renderer_->measureText(label_, &size.w, &size.h);
	size.w += boxSize + boxRightMargin;
	size.w += 16;
	size.h += 8;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Checkbox::draw(wzRect clip)
{
	clipToRect(clip);

	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)button_);
	const bool hover = wz_widget_get_hover((wzWidget *)button_);

	// Box.
	SDL_Rect boxRect;
	boxRect.x = rect.x;
	boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f);
	boxRect.w = boxSize;
	boxRect.h = boxSize;

	// Box background.
	if (wz_button_is_pressed(button_) && hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 127, 194, 229, 255);
		SDL_RenderFillRect(renderer_->get(), &boxRect);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 188, 229, 252, 255);
		SDL_RenderFillRect(renderer_->get(), &boxRect);
	}

	// Box border.
	SDL_SetRenderDrawColor(renderer_->get(), 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer_->get(), &boxRect);

	// Box checkmark.
	if (wz_button_is_set(button_))
	{
		boxRect.x = rect.x + 4;
		boxRect.y = (int)(rect.y + rect.h / 2.0f - boxSize / 2.0f) + 4;
		boxRect.w = boxSize - 8;
		boxRect.h = boxSize - 8;
		SDL_SetRenderDrawColor(renderer_->get(), 0, 0, 0, 255);
		SDL_RenderFillRect(renderer_->get(), &boxRect);
	}

	// Label.
	renderer_->textPrintf(rect.x + boxSize + boxRightMargin, rect.y + rect.h / 2, Renderer::TA_LEFT, Renderer::TA_CENTER, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

Combo::Combo(Widget *parent, const char **items, int nItems)
{
	renderer_ = parent->getRenderer();
	items_ = items;
	combo_ = wz_combo_create(wz_widget_get_desktop(parent->getWidget()));
	wzWidget *widget = (wzWidget *)combo_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
	
	list_.reset(new List(wz_combo_get_list(combo_), items, nItems));
}

void Combo::draw(wzRect clip)
{
	clipToRect(clip);

	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)combo_);
	const bool hover = wz_widget_get_hover((wzWidget *)combo_);

	// Background.
	if (hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 218, 218, 218, 255);
	}

	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border.
	if (hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&rect);

	// Label.
	int itemIndex = wz_list_get_selected_item((const wzList *)list_->getWidget());

	if (itemIndex >= 0)
		renderer_->textPrintf(rect.x + 10, rect.y + rect.h / 2, Renderer::TA_LEFT, Renderer::TA_CENTER, 0, 0, 0, items_[itemIndex]);
}

//------------------------------------------------------------------------------

GroupBox::GroupBox(Widget *parent, const char *label)
{
	renderer_ = parent->getRenderer();

	wzWidget *widget;
	wzSize size;

	strcpy(label_, label);
	groupBox_ = wz_groupbox_create(wz_widget_get_desktop(parent->getWidget()));
	widget = (wzWidget *)groupBox_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);

	size.w = 200;
	size.h = 200;
	wz_widget_set_size(widget, size);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void GroupBox::draw(wzRect clip)
{
	const int textLeftMargin = 20;
	const int textBorderSpacing = 5;

	clipToRect(clip);
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)groupBox_);
	
	// Background.
	SDL_SetRenderDrawColor(renderer_->get(), 255, 255, 255, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border - left, bottom, right, top left, top right.
	int textWidth, textHeight;
	renderer_->measureText(label_, &textWidth, &textHeight);
	SDL_SetRenderDrawColor(renderer_->get(), 98, 135, 157, 255);
	SDL_RenderDrawLine(renderer_->get(), rect.x, rect.y + textHeight / 2, rect.x, rect.y + rect.h);
	SDL_RenderDrawLine(renderer_->get(), rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(renderer_->get(), rect.x + rect.w, rect.y + textHeight / 2, rect.x + rect.w, rect.y + rect.h);
	SDL_RenderDrawLine(renderer_->get(), rect.x, rect.y + textHeight / 2, rect.x + textLeftMargin - textBorderSpacing, rect.y + textHeight / 2);
	SDL_RenderDrawLine(renderer_->get(), rect.x + textLeftMargin + textWidth + textBorderSpacing * 2, rect.y + textHeight / 2, rect.x + rect.w, rect.y + textHeight / 2);

	// Label.
	renderer_->textPrintf(rect.x + textLeftMargin, rect.y, Renderer::TA_LEFT, Renderer::TA_TOP, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

Scroller::Scroller(Widget *parent, wzScrollerType type, int value, int stepValue, int maxValue)
{
	renderer_ = parent->getRenderer();
	scroller_ = wz_scroller_create(wz_widget_get_desktop(parent->getWidget()), type);
	wz_scroller_set_max_value(scroller_, maxValue);
	wz_scroller_set_value(scroller_, value);
	wz_scroller_set_step_value(scroller_, stepValue);
	wzWidget *widget = (wzWidget *)scroller_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);

	wz_widget_add_child_widget(parent->getWidget(), widget);
}

Scroller::Scroller(wzScroller *scroller)
{
	wzWidget *widget = (wzWidget *)scroller;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	scroller_ = scroller;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_scroller_set_nub_size(scroller_, 16);

	decrementButton.reset(new Button(wz_scroller_get_decrement_button(scroller_), "-"));
	incrementButton.reset(new Button(wz_scroller_get_increment_button(scroller_), "+"));

	// Width will be ignored for vertical scrollers, height for horizontal. The scroller width/height will be automatically used for the buttons.
	wzSize buttonSize;
	buttonSize.w = 16;
	buttonSize.h = 16;
	wz_widget_set_size((wzWidget *)wz_scroller_get_decrement_button(scroller_), buttonSize);
	wz_widget_set_size((wzWidget *)wz_scroller_get_increment_button(scroller_), buttonSize);
}

void Scroller::draw(wzRect clip)
{
	clipToRect(clip);
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)scroller_);
	
	// Background.
	SDL_SetRenderDrawColor(renderer_->get(), 192, 192, 192, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Nub.
	wzRect nubRect = wz_scroller_get_nub_rect(scroller_);
	SDL_SetRenderDrawColor(renderer_->get(), 218, 218, 218, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&nubRect);
	SDL_SetRenderDrawColor(renderer_->get(), 112, 112, 112, 255);
	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&nubRect);
}

int Scroller::getValue() const
{
	return wz_scroller_get_value(scroller_);
}

//------------------------------------------------------------------------------

Label::Label(Widget *parent)
{
	renderer_ = parent->getRenderer();
	text_[0] = r = g = b = 0;
	label_ = wz_label_create(wz_widget_get_desktop(parent->getWidget()));
	wzWidget *widget = (wzWidget *)label_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
}

void Label::setText(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(text_, sizeof(text_), format, args);
	va_end(args);

	wzSize size;
	renderer_->measureText(text_, &size.w, &size.h);
	wz_widget_set_size((wzWidget *)label_, size);
}

void Label::setTextColor(uint8_t r, uint8_t g, uint8_t b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

void Label::draw(wzRect clip)
{
	clipToRect(clip);
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)label_);
	renderer_->textPrintf(rect.x, rect.y, Renderer::TA_LEFT, Renderer::TA_TOP, r, g, b, text_);
}

//------------------------------------------------------------------------------

List::List(Widget *parent, const char **items, int nItems)
{
	renderer_ = parent->getRenderer();
	list_ = wz_list_create(wz_widget_get_desktop(parent->getWidget()));
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	wzWidget *widget = (wzWidget *)list_;
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	wz_widget_add_child_widget(parent->getWidget(), widget);
	items_ = items;
	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));

	wzBorder border;
	border.left = border.right = border.top = border.bottom = itemsMargin;
	wz_list_set_items_border(list_, border);

	wzSize scrollerSize;
	scrollerSize.w = 16;
	scrollerSize.h = 0;
	wz_widget_set_size(scroller_->getWidget(), scrollerSize);
}

List::List(wzList *list, const char **items, int nItems)
{
	wzWidget *widget = (wzWidget *)list;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();

	list_ = list;
	wz_list_set_num_items(list_, nItems);
	wz_list_set_item_height(list_, itemHeight);
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget, DrawWidget);
	items_ = items;
	scroller_.reset(new Scroller(wz_list_get_scroller(list_)));

	wzBorder border;
	border.left = border.right = border.top = border.bottom = itemsMargin;
	wz_list_set_items_border(list_, border);

	wzSize scrollerSize;
	scrollerSize.w = 16;
	scrollerSize.h = 0;
	wz_widget_set_size(scroller_->getWidget(), scrollerSize);
}

void List::draw(wzRect clip)
{
	clipToRect(clip);
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)list_);
	
	// Background.
	SDL_SetRenderDrawColor(renderer_->get(), 255, 255, 255, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border.
	SDL_SetRenderDrawColor(renderer_->get(), 130, 135, 144, 255);
	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&rect);

	// Items.
	wzRect itemsRect = wz_list_get_absolute_items_rect(list_);

	if (!clipToRectIntersection(clip, itemsRect))
		return;

	int nItems = wz_list_get_num_items(list_);
	int scrollerValue = wz_scroller_get_value(wz_list_get_scroller(list_));
	int y = itemsRect.y - (scrollerValue % itemHeight);

	for (int i = wz_list_get_first_item(list_); i < nItems; i++)
	{
		// Outside widget?
		if (y > itemsRect.y + itemsRect.h)
			break;

		SDL_Rect itemRect;
		itemRect.x = itemsRect.x;
		itemRect.y = y;
		itemRect.w = itemsRect.w;
		itemRect.h = itemHeight;

		if (i == wz_list_get_selected_item(list_))
		{
			SDL_SetRenderDrawColor(renderer_->get(), 127, 194, 229, 255);
			SDL_RenderFillRect(renderer_->get(), &itemRect);
		}
		else if (i == wz_list_get_pressed_item(list_) || i == wz_list_get_hovered_item(list_))
		{
			SDL_SetRenderDrawColor(renderer_->get(), 188, 229, 252, 255);
			SDL_RenderFillRect(renderer_->get(), &itemRect);
		}

		renderer_->textPrintf(itemsRect.x + itemLeftPadding, y + itemHeight / 2, Renderer::TA_LEFT, Renderer::TA_CENTER, 0, 0, 0, items_[i]);
		y += itemHeight;
	}
}

//------------------------------------------------------------------------------

TabButton::TabButton(wzButton *button, const char *label)
{
	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop((wzWidget *)button));
	renderer_ = desktop->getRenderer();

	button_ = button;
	strcpy(label_, label);
	wz_widget_set_metadata((wzWidget *)button_, this);
	wz_widget_set_draw_function((wzWidget *)button_, DrawWidget);

	// Calculate width based on label text plus padding.
	int width;
	renderer_->measureText(label_, &width, NULL);
	width += 16;
	wz_widget_set_width((wzWidget *)button_, width);
}

void TabButton::draw(wzRect clip)
{
	clipToRect(clip);

	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)button_);
	const bool hover = wz_widget_get_hover((wzWidget *)button_);
	const bool set = wz_button_is_set(button_);

	// Background.
	if (set)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 127, 194, 229, 255);
	}
	else if (hover)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 188, 229, 252, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 218, 218, 218, 255);
	}

	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	// Border.
	if (set)
	{
		SDL_SetRenderDrawColor(renderer_->get(), 44, 98, 139, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer_->get(), 112, 112, 112, 255);
	}

	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&rect);

	// Label.
	renderer_->textPrintf(rect.x + rect.w / 2, rect.y + rect.h / 2, Renderer::TA_CENTER, Renderer::TA_CENTER, 0, 0, 0, label_);
}

//------------------------------------------------------------------------------

TabBar::TabBar(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabBar_ = wz_tab_bar_create(wz_widget_get_desktop(parent->getWidget()));

	wz_widget_set_metadata((wzWidget *)tabBar_, this);
	wz_widget_add_child_widget(parent->getWidget(), (wzWidget *)tabBar_);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

TabBar::TabBar(wzTabBar *tabBar)
{
	tabBar_ = tabBar;

	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop((wzWidget *)tabBar));
	renderer_ = desktop->getRenderer();

	wz_widget_set_metadata((wzWidget *)tabBar_, this);

	decrementButton.reset(new Button(wz_tab_bar_get_decrement_button(tabBar_), "<"));
	wz_widget_set_width((wzWidget *)decrementButton->getWidget(), 14);
	incrementButton.reset(new Button(wz_tab_bar_get_increment_button(tabBar_), ">"));
	wz_widget_set_width((wzWidget *)incrementButton->getWidget(), 14);
}

TabBar::~TabBar()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		delete tabs_[i];
	}
}

void TabBar::draw(wzRect clip)
{
}

void TabBar::addTab(const char *label)
{
	wzButton *tab = wz_tab_bar_add_tab(tabBar_);
	tabs_.push_back(new TabButton(tab, label));
}

void TabBar::addTab(wzButton *button, const char *label)
{
	tabs_.push_back(new TabButton(button, label));
}

//------------------------------------------------------------------------------

TabPage::TabPage(wzWidget *widget)
{
	widget_ = widget;
	Desktop *desktop = (Desktop *)wz_widget_get_metadata((wzWidget *)wz_widget_get_desktop(widget));
	renderer_ = desktop->getRenderer();
	wz_widget_set_metadata(widget, this);
	wz_widget_set_draw_function(widget_, DrawWidget);
}

void TabPage::draw(wzRect clip)
{
	clipToRect(clip);
	wzRect rect = wz_widget_get_absolute_rect((wzWidget *)widget_);

	SDL_SetRenderDrawColor(renderer_->get(), 224, 224, 224, 255);
	SDL_RenderFillRect(renderer_->get(), (SDL_Rect *)&rect);

	SDL_SetRenderDrawColor(renderer_->get(), 112, 112, 112, 255);
	SDL_RenderDrawRect(renderer_->get(), (SDL_Rect *)&rect);
}

//------------------------------------------------------------------------------

Tabbed::Tabbed(Widget *parent)
{
	renderer_ = parent->getRenderer();
	tabbed_ = wz_tabbed_create(wz_widget_get_desktop(parent->getWidget()));
	wz_widget_set_metadata((wzWidget *)tabbed_, this);
	wz_widget_add_child_widget(parent->getWidget(), (wzWidget *)tabbed_);
	tabBar_.reset(new TabBar(wz_tabbed_get_tab_bar(tabbed_)));
	tabBar_->setRect(0, 0, 0, 20);
}

Tabbed::~Tabbed()
{
	for (size_t i = 0; i < tabs_.size(); i++)
	{
		delete tabs_[i];
	}
}

void Tabbed::draw(wzRect clip)
{
}

TabPage *Tabbed::addTab(const char *label)
{
	wzButton *button;
	wzWidget *widget;
	wz_tabbed_add_tab(tabbed_, &button, &widget);
	tabBar_->addTab(button, label);
	
	TabPage *tab = new TabPage(widget);
	tabs_.push_back(tab);
	
	return tab;
}

} // namespace wz
