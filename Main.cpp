#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "SimpleTexture.h"

uint16_t WINDOW_HEIGHT = 768;
uint16_t WINDOW_WIDTH = 1024;


const uint8_t t_rows = 22;
const uint8_t t_rows_shown = 20;
const uint8_t t_columns = 10;
const uint8_t piece_size = 30;
const uint8_t total_blocks = 7;

enum Rotation{UP, RIGHT, DOWN, LEFT, ROTATIONS};

struct InputState
{
	enum Inputs{L = -1, NONE, R};
	Inputs rotation = NONE;
	Inputs move = NONE;
	bool fall = false;
};

struct Media
{
	enum Fonts { MAIN_FONT, GAME_OVER, UI_FONT, TOTAL_FONTS};
	TTF_Font* fonts[TOTAL_FONTS];
	SimpleTexture* gameOverTexture = nullptr;
	SimpleTexture* restartTexture = nullptr;
};

uint8_t block_I[]
{
	0, 0, 0, 0,
	1, 1, 1, 1,
	0, 0, 0, 0,
	0, 0, 0, 0
};

uint8_t block_J[] = {
	1, 0, 0,
	1, 1, 1,
	0, 0, 0
};

uint8_t block_L[] = {
	0, 0, 1,
	1, 1, 1,
	0, 0, 0
};

uint8_t block_O[] = {
	1, 1,
	1, 1
};

uint8_t block_S[] = {
	0, 1, 1,
	1, 1, 0,
	0, 0, 0 
};

uint8_t block_T[] = {
	0, 1, 0,
	1, 1, 1,
	0, 0, 0
};

uint8_t block_Z[] = {
	1, 1, 0,
	0, 1, 1,
	0, 0, 0
};

struct Tetramino
{
	uint8_t* block;
	uint8_t side;
};

struct CurrentBlock
{
	Tetramino* tetramino;
	Rotation rotation;
	
	int8_t row, column;
	int8_t fRow;
};

struct GameRules
{
	const uint8_t maxLevel = 10;
	const uint8_t linesToLevelUp = 10;
	
	uint8_t levelTimeDecrease = 50;
	uint16_t scoreForLines[5]{ 0, 40, 100, 300, 1200 };
	Uint32 fallTime = 500;
};



struct GameState
{

	uint8_t level = 0;
	uint32_t score = 0;
	uint16_t linesCleared = 0;
	
	Uint32 passedTime = 0;
	
	uint8_t table[t_rows][t_columns] = {};
	InputState input_state;

	bool game_over = false;
	bool restart = false;
	
	Tetramino blocks[total_blocks]
	{
		Tetramino{block_I, 4},
		Tetramino{block_J, 3},
		Tetramino{block_L, 3},
		Tetramino{block_O, 2},
		Tetramino{block_S, 3},
		Tetramino{block_T, 3},
		Tetramino{block_Z, 3}
	};
	CurrentBlock current_block;

	uint16_t t_posX, t_posY;
	
};

bool init();

bool loadMedia(Media* media, SDL_Renderer* renderer);

bool createWinRen(SDL_Window** window, SDL_Renderer** renderer);

void render(SDL_Renderer* renderer, GameState* game_state, Media* media);

void drawTable(SDL_Renderer* renderer, GameState* game_state);

void drawBlock(SDL_Renderer* renderer, GameState* game_state);

void drawScore(SDL_Renderer* renderer, GameState* game_state, Media* media);

void update(GameState* game_state, GameRules* game_rules);

void spawn(GameState* game_state);

void addToTable(GameState* game_state);

uint8_t fullRows(GameState* game_state);

void drop(GameState* game_state, GameRules* game_rules);

void testNext(GameState* game_state, GameRules* game_rules);

int8_t checkCollision(GameState* game_state);

uint8_t getCell(CurrentBlock& cb, uint8_t i, uint8_t k);

void findFuture(GameState* game_state);

void restart(GameState* game_state);

void calcScore(GameState* game_state, GameRules* game_rules, uint8_t lines);

void close(Media* media);


