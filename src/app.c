#include "app.h"

SDL_Rect entity_get_SDL_Rect(entity_t *p_entity) {
	return (SDL_Rect){
		.x = p_entity->x, .y = p_entity->y,
		.w = p_entity->w, .h = p_entity->h
	};
}

void particle_emit(particle_t *p_particle, float p_dir, float p_x, float p_y) {
	p_particle->x = p_x;
	p_particle->y = p_y;
	p_particle->w = PARTICLE_SCALE;
	p_particle->h = PARTICLE_SCALE;

	p_particle->lifetime   = 50 + rand() % 20;
	p_particle->life_timer = p_particle->lifetime;

	p_dir += (float)(50 - rand() % 100) / 100;

	p_particle->vel_x = cos(p_dir) * MAX_PARTICLE_SPEED;
	p_particle->vel_y = sin(p_dir) * MAX_PARTICLE_SPEED;
}

SDL_Rect particle_get_SDL_Rect(particle_t *p_particle) {
	return (SDL_Rect){
		.x = p_particle->x - p_particle->w / 2, .y = p_particle->y - p_particle->h / 2,
		.w = p_particle->w, .h = p_particle->h
	};
}

void app_init(app_t *p_app) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	p_app->window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                                 SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
	if (p_app->window == NULL) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	p_app->renderer = SDL_CreateRenderer(p_app->window, -1, SDL_RENDERER_ACCELERATED);
	if (p_app->renderer == NULL) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (SDL_RenderSetLogicalSize(p_app->renderer, SCREEN_W, SCREEN_H) != 0) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	if (SDL_SetRenderDrawBlendMode(p_app->renderer, SDL_BLENDMODE_BLEND) != 0) {
		SDL_Log("%s", SDL_GetError());

		exit(EXIT_FAILURE);
	}

	p_app->plr.w = PLR_SCALE;
	p_app->plr.h = PLR_SCALE;
	p_app->plr.x = SCREEN_W / 2 - p_app->plr.w / 2;
	p_app->plr.y = SCREEN_H / 2 - p_app->plr.h / 2;

	time_t random = rand();
	srand(time(&random));

	p_app->entities_count = 8;
	for (size_t i = 0; i < p_app->entities_count; ++ i) {
		p_app->entities[i].w = BOX_SCALE;
		p_app->entities[i].h = BOX_SCALE;

		int rand_pos = rand() % (SCREEN_W / 2 - BOX_SCALE * 2);
		rand_pos     = rand() % 2 == 1? rand_pos : SCREEN_W - rand_pos - BOX_SCALE;

		p_app->entities[i].x = rand_pos;
		p_app->entities[i].y = rand() % (SCREEN_H - BOX_SCALE);
	}

	p_app->key_states = SDL_GetKeyboardState(NULL);
}

void app_finish(app_t *p_app) {
	SDL_DestroyRenderer(p_app->renderer);
	SDL_DestroyWindow(p_app->window);

	SDL_Quit();
}

void app_run(app_t *p_app) {
	size_t fps_timer = 0;

	do {
		size_t now   = SDL_GetTicks();
		size_t delta = now - fps_timer;

		p_app->fps = 1000 / delta;
		fps_timer   = now;

		char title[32] = {0};
		snprintf(title, 32, TITLE" FPS: %li", p_app->fps);

		SDL_SetWindowTitle(p_app->window, title);

		app_render(p_app);
		app_events(p_app);

		++ p_app->tick;

		SDL_Delay(1000 / FPS_CAP);
	} while (!p_app->quit);
}

bool app_player_collides(app_t *p_app) {
	if (p_app->plr.x + p_app->plr.w > SCREEN_W || p_app->plr.y + p_app->plr.h > SCREEN_H)
		return true;
	else if (p_app->plr.x < 0 || p_app->plr.y < 0)
		return true;

	for (size_t i = 0; i < p_app->entities_count; ++ i) {
		SDL_Rect plr    = entity_get_SDL_Rect(&p_app->plr);
		SDL_Rect entity = entity_get_SDL_Rect(&p_app->entities[i]);

		if (SDL_HasIntersection(&plr, &entity))
			return true;
	}

	return false;
}

bool app_particle_collides(app_t *p_app, particle_t *p_particle) {
	for (size_t i = 0; i < p_app->entities_count; ++ i) {
		SDL_Rect entity   = entity_get_SDL_Rect(&p_app->entities[i]);
		SDL_Rect particle = particle_get_SDL_Rect(p_particle);

		if (SDL_HasIntersection(&particle, &entity))
			return true;
	}

	return false;
}

