#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "vector.cpp"
#include <windows.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define _CRT_SECURE_NO_WARNINGS
#define SCREEN_WIDTH	800 //szerokoœæ
#define SCREEN_HEIGHT	600 //wysokoœæ
#define HORSE_ANIMATIONS_FRAMES 18 //klatki animacji
#define SPEED 300 //szybkoœæ w pixelach
#define GRAVITY 500 //si³a gravitacji
#define STAR_POINTS 100 //punkty za gwiazdê
#define FAIRY_POINTS 10 //punkty za wró¿kê
#define MAX_LIVES 3 //pocz¹tkowe ¿ycia

const int DEFAULT_X = SCREEN_WIDTH / 10;	//po³o¿enie pocz¹tkowe x
const int DEFAULT_Y = SCREEN_HEIGHT /2;	//po³o¿enie pocz¹tkowe y


// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y)
{
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1)
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

//³adowanie sprita
SDL_Surface* loadSurface(char path[])
{
	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if (loadedSurface == NULL) {
		printf("SDL_LoadBMP(%s) error: %s\n", path, SDL_GetError());

		return 0;
	}
	return loadedSurface;
}

SDL_Surface* optimalizeSurface(char path[], SDL_Surface* screen)
{
	SDL_Surface* optimalizedSurface = NULL;
	SDL_Surface* loadedSurface = loadSurface(path);
	optimalizedSurface = SDL_ConvertSurface(loadedSurface, screen->format, 0);
	if (optimalizedSurface == NULL) {
		printf("SDL_LoadBMP %s error: %s\n", path, SDL_GetError());

		return 0;
	}
	SDL_FreeSurface(loadedSurface);
	return optimalizedSurface;
}

//zwalnianie pamieci
void Cleanup(SDL_Surface* charset, SDL_Surface* eti, SDL_Surface* eti_dash, SDL_Surface* grass, SDL_Surface* dead_screen, SDL_Surface* star, SDL_Surface* fairy, SDL_Surface* empty_heart, SDL_Surface* full_heart, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* tex_eti, SDL_Texture* eti_dash_tex)
{
	//surface
	SDL_FreeSurface(charset);
	SDL_FreeSurface(eti);
	SDL_FreeSurface(eti_dash);
	SDL_FreeSurface(grass);
	SDL_FreeSurface(dead_screen);
	SDL_FreeSurface(star);
	SDL_FreeSurface(fairy);
	SDL_FreeSurface(empty_heart);
	SDL_FreeSurface(full_heart);
	SDL_FreeSurface(screen);

	//scrtex window renderer
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	//textury
	SDL_DestroyTexture(tex_eti);
	SDL_DestroyTexture(eti_dash_tex);
	SDL_QUIT;
}

void zero_array(SDL_Rect* array, int n)
{
	for (int i = 0; i < n; i++)
		array[i].x = 0;
}

//czyszczenie tablic
void FreeArrays(int level_height, char**& map, SDL_Rect* wall, SDL_Rect* star_textures, SDL_Rect* fairies_tex)
{
	//mapa
	for (int i = 0; i < level_height; i++)
		delete[] map[i];
	delete[] map;
	map = NULL;

	//hitboxy
	//delete[] wall;
	wall = NULL;
	//delete[] star_textures;
	star_textures=NULL;
	//delete[] fairies_tex;
	fairies_tex=NULL;
}

//deadscreen
void died(SDL_Surface *screen, SDL_Surface* charset,int score,int LIVES)
{
	char text[128];
	sprintf(text, "You died, your score: %d. Lives left: %d. Save your score? [S] ", score,LIVES);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 50, text, charset);
	if (LIVES > 0)
	{
		sprintf(text, "N - new game, ESC - leave");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 80, text, charset);
	}
	else
	{
		sprintf(text, "No more lives, you lost");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 80, text, charset);
		sprintf(text, "N - back to menu, ESC - leave");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 100, text, charset);
	}
	//SDL_Delay(500);
}

