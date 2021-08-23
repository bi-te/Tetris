#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>

class SimpleTexture
{
	int posX = 0, posY = 0;
	int width = 0, height = 0;

	SDL_Texture* texture = nullptr;
public:
	SimpleTexture();
	SimpleTexture(std::string path, SDL_Renderer* renderer);
	SimpleTexture(int x, int y, std::string path, SDL_Renderer* renderer);
	SimpleTexture(TTF_Font* font, std::string text, SDL_Renderer* renderer, 
		SDL_Color fg, bool shaded, SDL_Color bg = {0});
	
	~SimpleTexture();

	bool loadTexture(std::string path, SDL_Renderer* renderer);
	bool loadTextureFromFont(TTF_Font* font, std::string text, SDL_Renderer* renderer,
		SDL_Color fg, bool shaded, SDL_Color bg = {0});
	void clearTexture();

	void render(SDL_Renderer* renderer, SDL_Rect* clip = nullptr);
	
	int get_pos_x() const;
	void set_pos_x(int pos_x);
	
	int get_pos_y() const;
	void set_pos_y(int pos_y);

	int get_width() const;
	void set_width(int width);

	int get_height() const;
	void set_height(int height);

	SDL_Texture* get_texture() const;
};
