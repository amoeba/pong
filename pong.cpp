/*  pong.cpp
    Pong in C++ using SDL
    
    Written by Bryce Mecum [http://github.com/amoeba]
    petridish@gmail.com
*/


#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include <string>
#include <sstream>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

const int FRAMES_PER_SECOND = 60;

SDL_Event event;

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *paddle_surface = NULL;
SDL_Surface *fps_text = NULL;
SDL_Surface *ball_surface = NULL;

TTF_Font *font = NULL;
SDL_Color text_color = {0, 0, 0};

class Paddle
{
public:
  int x, y;
  int velocity;
  Paddle(int _x, int _y) { x = _x; y = _y; velocity = 0; }
};

class Ball
{
public:
  int x, y, velx, vely;
  Ball(int _x, int _y, int _velx, int _vely) { x = _x; y = _y; velx = _velx; vely = _vely;}
};

SDL_Surface *load_image (std::string filename)
{
  SDL_Surface* loaded_image = NULL;
  SDL_Surface* optimized_image = NULL;

  loaded_image = IMG_Load(filename.c_str());

  if(loaded_image != NULL )
  {
    optimized_image = SDL_DisplayFormat(loaded_image);
    SDL_FreeSurface(loaded_image);

    if(optimized_image != NULL )
    {
      Uint32 colorkey = SDL_MapRGB(optimized_image->format, 0xFF, 0xFF, 0xFF);
      SDL_SetColorKey(optimized_image, SDL_SRCCOLORKEY, colorkey); 
    }
  }

  return optimized_image;
}

void apply_surface (int x, int y, SDL_Surface *source, SDL_Surface *destination)
{
  SDL_Rect offset;
  offset.x = x;
  offset.y = y;

  SDL_BlitSurface(source, NULL, destination, &offset);
}

bool init()
{
  if(SDL_Init(SDL_INIT_EVERYTHING) == -1 )
  {
    return false;
  }

  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

  if(screen == NULL)
  {
    return false;
  }

  if(TTF_Init() == -1)
  {
    return false;
  }

  SDL_WM_SetCaption("Pong", NULL);

  return true;
}

bool load_files()
{
  background = load_image("background.png");
  paddle_surface = load_image("paddle.png");
  ball_surface = load_image("ball.png");

  font = TTF_OpenFont("DejaVuSans.ttf", 24);

  if(background == NULL ||
    paddle_surface == NULL ||
    font == NULL ||
    ball_surface == NULL
  )
  {
    return false;
  }


  return true;
}

void clean_up()
{
  SDL_FreeSurface(background);
  SDL_FreeSurface(ball_surface);
  SDL_FreeSurface(paddle_surface);
  SDL_FreeSurface(fps_text);

  TTF_CloseFont(font);

  TTF_Quit();
  SDL_Quit();
}




int main (int argc, char* args[])
{
  bool running = true;
  std::stringstream caption;
  float frames = 0.0f;
  float fps = 0.0f;
  float ticks = 0.0f;

  if(init() == false )
  {
    return 1;
  }

  if(load_files() == false )
  {
    return 1;
  }

  Paddle* paddle = new Paddle(paddle_surface->w, SCREEN_HEIGHT/2);
  Ball* ball = new Ball(SCREEN_WIDTH / 2 - ball_surface->w / 2, SCREEN_HEIGHT / 2 - ball_surface->h / 2, -1, 1);

  while(running)
  {
    frames++;

    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        //Quit the program
        running = false;
      }
      else if(event.type == SDL_KEYDOWN)
      {
      if(event.key.keysym.sym == SDLK_UP)
      {
        paddle->velocity = -1;
      }
      else if(event.key.keysym.sym == SDLK_DOWN)
      {
        paddle->velocity = 1;
      }
      }
      else if(event.type == SDL_KEYUP)
      {
      if(event.key.keysym.sym == SDLK_UP)
      {
        paddle->velocity = 0;
      }
      else if(event.key.keysym.sym == SDLK_DOWN)
      {
        paddle->velocity = 0;
      }
      }
    }

    std::stringstream caption;

    ticks = SDL_GetTicks();
    fps = (ticks != 0) ? (frames / (ticks/1000.0f)) : 0.0f;
    caption << "FPS: " << fps;

    fps_text = TTF_RenderText_Blended(font, caption.str().c_str(), text_color);

    apply_surface(0, 0, background, screen);


    // Update the Paddle

    if((paddle->y > 0 && paddle->velocity != 1) ||
      (paddle->y < SCREEN_HEIGHT - paddle_surface->h && paddle->velocity != -1))
    {
    paddle->y += 3 * paddle->velocity;
    }


    // Update the Ball

    if(ball->x <= 0 || ball->x + ball_surface->h >= SCREEN_WIDTH)
    {
    ball->velx *= -1;
    }

    if(ball->y <= 0 || ball->y + ball_surface->w >= SCREEN_HEIGHT)
    {
    ball->vely *= -1;
    }

    // To the right
    if(ball->x <= paddle->x + paddle_surface->w &&
      ball->y >= paddle->y &&
      ball->y + ball_surface->h <= paddle->y + paddle_surface->h)
    {
    ball->velx *= -1;
    }


    ball->x += 3 * ball->velx;
    ball->y += 3 * ball->vely;


    // Fill the screen up

    apply_surface(paddle->x, paddle->y, paddle_surface, screen);
    apply_surface(ball->x, ball->y, ball_surface, screen);
    apply_surface(0, 0, fps_text, screen);

    if(SDL_Flip(screen) == -1)
    {
      return 1;
    }
  }

  clean_up();

  return 0;
}