//graficzne wyœwietlanie zyæ
void display_lives(SDL_Surface* screen, SDL_Surface* empty_heart, SDL_Surface* full_heart, int LIVES)
{

	for(int i=0;i<LIVES;i++)
		DrawSurface(screen, full_heart, full_heart->w/2+i* full_heart->w, 25);
	for(int i=MAX_LIVES;i>LIVES;i--)
		DrawSurface(screen, empty_heart, empty_heart->w / 2 + (i-1) * empty_heart->w, 25);

}
//fizyka skakania
int jump_fun(bool ground, double& jump_start_time, double worldTime)
{
	int jump_velocity=0;
	//skok
	if (ground == true)
		jump_start_time = worldTime;

	else if (ground == true)
		jump_start_time = 0;

	jump_velocity = (worldTime - jump_start_time) * GRAVITY * 1.5;


	return jump_velocity;
}

//wyœwietlenie w³aœnie zdobytych punktów 
void display_points(SDL_Surface* screen, SDL_Surface* charset,SDL_Rect hitbox, int points)
{
	char gained_points[10]="";
	sprintf(gained_points, "%d", points);
	DrawString(screen, hitbox.x + hitbox.w + 5, hitbox.y -10,gained_points,charset);
}

//usuniêcie zniszczonej gwiazdy
void destroy_star(SDL_Rect hitbox, SDL_Rect star_texture, int x_pos,char** map)
{
	int i = star_texture.y/64;

	int j = (star_texture.x + x_pos) / 64;

	map[i][j] = 'x';
}

//usuniêcie zniszczonej gwiazdy
void destroy_fairy(SDL_Rect hitbox, SDL_Rect fairy_tex, int x_pos, char** map)
{
	int i = fairy_tex.y / 64;

	int j = (fairy_tex.x + x_pos) / 64;

	map[i][j] = 'f';
}

//losowe pojawienie siê obiektów
void rand_stars_and_fairies(char**map,int level_height,int level_width,int odds)
{
	for (int i = 0; i < level_height; i++)
	{
		int random1 = 0;
		int random2 = 0;
		for (int j = 0; j < level_width; j++)
		{
			if (map[i][j] == 'x')
			{
				random1 = rand() % 100;
				if (random1 >= 100-odds)
					map[i][j] = 'X';
			}
			else if (map[i][j] == 'f')
			{
				random2 = rand() % 100;
				if (random2 >= 100 - odds)
					map[i][j] = 'F';
			}
		}
	}

}

//usuwanie wylosowanych elementów do ponownego losowania
void delete_all_objects(char** map, int level_height, int level_width)
{
	for (int i = 0; i < level_height; i++)
	{
		for (int j = 0; j < level_width; j++)
		{
			if (map[i][j] == 'X')
				map[i][j] = 'x';

			if (map[i][j] == 'F')
				map[i][j] = 'f';
		}
	}
}

//sprawdzenie kolizji z gwiadami i dodanie punktów
void StarCollision(int stars, SDL_Rect& hitbox, SDL_Rect* star_textures, bool dash, int& taken_stars, int x_pos, char** map, int level_width, int& objectPoints, int& points, double& got_object_time, double worldTime, bool& dead, bool& start)
{
	for (int i = 0; i < stars; i++)
	{
		if (SDL_HasIntersection(&hitbox, &star_textures[i]) == true)
		{
			//zniszczenie gwiazdy
			if (dash == true)
			{
				taken_stars++;
				destroy_star(hitbox, star_textures[i], x_pos, map);
				star_textures[i] = { level_width,0,0,0 };
				objectPoints = taken_stars * STAR_POINTS;
				points += objectPoints;
				got_object_time = worldTime;
			}
			//œmieræ		
			else
			{
				dead = true;
				start = false;
				break;
			}
		}
	}
}

