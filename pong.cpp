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
#include <vector>
#include <cmath>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

const int FRAMES_PER_SECOND = 30;

SDL_Event event;

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *paddle_surface = NULL;
SDL_Surface *fps_text = NULL;
SDL_Surface *ball_surface = NULL;
SDL_Surface *score_player_surface = NULL;
SDL_Surface *score_computer_surface = NULL;

TTF_Font *font = NULL;
SDL_Color text_color = {0, 0, 0};

int score_player;
int score_computer;

template <class T>
inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

class Paddle
{
public:
  int x, y, w, h, vy;
  
  Paddle(int, int, int, int); 
  void update();
} *paddle;

Paddle::Paddle(int _x, int _y, int _w, int _h)
{
  x = _x;
  y = _y;
  w = _w;
  h = _h;
  vy = 0;
}

void Paddle::update()
{
  if((this->y > 0 && this->vy != 1) ||
    (this->y < SCREEN_HEIGHT - paddle_surface->h && this->vy != -1))
  {
    this->y += 10 * this->vy;
  }
}


class Ball
{
public:
  int x, y, w, h, vx, vy;
  
  Ball(int, int, int, int, int, int);
  void update();
  
  void collide_with(Paddle*);
  
  int cx() { return (this->x + (this->w)/2); }
  int cy() { return (this->y + (this->h)/2); }
  int r() { return ((this->w)/2); }
} *ball;

Ball::Ball(int _x, int _y, int _w, int _h, int _vx, int _vy)
{
  this->x = _x;
  this->y = _y;
  this->w = _w;
  this->h = _h;
  this->vx = _vx;
  this->vx = _vy;
}

void Ball::update()
{
  if(this->x <= 0)
    {
      this->x = SCREEN_WIDTH / 2 - this->w;
      this->y = SCREEN_HEIGHT / 2 - this->h;
      this->vx = -1;
      this->vy = -1;
    }
    else
    {
      if(this->y <= 0 || this->y + ball_surface->w >= SCREEN_HEIGHT)
      {
        this->vy *= -1;
      }

      if(this->x + ball_surface->h >= SCREEN_WIDTH)
      {
        this->vx *= -1;
      }

      this->collide_with(paddle);

      this->x += 3 * this->vx;
      this->y += 3 * this->vy;
    }
}


void Ball::collide_with(Paddle *paddle)
{
  if(this->vx != -1)
  {
    return;
  }
  
  int px, py, pw, ph;
  int bx, by, bw, bh;

  px = paddle->x;
  py = paddle->y;
  pw = paddle->w;
  ph = paddle->h;
  
  bx = this->x;
  by = this->y;
  bw = this->w;
  bh = this->h;

  int r = bw/2;
  
  // Check right side collision
  if((by + r) >= py &&
    (by + r) <= (py + ph) &&
    bx <= (px + pw))
  {
    this->vx *= -1;
  }
  
  // Check top/bottom collision
  if((bx + r) >= px &&
    (bx + r) <= (px + pw) &&
    (by + bh) >= py &&
    by <= (py + ph))
  {
    this->vy *= -1;
  }
    
  
    
  
  return;
}

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
  SDL_FreeSurface(score_player_surface);
  SDL_FreeSurface(score_computer_surface);

  TTF_CloseFont(font);

  TTF_Quit();
  SDL_Quit();
}

int main (int argc, char* args[])
{
  bool running = true;

  float frames = 0.0f;
  float fps = 0.0f;
  
  float before_frame_time = 0.0f;
  float after_frame_time = 0.0f;
  float delta_time = 0.0f;
  float elapsed_time = 0.0f;
  float initial_time = (float)SDL_GetTicks();
  
  if(init() == false )
  {
    return 1;
  }

  if(load_files() == false )
  {
    return 1;
  }

  paddle = new Paddle(0, SCREEN_HEIGHT/2, paddle_surface->w, paddle_surface->h);
  ball = new Ball(SCREEN_WIDTH / 2 - ball_surface->w / 2, SCREEN_HEIGHT / 2 - ball_surface->h / 2, ball_surface->w, ball_surface->h, -1, 1);

  while(running)
  {
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
        paddle->vy = -1;
      }
      else if(event.key.keysym.sym == SDLK_DOWN)
      {
        paddle->vy = 1;
      }
      }
      else if(event.type == SDL_KEYUP)
      {
      if(event.key.keysym.sym == SDLK_UP)
      {
        paddle->vy = 0;
      }
      else if(event.key.keysym.sym == SDLK_DOWN)
      {
        paddle->vy = 0;
      }
      }
    }

    
    // Calculate FPS
    
    /*tnew = SDL_GetTicks();
    dt = tnew - told;
    told = tnew;
    
    
    
    //fps = (dt != 0) ? (1/(float)(dt/1000.0)) : 0.0f;
    fps = (tnew != 0) ? (frames/((tnew-tstart)/1000)) : 0;
    
    */
    
    before_frame_time = after_frame_time;
    after_frame_time = (float)SDL_GetTicks();
    delta_time = after_frame_time - before_frame_time;
    elapsed_time = SDL_GetTicks() - initial_time;
    fps = frames / (elapsed_time/1000);
    
    fps_text = TTF_RenderText_Blended(font, to_string(fps).c_str(), text_color);


    // Update screen objects
    
    paddle->update();
    //ball->update();
    
    // Update the score text
       
    score_player_surface = TTF_RenderText_Blended(font, to_string(score_player).c_str(), text_color);
    score_computer_surface = TTF_RenderText_Blended(font, to_string(score_computer).c_str(), text_color);

    // Fill the screen up

    apply_surface(0, 0, background, screen);
    apply_surface(paddle->x, paddle->y, paddle_surface, screen);
    apply_surface(ball->x, ball->y, ball_surface, screen);
    apply_surface(0, SCREEN_HEIGHT - fps_text->h, fps_text, screen);
    apply_surface(0, 0, score_player_surface, screen);
    apply_surface(SCREEN_WIDTH - score_computer_surface->w, 0, score_computer_surface, screen);


    SDL_FreeSurface(fps_text);
    SDL_FreeSurface(score_player_surface);
    SDL_FreeSurface(score_computer_surface);
    
    if(SDL_Flip(screen) == -1)
    {
      return 1;
    }

      
      
    frames++;
    
    if((SDL_GetTicks() - before_frame_time) < (1000.0 / FRAMES_PER_SECOND))
    {
      SDL_Delay((Uint32)((1000.0 / FRAMES_PER_SECOND) - (SDL_GetTicks() - before_frame_time)));
    }
  }

  clean_up();

  return 0;
}
