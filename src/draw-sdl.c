#include <math.h>
#include <stdbool.h>

#include <SDL/SDL.h>

#include "draw.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

/* window */
static SDL_Surface * screen;

static int width;
static int height;
static int fps;

static size_t frame;
static Uint32 draw_time;

/* star sprite */
static SDL_Surface * star;
static Uint8 * alphas;

static void draw_sprite_free (void) {
  free(alphas);
  alphas = NULL;
}

static void draw_sprite_init (size_t n) {
  if (alphas != NULL)
    return;

  alphas = malloc(n * sizeof(Uint8));

  if (alphas == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }
}

static inline value draw_sprite_kinetic (const particle * p) {
  vector v2 = p->velocity*p->velocity;
  return V(0.5)*p->mass*sqrt(v2[0] + v2[1]);
}

static inline void draw_sprite_calculate_alphas (const particle * particles, size_t n) {
  size_t i;
  value max;

  draw_sprite_init(n);

  max = V(0.0);
  for (i = 0; i < n; i++) {
    value Ek = draw_sprite_kinetic(&particles[i]);

    if (Ek > max)
      max = Ek;
  }

  for (i = 0; i < n; i++) {
    value Ek = draw_sprite_kinetic(&particles[i]);
    alphas[i] = Ek/max*255;
  }
}

/* camera */
static vector camera;

static int scale;
static value zoom;

static size_t focus;

enum {
  CAMERA_FREE,
  CAMERA_FOCUS
} camera_mode;

static void draw_camera (const vector position, SDL_Rect * rect) {
  vector r = (position - camera)*(scale*zoom*V(0.5));

  rect->x = r[0];
  rect->y = r[1];

  rect->x += width/2;
  rect->y += height/2;
}

/* trail */
#define TRAIL_LENGTH (1*60)
typedef vector trail_vector[TRAIL_LENGTH];
static trail_vector * trail = NULL;

static void draw_trail_free (void) {
  free(trail);
  trail = NULL;
}

static void draw_trail_init (size_t n) {
  if (trail != NULL)
    return;

  trail = malloc(n * sizeof(trail_vector));

  if (trail == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }
}

static inline void draw_trail_record (size_t i, const vector position) {
  trail[i][frame % TRAIL_LENGTH] = position;
}

static void draw_trail_replay (size_t i, size_t n) {
  size_t j;

  for (j = 0; j < min(frame, TRAIL_LENGTH); j++) {
    SDL_Rect rect;
    vector t = trail[i][j];

    if (camera_mode == CAMERA_FOCUS)
      t += camera - trail[focus % n][j];

    draw_camera(t, &rect);

    rect.w = 1;
    rect.h = 1;

    SDL_FillRect(screen, &rect,
		 SDL_MapRGB(screen->format, 0x7f, 0x7f, 0x7f));
  }
}

void draw_free (void) {
  SDL_FreeSurface(star);
  SDL_Quit();
}

void draw_init (int w, int h, int f) {
  const SDL_VideoInfo * info;
  SDL_Surface * temp;

  SDL_Init(SDL_INIT_EVERYTHING);

  info = SDL_GetVideoInfo();

  width  = w ? w : info->current_w;
  height = h ? h : info->current_h;
  fps    = f ? f : 60;
  scale = min(width, height);

  screen = SDL_SetVideoMode(width, height, 0,
			    SDL_HWSURFACE | SDL_NOFRAME);

  SDL_SetAlpha(screen, SDL_SRCALPHA, 0);
  SDL_ShowCursor(SDL_DISABLE);

  temp = SDL_LoadBMP("../sprites/star.bmp");
  star = SDL_ConvertSurface(temp, screen->format, SDL_HWSURFACE | SDL_SRCALPHA);
  SDL_FreeSurface(temp);

  zoom = V(0.5);

  draw_reset();
}

static unsigned int draw_handle_keypress (unsigned int app_state,
					  SDL_KeyboardEvent * key) {
  switch (key->keysym.sym) {
  case SDLK_ESCAPE:
    /* fall-through */
  case SDLK_q:
    app_state |= EXIT;
    break;
  case SDLK_r:
    app_state |= RESET;
    break;
  case SDLK_PLUS:
    app_state |= TIME_DELTA_INCREASE;
    break;
  case SDLK_MINUS:
    app_state |= TIME_DELTA_DECREASE;
    break;
  case SDLK_f:
    if (camera_mode == CAMERA_FREE)
      camera_mode = CAMERA_FOCUS;
    else
      camera_mode = CAMERA_FREE;
    break;
  case SDLK_w:
    /* fall-through */
  case SDLK_UP:
    if (camera_mode == CAMERA_FREE)
      camera[1] -= V(0.5);
    else
      focus += 1;
    break;
  case SDLK_s:
    /* fall-through */
  case SDLK_DOWN:
    if (camera_mode == CAMERA_FREE)
      camera[1] += V(0.5);
    else
      focus -= 1;
    break;
  case SDLK_a:
    /* fall-through */
  case SDLK_LEFT:
    if (camera_mode == CAMERA_FREE)
      camera[0] -= V(0.5);
    else
      focus -= 1;
    break;
  case SDLK_d:
    /* fall-through */
  case SDLK_RIGHT:
    if (camera_mode == CAMERA_FREE)
      camera[0] += V(0.5);
    else
      focus += 1;
    break;
  case SDLK_z:
    zoom += V(0.25);
    break;
  case SDLK_x:
    zoom -= V(0.25);
    break;
  default:
    break;
  }

  return app_state;
}

unsigned int draw_input (unsigned int app_state) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      app_state |= EXIT;
      break;
    case SDL_KEYDOWN:
      app_state = draw_handle_keypress(app_state, &event.key);
      break;
    default:
      break;
    }
  }

  return app_state;
}

static inline void draw_particle_2d (const particle * particles, size_t i) {
  SDL_Rect rect;

  draw_camera(particles[i].position, &rect);

  rect.w = 5; rect.x -= rect.w/2;
  rect.h = 5; rect.y -= rect.h/2;

  SDL_SetAlpha(star, SDL_RLEACCEL | SDL_SRCALPHA, alphas[i]);
  SDL_BlitSurface(star, NULL, screen, &rect);
}

void draw_particles (const particle * particles, size_t n) {
  size_t i;

  if (SDL_GetTicks() < draw_time + 1000/fps)
    return;

  draw_sprite_calculate_alphas(particles, n);
  draw_trail_init(n);

  SDL_FillRect(screen, NULL, 0);

  if (camera_mode == CAMERA_FOCUS)
    camera = particles[focus % n].position;

  for (i = 0; i < n; i++)
    draw_trail_record(i, particles[i].position);

  for (i = 0; i < n; i++) {
    draw_particle_2d(particles, i);
    draw_trail_replay(i, n);
  }

  SDL_Flip(screen);

  draw_time = SDL_GetTicks();
  frame += 1;
}

void draw_reset (void) {
  camera[0] = V(0.0);
  camera[1] = V(0.0);

  focus = 0;
  frame = 0;
  draw_time = 0;

  draw_sprite_free();
  draw_trail_free();
}