int main(int arg, char* args[])
{
	srand(time(nullptr));
	
	if (!init())
	{
		return 1;
	}

	SDL_Window* mainWindow = nullptr;
	SDL_Renderer* mainWindowRenderer = nullptr;
	if (!createWinRen(&mainWindow, &mainWindowRenderer))
	{
		return 1;
	}

	Media media;

	if(!loadMedia(&media, mainWindowRenderer))
	{
		return 1;
	}

	GameState game_state;
	game_state.t_posX = (WINDOW_WIDTH - piece_size * t_columns) / 2;
	game_state.t_posY = (WINDOW_HEIGHT - piece_size * t_rows_shown) / 2;
	GameRules game_rules;
	
	
	bool quit = false;
	SDL_Event e;
	const Uint8* scans = SDL_GetKeyboardState(NULL);
	Uint32 time, newTime;
	newTime = SDL_GetTicks();
	
	spawn(&game_state);
	while(!quit)
	{
		time = newTime;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			} else if(e.type == SDL_KEYDOWN)
			{
				if(e.key.keysym.sym == SDLK_r)
				{
					restart(&game_state);
				}
			}

			if (scans[SDL_SCANCODE_Z])
			{
				game_state.input_state.rotation = InputState::L;
			}
			else if (scans[SDL_SCANCODE_X])
			{
				game_state.input_state.rotation = InputState::R;
			}

			if(scans[SDL_SCANCODE_LEFT])
			{
				game_state.input_state.move = InputState::L;
			} else if (scans[SDL_SCANCODE_RIGHT])
			{
				game_state.input_state.move = InputState::R;
			}

			if(scans[SDL_SCANCODE_DOWN])
			{
				game_state.input_state.fall = true;
			}
		}

		newTime = SDL_GetTicks();
		if(!game_state.game_over)
		{
			game_state.passedTime += newTime - time;
			update(&game_state, &game_rules);
		}
		
		render(mainWindowRenderer, &game_state, &media);
	}

	SDL_DestroyWindow(mainWindow);
	SDL_DestroyRenderer(mainWindowRenderer);
	
	close(&media);
	
	return 0;
}

bool init()
{
	bool s = true;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		s = false;
		std::cerr << SDL_GetError();
	} else
	{
		if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
		{
			s = false;
			std::cerr << IMG_GetError();
		}
		if(TTF_Init() == -1)
		{
			s = false;
			std::cerr << TTF_GetError();
		}
	}

	return s;
}

bool loadMedia(Media* media, SDL_Renderer* renderer)
{
	bool s = true;

	media->fonts[Media::MAIN_FONT] = TTF_OpenFont("media/fonts/BebasNeue-Regular.ttf", 18);
	media->fonts[Media::GAME_OVER] = TTF_OpenFont("media/fonts/junegull.ttf", 72);
	media->fonts[Media::UI_FONT] = TTF_OpenFont("media/fonts/Marlboro.ttf", 36);
	if(media->fonts[Media::MAIN_FONT] == nullptr 
		|| media->fonts[Media::GAME_OVER] == nullptr
		|| media->fonts[Media::UI_FONT] == nullptr)
	{
		s = false;
		std::cerr << TTF_GetError();
	} else
	{
		media->gameOverTexture = new SimpleTexture(media->fonts[Media::GAME_OVER],
			"GAME OVER", renderer, SDL_Color{ 0, 255, 70 , 255 }, true,
			SDL_Color{ 0, 0, 0, 120 });
		
		media->restartTexture = new SimpleTexture(media->fonts[Media::UI_FONT], 
			"Press R to restart", renderer, SDL_Color{ 0, 255, 70 , 255 },true,
			SDL_Color{ 0, 0, 0, 120 });
	}
	
	
	return s;
}


bool createWinRen(SDL_Window** window, SDL_Renderer** renderer)
{
	bool s = true;
	*window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (*window == nullptr)
	{
		std::cerr << SDL_GetError();
		s = false;
	} else
	{
		*renderer = SDL_CreateRenderer(*window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (renderer == nullptr)
		{
			std::cerr << SDL_GetError();
			s = false;
		}
	}
	
	return s;
}

void render(SDL_Renderer* renderer, GameState* game_state, Media* media)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	drawTable(renderer, game_state);
	drawScore(renderer, game_state, media);
	
	if (game_state->game_over)
	{
		SimpleTexture*& t = media->gameOverTexture;
		SimpleTexture*& rt = media->restartTexture;

		t->set_pos_x((WINDOW_WIDTH - t->get_width()) / 2);
		t->set_pos_y((WINDOW_HEIGHT - t->get_height()) / 2);
		t->render(renderer);

		rt->set_pos_x((WINDOW_WIDTH - rt->get_width()) / 2);
		rt->set_pos_y(t->get_pos_y() + t->get_height() + 10);
		rt->render(renderer);
	}
	else
	{
		drawBlock(renderer, game_state);
	}

	SDL_RenderPresent(renderer);
}

