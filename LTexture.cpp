class LTexture
{
public:
	//initialize
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(char path[], SDL_Renderer* renderer);

	//Deallocates texture
	void free();

	//Renders texture at given point
	void render(SDL_Renderer* renderer, int x, int y,SDL_Rect* clip=NULL);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//wymiary
	int mWidth;
	int mHeight;
};

LTexture::LTexture()
{
	//wartoœci startowe
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{

	free();
}

bool LTexture::loadFromFile(char path[], SDL_Renderer* renderer)
{
	free();

	//wczytywana tekstura
	SDL_Texture* newTexture = NULL;

	//za³adowanie obrazu
	SDL_Surface* loadedSurface = loadSurface(path);
	if (loadedSurface != NULL)
	{
		//Color key image
		//SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Surface -> texture
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
		}
		else
		{
			//odczytanie wymiarów
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	SDL_DestroyTexture(newTexture);
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(SDL_Renderer* renderer, int x, int y,SDL_Rect* clip)
{
	//render na ekran pod danymi kordynatami
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	if (clip != NULL)
	{
		renderQuad.w=clip->w;
		renderQuad.h=clip->h;
	}
	//if(screen!=NULL)
	//SDL_UpdateTexture(mTexture, NULL, screen->pixels, screen->pitch);

	SDL_RenderCopy(renderer, mTexture, clip, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}