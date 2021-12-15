#include <stdio.h> /* printf and fprintf */
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* Sets constants */
#define DELAY 17

int WIDTH = 800;
int HEIGHT = 600;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event e;

void render_screen();
void draw_circle(SDL_Point center, int radius, SDL_Color color);
void draw_points(int xpos, int points);
void ball_collisions();

int gameState = 1;

typedef struct {
	SDL_Rect pos;
	int velocity;
}Paddle;
typedef struct{
	int x;
	int y;
}Vector2;

Paddle player;
Paddle enemy;
SDL_Rect ball;
Vector2 ballVel;
int player_points = 0;
int enemy_points = 0;
const int maxYVel = 10;


void clear_screen(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(renderer);
}

int main (int argc, char **argv)
{
	player = (Paddle){{30, HEIGHT / 2 - 25, 15, 50}, 0};
	enemy = (Paddle){{WIDTH - 30, HEIGHT / 2 - 25, 15, 50}, 0};
	ball = (SDL_Rect){WIDTH / 2, HEIGHT / 2, 7, 7};
	ballVel = (Vector2){10, 5};
	/* Initialises data */
	bool quit = false;
	/*
	* Initialises the SDL video subsystem (as well as the events subsystem).
	* Returns 0 on success or a negative error code on failure using SDL_GetError().
	*/
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
		return 1;
	}
	/* Creates a SDL window */
	window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	/* Checks if window has been created; if not, exits program */
	if (window == NULL) {
		fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
		return 1;
	}
	
	while(!quit){
		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_KEYDOWN){
				if(e.key.keysym.sym == SDLK_ESCAPE){
					quit = true;
				}
			}
			if(e.type == SDL_QUIT){
				quit = true;
			}
			if(e.type == SDL_WINDOWEVENT){
				if(e.window.event == SDL_WINDOWEVENT_RESIZED){
					SDL_GetWindowSize(window, &WIDTH, &HEIGHT);
					enemy.pos.x = WIDTH - 30;
				}
			}
		}
		/* Pauses all SDL subsystems for a variable amount of milliseconds */


		render_screen();
		SDL_Delay(DELAY);
	}

	/* Frees memory */
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	/* Shuts down all SDL subsystems */
	SDL_Quit(); 

	return 0;
}

void render_screen(){
	clear_screen();


		
	if(gameState == 1){//Gameplay state
		//Player movement
		player.velocity = 0;	
		int newPaddlePos = 0;
		SDL_GetMouseState(NULL, &newPaddlePos);
		player.velocity = player.pos.y + (player.pos.h / 2) - newPaddlePos;
		player.pos.y = newPaddlePos;
		player.pos.y -= player.pos.h / 2;
		
		//Enemy movement
		enemy.velocity = (ball.y - enemy.pos.y < 6) ? ball.y - enemy.pos.y : 6;
		enemy.pos.y += enemy.velocity;
	
		//Ball movement
		ball.y += ballVel.y;
		ball.x += ballVel.x;
		ball_collisions();

		//Rendering
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0xff);
		SDL_RenderFillRect(renderer, &player.pos);//Player paddle
		SDL_RenderFillRect(renderer, &enemy.pos);//Enemy paddle
		draw_points(10, player_points);//Player points
		draw_points(WIDTH - (enemy_points * 16), enemy_points);//Enemy points
		draw_circle((SDL_Point){ball.x, ball.y}, ball.w, (SDL_Color){255, 255, 255, 255});//Ball
		//Center dotted line
		SDL_Rect dotLine = {WIDTH / 2 - 6, 0, 8, 8};
		for(int i = 0; i < HEIGHT / dotLine.w; i++){
			if(i % 2 == 0){
				dotLine.y = i * dotLine.w;
				SDL_RenderFillRect(renderer, &dotLine);
			}
		}
	}
	

	SDL_RenderPresent(renderer);
}

void ball_collisions(){
	SDL_Rect ballRect = {ball.x - ball.w, ball.y - ball.w, ball.w * 2, ball.w * 2};
	if(ballVel.y < 0){//Going up
		if(ball.y - ball.w < 0){//Top collision
			ballVel.y = -ballVel.y;
		}
	}else{//Going down
		if(ball.y + ball.w > HEIGHT){//Bottom collision
			ballVel.y = -ballVel.y;
		}
	}
	if(ballVel.x < 0){//Going left
		if(ball.x - ball.w < 0){//Left collision / enemy point
			ballVel.x = -ballVel.x;
			enemy_points++;
		}
		if(SDL_HasIntersection(&player.pos, &ballRect) && ballVel.x < 0){//Player hits ball
			ballVel.y = (abs(-player.velocity + 5) < maxYVel) ? -player.velocity : (-player.velocity < 0 ? -maxYVel : maxYVel);
			ballVel.x = -ballVel.x;
			printf("hit!\n");
		}
	}else{//Going right
		if(ball.x + ball.w > WIDTH){//Right collision / player point
			ballVel.x = -ballVel.x;
			player_points++;
		}
		if(SDL_HasIntersection(&enemy.pos, &ballRect) && ballVel.x > 0){//Enemy hits ball
			ballVel.x = -ballVel.x;
		}
	}
}

void draw_points(int xpos, int points){
	SDL_Rect point_rect = {xpos, 10, 8, 8};
	for(int i = 0; i < points; i++){
		point_rect.x = xpos + i * (8 + 8);
		SDL_RenderFillRect(renderer, &point_rect);
	}
}

void draw_circle(SDL_Point center, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, center.x + dx, center.y + dy);
            }
        }
    }
}