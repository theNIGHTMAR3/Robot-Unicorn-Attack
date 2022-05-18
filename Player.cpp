class Player_class
{
public:
	//rozmiar gracza
	static const int UNI_WIDTH = 90;
	static const int UNI_HEIGHT = 90;

	//maksymalna predkoœæ
	static const int UNI_VEL = 10;

	//Initializes the variables
    Player_class();

	//Takes key presses and adjusts the player's velocity
	void handleEvent(SDL_Event& event);

	//Moves the player
	void move();

	//Shows the player on the screen
	void player_render(SDL_Renderer* renderer, LTexture player_tex);

private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;

	//The velocity of the dot
	int mVelX, mVelY;
};

Player_class::Player_class()
{
    //wartoœci startowe
    mPosX = SCREEN_WIDTH/10;
    mPosY = SCREEN_HEIGHT/2;

    mVelX = 0;
    mVelY = 0;
}

void Player_class::handleEvent(SDL_Event& event)
{
    //wcisniecie klawisza
    if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
    {
        //zmiana predkosci
        switch (event.key.keysym.sym)
        {
        case SDLK_UP: mVelY -= UNI_VEL; break;
        case SDLK_DOWN: mVelY += UNI_VEL; break;
        case SDLK_LEFT: mVelX -= UNI_VEL; break;
        case SDLK_RIGHT: mVelX += UNI_VEL; break;
        }
    }
    //puszczenie klawisza
    else if (event.type == SDL_KEYUP && event.key.repeat == 0)
    {
        //zmiana predkosci
        switch (event.key.keysym.sym)
        {
        case SDLK_UP: mVelY += UNI_VEL; break;
        case SDLK_DOWN: mVelY -= UNI_VEL; break;
        case SDLK_LEFT: mVelX += UNI_VEL; break;
        case SDLK_RIGHT: mVelX -= UNI_VEL; break;
        }
    }
}

void Player_class::move()
{

    //ruch
    mPosX += mVelX;
    mPosY += mVelY;


    //korekcja
    if ((mPosX < 0) || (mPosX + UNI_WIDTH > SCREEN_WIDTH))
    {
        //Move back
        mPosX -= mVelX;
    }

    if ((mPosY < 0) || (mPosY + UNI_HEIGHT > SCREEN_HEIGHT))
    {
        //Move back
        mPosY -= mVelY;
    }
}

void Player_class::player_render(SDL_Renderer* renderer,LTexture player_tex)
{
    //render gracza
    player_tex.render(renderer,mPosX, mPosY,NULL);
}