void jump_fun(bool ground, double& jump_start_time, double worldTime, int& jump_velocity);

void Cleanup(SDL_Surface* charset, SDL_Surface* eti, SDL_Surface* eti_dash, SDL_Surface* grass, SDL_Surface* menu, SDL_Surface* dead_screen, SDL_Surface* star, SDL_Surface* fairy, SDL_Surface* empty_heart, SDL_Surface* full_heart, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* tex_eti, SDL_Texture* eti_dash_tex);

void FreeArrays(int level_height, char**& map, SDL_Rect* wall, SDL_Rect* star_textures, SDL_Rect* fairies_tex);

void StarCollision(int stars, SDL_Rect& hitbox, SDL_Rect* star_textures, bool dash, int& taken_stars, int x_pos, char** map, int level_width, int& objectPoints, int& points, double& got_object_time, double worldTime, bool& dead, bool& start);
