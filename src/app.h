#ifndef APP_H__HEADER_GUARD__
#define APP_H__HEADER_GUARD__

#include <stdlib.h>  /* exit, EXIT_FAILURE, size_t, srand, rand */
#include <stdbool.h> /* bool, true, false */
#include <time.h>    /* time, time_t */
#include <math.h>    /* cos, sin, atan2 */
#include <stdio.h>   /* snprintf */

#include <SDL2/SDL.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define TITLE "AABB Collision"

#define SCREEN_W 800
#define SCREEN_H 450

#define FPS_CAP 60

#define FRICTION 0.96

#define MAX_PLR_SPEED   3
#define PLR_SCALE       30
#define PLR_ACCEL_SPEED 0.2

#define BOX_SCALE 45

#define MAX_PARTICLE_SPEED 10
#define PARTICLE_SCALE     20

#define SIZE_OF(p_arr) (sizeof(p_arr) / sizeof(p_arr[0]))

typedef struct {
	size_t w, h;
	float  x, y;
	float  vel_x, vel_y;
} entity_t;

SDL_Rect entity_get_SDL_Rect(entity_t *p_entity);

typedef struct {
	size_t w, h, life_timer, lifetime;
	float  x, y;
	float  vel_x, vel_y;
} particle_t;

void     particle_emit(particle_t *p_particle, float p_dir, float p_x, float p_y);
SDL_Rect particle_get_SDL_Rect(particle_t *p_particle);

typedef struct {
	SDL_Window   *window;
	SDL_Renderer *renderer;

	SDL_Event    event;
	const Uint8 *key_states;
	int          mouse_x, mouse_y;

	size_t fps, tick;

	float    plr_dir;
	entity_t plr;
	entity_t entities[16];
	size_t   entities_count;

	particle_t particles[128];

	bool quit;
} app_t;

void app_init(app_t *p_app);
void app_finish(app_t *p_app);
void app_run(app_t *p_app);

bool app_player_collides(app_t *p_app);
bool app_particle_collides(app_t *p_app, particle_t *p_particle);

void app_render(app_t *p_app);
void app_events(app_t *p_app);

#endif