void app_render(app_t *p_app) {
	SDL_SetRenderDrawColor(p_app->renderer, 13, 43, 69, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(p_app->renderer);

	SDL_Rect rect;
	for (size_t i = 0; i < SIZE_OF(p_app->particles); ++ i) {
		if (p_app->particles[i].life_timer == 0)
			continue;

		float opacity = (float)p_app->particles[i].life_timer / p_app->particles[i].lifetime * 200;

		if (i % 2 == 1)
			SDL_SetRenderDrawColor(p_app->renderer, 103, 78, 104, opacity);
		else
			SDL_SetRenderDrawColor(p_app->renderer, 84, 78, 104, opacity);

		rect = particle_get_SDL_Rect(&p_app->particles[i]);

		SDL_RenderFillRect(p_app->renderer, &rect);

		float prev = p_app->particles[i].x;
		p_app->particles[i].x += p_app->particles[i].vel_x;

		if (app_particle_collides(p_app, &p_app->particles[i]))
			p_app->particles[i].x = prev;


		prev = p_app->particles[i].y;
		p_app->particles[i].y += p_app->particles[i].vel_y;

		if (app_particle_collides(p_app, &p_app->particles[i]))
			p_app->particles[i].y = prev;

		p_app->particles[i].vel_x *= FRICTION;
		p_app->particles[i].vel_y *= FRICTION;

		-- p_app->particles[i].life_timer;
	}

	SDL_SetRenderDrawColor(p_app->renderer, 208, 129, 89, SDL_ALPHA_OPAQUE);

	rect = entity_get_SDL_Rect(&p_app->plr);
	SDL_RenderFillRect(p_app->renderer, &rect);

	SDL_SetRenderDrawColor(p_app->renderer, 141, 105, 122, SDL_ALPHA_OPAQUE);
	for (size_t i = 0; i < p_app->entities_count; ++ i) {
		rect = entity_get_SDL_Rect(&p_app->entities[i]);
		SDL_RenderFillRect(p_app->renderer, &rect);
	}

	SDL_RenderPresent(p_app->renderer);
}

void app_events(app_t *p_app) {
	while (SDL_PollEvent(&p_app->event)) {
		switch (p_app->event.type) {
		case SDL_QUIT: p_app->quit = true; break;

		case SDL_KEYDOWN:
			switch (p_app->event.key.keysym.sym) {
			case SDLK_ESCAPE: p_app->quit = true; break;

			default: break;
			}

			break;

		case SDL_MOUSEMOTION: SDL_GetMouseState(&p_app->mouse_x, &p_app->mouse_y); break;

		default: break;
		}
	}

	p_app->plr_dir = atan2(p_app->mouse_y - p_app->plr.y - p_app->plr.h / 2,
	                       p_app->mouse_x - p_app->plr.x - p_app->plr.w / 2);

	if (p_app->key_states[SDL_SCANCODE_E] && p_app->tick % 2 == 0) {
		for (size_t i = 0; i < SIZE_OF(p_app->particles); ++ i) {
			if (p_app->particles[i].life_timer == 0) {
				particle_emit(&p_app->particles[i], p_app->plr_dir,
				              p_app->plr.x + p_app->plr.w / 2,
				              p_app->plr.y + p_app->plr.h / 2);

				break;
			}
		}
	}

	if (p_app->key_states[SDL_SCANCODE_W]) {
		p_app->plr.vel_x += cos(p_app->plr_dir) * PLR_ACCEL_SPEED;
		p_app->plr.vel_y += sin(p_app->plr_dir) * PLR_ACCEL_SPEED;
	}

	if (p_app->plr.vel_x != 0) {
		if (p_app->plr.vel_x > MAX_PLR_SPEED)
			p_app->plr.vel_x = MAX_PLR_SPEED;
		else if (p_app->plr.vel_x < -MAX_PLR_SPEED)
			p_app->plr.vel_x = -MAX_PLR_SPEED;

		float prev = p_app->plr.x;
		p_app->plr.x += p_app->plr.vel_x;

		if (app_player_collides(p_app)) {
			p_app->plr.x     = prev;
			p_app->plr.vel_x = 0;
		}

		p_app->plr.vel_x *= FRICTION;
	}

	if (p_app->plr.vel_y != 0) {
		if (p_app->plr.vel_y > MAX_PLR_SPEED)
			p_app->plr.vel_y = MAX_PLR_SPEED;
		else if (p_app->plr.vel_y < -MAX_PLR_SPEED)
			p_app->plr.vel_y = -MAX_PLR_SPEED;

		float prev = p_app->plr.y;
		p_app->plr.y += p_app->plr.vel_y;

		if (app_player_collides(p_app)) {
			p_app->plr.y     = prev;
			p_app->plr.vel_y = 0;
		}

		p_app->plr.vel_y *= FRICTION;
	}
}