void drawTable(SDL_Renderer* renderer, GameState* game_state)
{
	SDL_Rect piece{ 0, game_state->t_posY, piece_size, piece_size };

	for (auto i = 0; i < t_rows_shown; i++)
	{
		piece.y += piece_size;
		piece.x = game_state->t_posX;
		for (auto k = 0; k < t_columns; k++)
		{
			piece.x += piece_size;


			if (game_state->table[i][k])
			{
				SDL_SetRenderDrawColor(renderer, 0, 150, 200, 255);
				SDL_RenderFillRect(renderer, &piece);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawRect(renderer, &piece);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 57, 57, 57, 157);
				SDL_RenderDrawRect(renderer, &piece);
			}
		}
	}
}

void drawBlock(SDL_Renderer* renderer, GameState* game_state)
{
	SDL_Rect piece{ 0, game_state->t_posY + game_state->current_block.fRow * piece_size, piece_size, piece_size };

	for (auto i = 0; i < game_state->current_block.tetramino->side; i++)
	{
		piece.y += piece_size;
		piece.x = game_state->t_posX + game_state->current_block.column * piece_size;
		for (auto k = 0; k < game_state->current_block.tetramino->side; k++)
		{
			piece.x += piece_size;

			if (getCell(game_state->current_block, i, k) && i + game_state->current_block.fRow >= 0)
			{
				SDL_SetRenderDrawColor(renderer, 0, 150, 200, 255);
				SDL_RenderDrawRect(renderer, &piece);
			}
		}
	}

	piece.y = game_state->t_posY + game_state->current_block.row * piece_size;

	for (auto i = 0; i < game_state->current_block.tetramino->side; i++)
	{
		piece.y += piece_size;
		piece.x = game_state->t_posX + game_state->current_block.column * piece_size;
		for (auto k = 0; k < game_state->current_block.tetramino->side; k++)
		{
			piece.x += piece_size;

			if (getCell(game_state->current_block, i, k) && i + game_state->current_block.row >= 0)
			{
				SDL_SetRenderDrawColor(renderer, 0, 150, 200, 255);
				SDL_RenderFillRect(renderer, &piece);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawRect(renderer, &piece);
			}
		}
	}
}

void drawScore(SDL_Renderer* renderer, GameState* game_state, Media* media)
{
	SimpleTexture* score = new SimpleTexture(media->fonts[Media::UI_FONT],
		"Score: " + std::to_string(game_state->score), renderer,
		SDL_Color{ 255, 255, 255, 255 }, false);

	score->set_pos_x(10);
	score->set_pos_y((WINDOW_HEIGHT - score->get_height()) / 2);
	score->render(renderer);

	SimpleTexture* level = new SimpleTexture(media->fonts[Media::UI_FONT],
		"Level: " + std::to_string(game_state->level), renderer,
		SDL_Color{ 255, 255, 255, 255 }, false);
	
	level->set_pos_x(10);
	level->set_pos_y(score->get_pos_y() + score->get_height() + 10);
	level->render(renderer);
	
	delete score;
	delete level;
}



void update(GameState* game_state, GameRules* game_rules)
{
	if(game_state->input_state.move)
	{
		game_state->current_block.column += game_state->input_state.move;
		if (checkCollision(game_state) != -1)
		{
			game_state->current_block.column -= game_state->input_state.move;
		}
		game_state->input_state.move = InputState::NONE;
	}
	
	if(game_state->input_state.rotation)
	{
		game_state->current_block.rotation = (Rotation)((game_state->current_block.rotation + game_state->input_state.rotation + 4) % ROTATIONS);
		int8_t k = checkCollision(game_state);
		if (k != -1)
		{
			if(k < game_state->current_block.tetramino->side / 2)
			{
				k++;
			} else
			{
				k = game_state->current_block.tetramino->side - k;
				k *= -1;
			}

			game_state->current_block.column += k;
			if(checkCollision(game_state) != -1)
			{
				game_state->current_block.rotation = (Rotation)((game_state->current_block.rotation - game_state->input_state.rotation + 4) % ROTATIONS);
				game_state->current_block.column -= k;
			}
			
		}
		game_state->input_state.rotation = InputState::NONE;
		
	}
	testNext(game_state, game_rules);
	
	if(game_state->input_state.fall)
	{
		
		drop(game_state, game_rules);
		game_state->input_state.fall = false;
	}

	while(game_state->passedTime > game_rules->fallTime)
	{
		testNext(game_state, game_rules);
		game_state->passedTime -= game_rules->fallTime;
		drop(game_state,game_rules);
	}

	findFuture(game_state);
}

void spawn(GameState* game_state)
{
	
	CurrentBlock& cb = game_state->current_block;
	cb.tetramino = &(game_state->blocks[rand()%total_blocks]);
	cb.rotation = UP;
	cb.row = -1;
	cb.column = 3;

	if(checkCollision(game_state) != -1)
	{
		game_state->game_over = true;
		game_state->passedTime = 0;
	}
	cb.row--;
}