//sprawdzenie kolizji z wró¿kami i dodanie punktów
void FairyCollision(int fairies, SDL_Rect& hitbox, SDL_Rect* fairies_tex, int& taken_fairies, int x_pos, char** map, int& objectPoints, int& points, double& got_object_time, double worldTime)
{
	for (int i = 0; i < fairies; i++)
	{
		if (SDL_HasIntersection(&hitbox, &fairies_tex[i]) == true)
		{
			//zebranie wró¿ki

			taken_fairies++;
			destroy_fairy(hitbox, fairies_tex[i], x_pos, map);
			fairies_tex[i] = { 0,0,0,0 };
			objectPoints = taken_fairies * FAIRY_POINTS;
			points += objectPoints;
			got_object_time = worldTime;
		
		}
	}
}


// main
#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char** argv) {

	srand(time(NULL));

	int t1 = 0, t2 = 0, quit, frames = 0;
	double delta = 0, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface* charset, * eti, * eti_dash,*dead_screen;
	SDL_Surface* screen;
	SDL_Surface* grass, * background,*star,*fairy;
	SDL_Surface* full_heart, * empty_heart;
	SDL_Texture* scrtex, *tex_eti, * eti_dash_tex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	//wczytywanie mapy i zapisów
	FILE* read;
	int retValue = 0;
	read = fopen("map.txt", "r");
	if (read == NULL)
	{
		printf("b³ad pliku\n");
		exit(1);
	}

	 int level_height;
	 int level_width ;
	 int textures = 0;
	 int stars = 0;
	 int fairies = 0;
	 int odds = 0;
	 
	 //wczytywanie parametrów
	retValue = fscanf(read, "%d", &level_height);
	retValue = fscanf(read, "%d", &level_width);
	retValue = fscanf(read, "%d", &odds);

	const int LEVEL_WIDTH = level_width * 64;
	const int LEVEL_HEIGHT = level_height * 64;

	char** map = new char* [level_height];   //generowanie tablicy dynamicznej
	for(int i=0;i< level_height;++i)
	{
		map[i] = new char[level_width];
	}

	//wczytywanie pliku z map¹
	for (int i = 0; i < level_height; i++)
	{
		for (int j = 0; j < level_width; j++)
		{
			char sign;

			//liczenie poszczególnych obiektów
			retValue = fscanf(read, "%c", &sign);
			if (sign == 'M') textures++;
			if (sign == 'x') stars++;
			if (sign == 'f') fairies++;
			if (sign != '\n' && sign != ' ')
			{
				map[i][j] = sign;
			}
			else j--;
		}
	}		

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy
	//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,&window, &renderer);
	// tryb okienkowy
	window = SDL_CreateWindow("Robot Unicorn Attack", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (window == NULL || renderer == NULL) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = loadSurface("images/cs8x8.bmp");
	SDL_SetColorKey(charset, true, 0x000000);


	eti = loadSurface("images/eti.bmp");
	eti_dash = loadSurface("images/eti_dash.bmp");
	grass = loadSurface("images/grass64.bmp");
	star = loadSurface("images/star.bmp");
	fairy = loadSurface("images/fairy.bmp");

	background = loadSurface("images/sky.bmp");
	dead_screen = loadSurface("images/deadscreen.bmp");

	full_heart = loadSurface("images/full.bmp");
	empty_heart = loadSurface("images/empty.bmp");

	tex_eti=SDL_CreateTextureFromSurface(renderer, eti);
	eti_dash_tex=SDL_CreateTextureFromSurface(renderer, eti_dash);

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	bool restart = false;

//BEGINING
	do
	{
		restart = false;
		SDL_Surface* menu;
		menu = loadSurface("images/menu.bmp");

		//inicjacja wartoœci
		frames = 0;
		fpsTimer = 0;
		fps = 0;
		quit = 0;
		worldTime = 0;
		distance = 0;
		etiSpeed = 1;

		//liczba ¿yæ
		int LIVES = MAX_LIVES;

		int frame = 0;
		bool start = false;

		//po³o¿enie pocz¹tkowe
		int x_pos = DEFAULT_X;
		int y_pos = DEFAULT_Y;
		int x_vel = 0;
		int y_vel = 0;

		int taken_stars = 0;
		int taken_fairies = 0;

		float score = 0;
		int points = 0;
		int up = 0, up_original = 0;
		int down = 0;
		int left = 0;
		int right = 0;
		int last_x_vel = SPEED;
		bool dash = false;
		bool original = false;
		bool jump = false, jump_begin = false;
		bool ground;
		bool begin = true;
		bool dead = false;
		bool just_dead = false;
		double jump_start_time = 0;
		bool double_jump = true;
		double dash_begin = 0;
		double got_object_time = 0;
		bool falling = false;
		int objectPoints = 0;

		//hitbox gracza
		SDL_Rect hitbox = { x_pos,y_pos,eti->w,eti->h };

		//tablice obiektów
		SDL_Rect* wall = new SDL_Rect[textures];
		SDL_Rect* star_textures = new SDL_Rect[stars];
		zero_array(star_textures, stars);
		SDL_Rect* fairies_tex = new SDL_Rect[fairies];
		zero_array(fairies_tex, fairies);

		//losowanie na pocz¹tek gry
		rand_stars_and_fairies(map, level_height, level_width, odds);

		/////////////////////////////////////////////////////////////////////////main loop
		while (!quit) {

			// obs³uga zdarzeñ (o ile jakieœ zasz³y)
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode)
					{
					case SDL_SCANCODE_ESCAPE: quit = 1; break;
					case SDL_SCANCODE_N: start = true; dead = false;
						if (LIVES == 0)
						{
							start = false;
							//restart gry
							restart = true;
							//goto BEGINING;	
							break;
						}
						break;
					case SDL_SCANCODE_LEFT:  left = 1; break;
					case SDL_SCANCODE_RIGHT: right = 1; break;
					case SDL_SCANCODE_UP:   jump = true; up = 1; break;
					case SDL_SCANCODE_DOWN:  down = 1; break;
					case SDL_SCANCODE_D:  original = true; break;
					case SDL_SCANCODE_Z:if (original)
					{
						up_original = 1; jump = true;
					}break;
					case SDL_SCANCODE_X:if (original)
					{
						dash = true; dash_begin = worldTime;
					}break;
					}
					break;
				case SDL_KEYUP:
					etiSpeed = 1;
					switch (event.key.keysym.scancode)
					{
					case SDL_SCANCODE_LEFT:  left = 0; break;
					case SDL_SCANCODE_RIGHT: right = 0; break;
					case SDL_SCANCODE_UP:  jump = false;  up = 0; break;
					case SDL_SCANCODE_DOWN:  down = 0; break;
					case SDL_SCANCODE_Z:if (original)
					{
						up_original = 0; jump = false;
					} break;
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};

			};
			//pomijanie pêtli przy restarcie
			if (restart == true) quit = 1;

			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			SDL_RenderClear(renderer);

			//start poziomu
			if (start == false || dead == true)
			{
				DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
				//nowa gra
				if (!start && menu != NULL)
					DrawSurface(screen, menu, menu->w / 2.7, menu->h / 2);

				//œmieræ
				else if (dead)
				{
					//jednorazowy reset danych po œmierci
					if (!just_dead)
					{
						delete_all_objects(map, level_height, level_width);
						rand_stars_and_fairies(map, level_height, level_width, odds);
						dash = false;
						score = worldTime * 10;
						points = 0;
						LIVES--;
						frames = 0;
						fpsTimer = 0;
						worldTime = 0;
						just_dead = true;
						x_pos = DEFAULT_X;
						y_pos = DEFAULT_Y;
						jump_start_time = 0;
						hitbox.x = x_pos;
						hitbox.y = y_pos;
						got_object_time = 0;
						taken_stars = 0;
					}
					//render deadscreena
					DrawSurface(screen, dead_screen, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
					died(screen, charset, score, LIVES);

				}

				SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
				SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			}
			else
			{
				if (menu != NULL)
				{
					t1 = SDL_GetTicks();
					SDL_FreeSurface(menu);
					menu = NULL;
				}
				just_dead = false;
				//czas
				t2 = SDL_GetTicks();
				// w tym momencie t2-t1 to czas w milisekundach,
				// jaki uplyna³ od ostatniego narysowania ekranu
				// delta to ten sam czas w sekundach
				delta = (t2 - t1) * 0.001;
				t1 = t2;
				worldTime += delta;
				//czas
				fpsTimer += delta;
				if (fpsTimer > 0.5) {
					fps = frames * 2;
					frames = 0;
					fpsTimer -= 0.5;
				};

				SDL_RenderClear(renderer);

				//t³o
				int grass_count = 0;
				int stars_count = 0;
				int fairies_count = 0;
				DrawSurface(screen, background, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 1.5);

				//render mapy
				for (int i = 0; i < level_height; i++)
				{
					for (int j = 0; j < level_width; j++)
					{
						if (map[i][j] == 'M')
						{
							//obiekty na mapie
							DrawSurface(screen, grass, (32 + j * grass->w) - x_pos, 32 + i * grass->w);
							//hitboxy
							wall[grass_count] = { (j * grass->w) - x_pos,i * grass->w,grass->w,grass->h };
							grass_count++;
						}
						else if (map[i][j] == 'X')
						{
							//gwiazdy na mapie
							DrawSurface(screen, star, (32 + j * star->w) - x_pos, 32 + i * star->w);
							//hitboxy
							star_textures[stars_count] = { (j * grass->w) - x_pos,i * star->w,star->w,star->h };
							stars_count++;
						}
						else if (map[i][j] == 'F')
						{
							//wró¿ki na mapie
							DrawSurface(screen, fairy, (32 + j * fairy->w) - x_pos, 32 + i * fairy->w);
							//hitboxy
							fairies_tex[fairies_count] = { (j * fairy->w) - x_pos,i * fairy->w,fairy->w,fairy->h };
							fairies_count++;
						}
					}
				}

				//¿ycia
				display_lives(screen, empty_heart, full_heart, LIVES);

				//prêdkoœæ
				x_vel = 0;  y_vel = 0;
				ground = false;

				//ruch strza³kami
				if (original == false)
				{
					if (up && !down) y_vel = -2.5 * SPEED - GRAVITY;
					if (down && !up) y_vel = SPEED;
					if (left && !right) x_vel = -1 * SPEED;
					if (right && !left) x_vel = SPEED;
				}
				//ruch oryginalny
				else
				{
					if (dash)
						x_vel = last_x_vel;
					else
						x_vel = 5 * (int)worldTime + SPEED;

					last_x_vel = x_vel;

					if (up_original)  y_vel = -2.5 * SPEED - GRAVITY;
					if (dash)
					{
						if (worldTime - dash_begin > 0.5)
						{
							jump_start_time = worldTime;
							dash = false;
							double_jump = true;
						}
					}
				}

				//zawsze dzia³a gravitacja
				y_vel += GRAVITY;

				//kolizje z obiektami
				bool collision = false;
				for (int i = 0; i < textures; i++)
				{
					if (SDL_HasIntersection(&hitbox, &wall[i]) == true)
					{
						//punkty
						//A
						int xAu = wall[i].x + 4; int yAu = wall[i].y;
						int xAd = wall[i].x; int yAd = wall[i].y + 12;
						//B
						int xBu = wall[i].x + wall[i].w - 4; int yBu = wall[i].y;
						int xBd = wall[i].x + wall[i].w; int yBd = wall[i].y + 12;
						//C
						int xCu = wall[i].x + wall[i].w; int yCu = wall[i].y + wall[i].h - 12;
						int xCd = wall[i].x + wall[i].w - 4; int yCd = wall[i].y + wall[i].h;
						//D
						int xDu = wall[i].x; int yDu = wall[i].y + wall[i].h - 12;
						int xDd = wall[i].x + 4; int yDd = wall[i].y + wall[i].h;

						//kolizja z góry
						if (SDL_IntersectRectAndLine(&hitbox, &xAu, &yAu, &xBu, &yBu))
						{
							y_pos = wall[i].y - eti->w;
							hitbox.y = wall[i].y - hitbox.w;
							ground = true;
							double_jump = true;
							falling = false;

						}
						//kolizja z do³u
						if (SDL_IntersectRectAndLine(&hitbox, &xDd, &yDd, &xCd, &yCd))
						{
							y_pos = wall[i].y + grass->w;
							//break;
						}
						//kolizja z prawej -> œmieræ
						if (SDL_IntersectRectAndLine(&hitbox, &xBd, &yBd, &xCu, &yCu))
						{
							start = false;
							dead = true;
							//x_pos = wall[i].x + grass->w;
							break;
						}
						//kolizja z lewej -> œmieræ
						else if (SDL_IntersectRectAndLine(&hitbox, &xAd, &yAd, &xDu, &yDu))
						{
							start = false;
							dead = true;
							//x_pos = wall[i].x - eti->w;
							break;
						}
					}
				}
				bool got_star = false;

				//kolizje z gwiazdami
				StarCollision(stars, hitbox, star_textures, dash, taken_stars, x_pos, map, level_width, objectPoints, points, got_object_time, worldTime, dead, start);

				//kolizje z wró¿kami
				FairyCollision(fairies, hitbox, fairies_tex, taken_fairies, x_pos, map, objectPoints, points, got_object_time, worldTime);

				//ograniczenie szybkoœci spadania
				if (y_vel > 0) falling = true;
				if (y_vel > GRAVITY) y_vel = GRAVITY;


				//double jump
				if (ground == 0 && jump && falling && double_jump)
				{
					double_jump = false;
					jump_start_time = worldTime;
				}

				//skok
				y_vel += jump_fun(ground, jump_start_time, worldTime);

				//dash
				if (dash) y_vel = 0;

				//pozycja
				x_pos += (int)x_vel / 120;
				y_pos += (int)y_vel / 120;

				//kolizje z ekranem
				if ((x_pos < 0) || (x_pos + eti->w > LEVEL_WIDTH - SCREEN_WIDTH / 5))
					x_pos -= x_vel / 120;
				if (y_pos < 0)
					y_pos -= y_vel / 120;
				if (y_pos > LEVEL_HEIGHT)
					dead = true;

				//pêtla + losowanie przeszkód
				if (x_pos > LEVEL_WIDTH - 14 * grass->w)
				{
					x_pos = DEFAULT_X;
					delete_all_objects(map, level_height, level_width);
					rand_stars_and_fairies(map, level_height, level_width, odds);
				}

				//ruch
				hitbox.x = x_pos;
				hitbox.y = y_pos;

				//ograniczenie na ekranie
				if (x_pos > DEFAULT_X)
					hitbox.x = DEFAULT_X;


				// tekst informacyjny
				DrawRectangle(screen, 200, 4, SCREEN_WIDTH - 200, 20, czerwony, niebieski);

				sprintf(text, "Robot Unicorn Attack, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
				DrawString(screen, screen->w / 1.5 - strlen(text) * 8 / 2, 10, text, charset);

				//wynik
				score = worldTime * 10 + points;
				sprintf(text, "Wynik: %.0lf", score);
				DrawString(screen, screen->w / 1.5 - strlen(text) * 8 / 2, 36, text, charset);

				//zebrane punkty
				if (worldTime - got_object_time < 2 && got_object_time != 0)
					display_points(screen, charset, hitbox, objectPoints);

				//tekstury ekranu
				SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
				SDL_RenderCopy(renderer, scrtex, NULL, NULL);

				//render gracza
				if (dash == false)
					SDL_RenderCopy(renderer, tex_eti, NULL, &hitbox);
				else
					SDL_RenderCopy(renderer, eti_dash_tex, NULL, &hitbox);
			}

			//update screen
			SDL_RenderPresent(renderer);
			if (dead)
				SDL_Delay(500);

			frames++;
		};

		if(restart==false)
			FreeArrays(level_height, map, wall, star_textures, fairies_tex);

	SDL_FreeSurface(menu);


	}while (restart == true);

	// zwolnienie pamiêci


	fclose(read);

	Cleanup(charset, eti, eti_dash, grass, dead_screen, star, fairy, empty_heart, full_heart, screen, scrtex, window, renderer, tex_eti, eti_dash_tex);

	return 0;
}
