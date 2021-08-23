#include "SimpleTexture.h"


SimpleTexture::SimpleTexture()
{
}


SimpleTexture::SimpleTexture(std::string path, SDL_Renderer* renderer)
{
	loadTexture(path, renderer);
}


SimpleTexture::SimpleTexture(int x, int y, std::string path, SDL_Renderer* renderer)
{
	posX = x;
	posY = y;
	loadTexture(path, renderer);
}


SimpleTexture::SimpleTexture(TTF_Font* font, std::string text, SDL_Renderer* renderer,
	SDL_Color fg, bool shaded, SDL_Color bg)
{
	loadTextureFromFont(font, text, renderer, fg, shaded, bg);
}

SimpleTexture::~SimpleTexture()
{
	clearTexture();
}

bool SimpleTexture::loadTexture(std::string path, SDL_Renderer* renderer)
{
	bool s = true;

	clearTexture();
	
	SDL_Surface* sur = IMG_Load(path.c_str());
	if (sur == nullptr)
	{
		s = false;
		std::cerr << IMG_GetError();
	} else
	{
		texture = SDL_CreateTextureFromSurface(renderer, sur);
		if (texture == nullptr)
		{
			s = false;
			std::cerr << SDL_GetError();
		} else
		{
			width = sur->w;
			height = sur->h;
		}
		SDL_FreeSurface(sur);
	}
	return s;
}


bool SimpleTexture::loadTextureFromFont(TTF_Font* font, std::string text, SDL_Renderer* renderer, SDL_Color fg, bool shaded, SDL_Color bg  )
{
	bool s = true;

	SDL_Surface* sur;
	
	clearTexture();
	if(shaded)
	{
		sur = TTF_RenderText_Shaded(font, text.c_str(), fg, bg);
	} else
	{
		sur = TTF_RenderText_Blended(font, text.c_str(), fg);
	}
	
	if (sur == nullptr)
	{
		std::cerr << TTF_GetError();
	}
	else
	{
		texture = SDL_CreateTextureFromSurface(renderer, sur);
		if (texture == nullptr)
		{
			s = false;
			std::cerr << SDL_GetError();
		} else
		{
			width = sur->w;
			height = sur->h;
		}
		SDL_FreeSurface(sur);
	}

	return s;
}

void SimpleTexture::clearTexture()
{
	SDL_DestroyTexture(texture);
}


void SimpleTexture::render(SDL_Renderer* renderer, SDL_Rect* clip)
{
	SDL_Rect r{posX, posY};
	if (clip == nullptr)
	{
		r.w = width;
		r.h = height;
	} else
	{
		r.w = clip->w;
		r.h = clip->h;
	}
	SDL_RenderCopy(renderer, texture, clip, &r);
}

int SimpleTexture::get_pos_x() const
{
	return posX;
}

void SimpleTexture::set_pos_x(int pos_x)
{
	posX = pos_x;
}

int SimpleTexture::get_pos_y() const
{
	return posY;
}

void SimpleTexture::set_pos_y(int pos_y)
{
	posY = pos_y;
}

int SimpleTexture::get_width() const
{
	return width;
}

void SimpleTexture::set_width(int width)
{
	this->width = width;
}

int SimpleTexture::get_height() const
{
	return height;
}

void SimpleTexture::set_height(int height)
{
	this->height = height;
}

SDL_Texture* SimpleTexture::get_texture() const
{
	return texture;
}