void addToTable(GameState* game_state)
{
	CurrentBlock& cb = game_state->current_block;
	for(auto i = 0; i < cb.tetramino->side; i++)
	{
		for(auto k = 0; k < cb.tetramino->side; k++)
		{
			if(getCell(cb, i, k) == 1 && cb.row + i >= 0)
			{
				game_state->table[cb.row + i][cb.column + k] = 1;
			}
			
		}
	}
}

uint8_t fullRows(GameState* game_state)
{
	uint8_t totalFullRows = 0;
	bool fullRow;
	for(auto i = 0; i < t_rows_shown; i ++)
	{
		fullRow = true;
		for (auto k = 0; k < t_columns && fullRow; k++)
		{
			if (game_state->table[i][k] != 1)
			{
				fullRow = false;
			}
		}

		if (fullRow)
		{
			totalFullRows++;
			
			for(auto j = i; j > 0; j--)
			{
				for (auto k = 0; k < t_columns; k++)
				{
					game_state->table[j][k] = game_state->table[j - 1][k];
				}
			}
		}
	}
	return totalFullRows;
}

int8_t checkCollision(GameState* game_state)
{
	int8_t s = -1;
	CurrentBlock& cb = game_state->current_block;

	uint8_t place;
	int8_t cc, cr;
	for(auto i = 0; i < cb.tetramino->side && s == -1; i++)
	{
		for (auto k = 0; k < cb.tetramino->side && s == -1; k++)
		{
			cc = k + cb.column;
			cr = i + cb.row;

			if(cr < 0)
			{
				place = 0;
			} else if(cc >=0 && cc < t_columns && cr < t_rows_shown)
			{
				place = game_state->table[cb.row + i][cb.column + k];
			} else
			{
				place = 1;
			}
			 if(getCell(cb, i, k) && place)
			 {
			 	if(!(k > cb.tetramino->side / 2 && s > k))
			 	{
					s = k;
			 	}
				 
			 }
		}
	}
	
	return s;
}

void testNext(GameState* game_state, GameRules* game_rules)
{
	game_state->current_block.row++;
	if (checkCollision(game_state) != -1)
	{
		game_state->current_block.row -= 1;
		addToTable(game_state);
		calcScore(game_state, game_rules, fullRows(game_state));
		spawn(game_state);

	} else
	{
		game_state->current_block.row -= 1;
	}
}


inline void drop(GameState* game_state, GameRules* game_rules)
{
	game_state->current_block.row++;
	testNext(game_state, game_rules);
}

void findFuture(GameState* game_state)
{
	CurrentBlock& cb = game_state->current_block;
	int8_t cr = cb.row;
	for (; cb.row < t_rows_shown; cb.row++)
	{
		
		if(checkCollision(game_state) != -1)
		{
			cb.fRow = cb.row - 1;
			cb.row = t_rows_shown;
		}
	}
	cb.row = cr;
}



uint8_t getCell(CurrentBlock& cb, uint8_t i, uint8_t k)
{
	uint8_t res;
	switch (cb.rotation)
	{
	case UP: res = cb.tetramino->block[i * cb.tetramino->side + k]; break;
	case RIGHT: res = cb.tetramino->block[cb.tetramino->side * (cb.tetramino->side - k - 1) + i]; break;
	case DOWN: res = cb.tetramino->block[(cb.tetramino->side - i - 1) * cb.tetramino->side + (cb.tetramino->side - k - 1)]; break;
	case LEFT: res = cb.tetramino->block[k * cb.tetramino->side + (cb.tetramino->side - i - 1)]; break;
		
	}
	return res;
}

void restart(GameState* game_state)
{
	
	for(auto i = 0; i < t_rows_shown; i ++)
	{
		for(auto k = 0; k < t_columns; k++)
		{
			game_state->table[i][k] = 0;
		}
	}

	game_state->game_over = false;
	game_state->score = 0;
	game_state->level = 0;
	spawn(game_state);
}

inline void calcScore(GameState* game_state, GameRules* game_rules, uint8_t lines)
{
	game_state->linesCleared += lines;
	if(game_state->linesCleared >= 10)
	{
		game_state->linesCleared -= 10;
		game_state->level++;
		if(game_state->level >= 10)
		{
			game_state->level = 10;
		} else
		{
			game_rules->fallTime -= game_rules->levelTimeDecrease;
		}
		
	}
	game_state->score += game_rules->scoreForLines[lines] * (game_state->level + 1);
}

void close(Media* media)
{
	delete media->gameOverTexture;
	delete media->restartTexture;
	for (int i = 0; i < media->TOTAL_FONTS; ++i)
	{
		TTF_CloseFont(media->fonts[i]);
	}
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
}
