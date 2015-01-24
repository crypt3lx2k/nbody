#include <math.h>
#include <stdio.h>

#include <fontconfig/fontconfig.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "align_malloc.h"
#include "draw.h"

#define EXPAND_STR(x) STR(x)
#define STR(x) #x

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* window */
static SDL_Surface * screen;

static int width;
static int height;
static int fps;

static size_t frame;
static Uint32 draw_time;

/* font */
static TTF_Font * font;
static SDL_Color font_color = {255, 255, 255, 0};
#define FONT_TIMES_N 16
static Uint32 font_times[FONT_TIMES_N];
static Uint32 font_prev_draw;

static void draw_font_free (void) {
  TTF_CloseFont(font);
  TTF_Quit();
  FcFini();
}

static void draw_font_init (void) {
  FcChar8 * filename;

  FcResult res;
  FcPattern * pattern_mono;
  FcPattern * pattern_file;

  FcInit();
  TTF_Init();

  pattern_mono = FcNameParse((FcChar8 *) "mono");
  FcConfigSubstitute(NULL, pattern_mono, FcMatchPattern);
  FcDefaultSubstitute(pattern_mono);

  pattern_file = FcFontMatch(NULL, pattern_mono, &res);
  FcPatternGetString(pattern_file, FC_FILE, 0, &filename);

  font = TTF_OpenFont((char *) filename, 8);

  FcPatternDestroy(pattern_file);
  FcPatternDestroy(pattern_mono);
}

static void draw_font_reset (void) {
  font_prev_draw = 0;

  memset(font_times, 0, FONT_TIMES_N*sizeof(Uint32));
}

static void draw_font_fps (size_t n, value dt) {
  size_t i;
  double fps = 0.0;

  char buffer[256];
  SDL_Surface * temp;

  font_times[frame % FONT_TIMES_N] = draw_time - font_prev_draw;

  for (i = 0; i < FONT_TIMES_N; i++)
    fps += (double) 1000.0/font_times[i];

  fps /= FONT_TIMES_N;

  snprintf(buffer, 256, "fps: %f particles: %zu dt: %e", fps, n, dt);

  temp = TTF_RenderText_Blended(font, buffer, font_color);
  SDL_BlitSurface(temp, NULL, screen, NULL);
  SDL_FreeSurface(temp);

  font_prev_draw = draw_time;
}

/* camera */
static value camera[VECTOR_SIZE];

static int scale;
static value zoom;

static size_t focus;

enum {
  CAMERA_FREE  = 0,
  CAMERA_FOCUS = 1
} camera_mode;

static void draw_camera (value px, value py, SDL_Rect * rect) {
  value s = value_literal(0.5) * scale * zoom;
  value r[VECTOR_SIZE];

  r[0] = px - camera[0];
  r[1] = py - camera[1];

  r[0] *= s;
  r[1] *= s;

  rect->x = r[0];
  rect->y = r[1];

  /* center on screen */
  rect->x += width/2;
  rect->y += height/2;
}

/* star sprite */
static SDL_Surface * star;
static value * star_kinetics;
static Uint8 * star_alphas;
static int star_w;
static int star_h;

static void draw_sprite_free (void) {
  free(star_alphas);
  star_alphas = NULL;
}

static void draw_sprite_init (size_t n) {
  SDL_Surface * temp;

  star_alphas = malloc(n * sizeof(Uint8));
  star_kinetics = malloc(n * sizeof(value));

  if (star_alphas == NULL || star_kinetics == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  temp = SDL_LoadBMP(EXPAND_STR(COMPILE_DIR) "/" "../sprites/star.bmp");
  star = SDL_ConvertSurface(temp, screen->format, SDL_HWSURFACE | SDL_SRCALPHA);
  SDL_FreeSurface(temp);

  SDL_SetColorKey(star, SDL_SRCCOLORKEY,
		  SDL_MapRGB(screen->format, 0, 0, 0));

  star_w = star->w;
  star_h = star->h;
}

static void draw_sprite_reset (size_t n) {
  memset(star_alphas, 0, n*sizeof(Uint8));
  memset(star_kinetics, 0, n*sizeof(value));
}

static inline value draw_sprite_kinetic (value vx, value vy, value m) {
  return value_literal(0.5)*m*sqrtv(vx*vx + vy*vy);
}

static inline void draw_sprite_calculate_alphas (size_t n,
						 const value * vx, const value * vy,
						 const value * m) {
  size_t i;
  value max;

  max = value_literal(0.0);
  for (i = 0; i < n; i++) {
    value Ek;
    value x = vx[i];
    value y = vy[i];

    if (camera_mode == CAMERA_FOCUS) {
      x -= vx[focus % n];
      y -= vy[focus % n];
    }

    Ek = draw_sprite_kinetic(x, y, m[i]);

    if (Ek > max)
      max = Ek;

    star_kinetics[i] = Ek;
  }

  for (i = 0; i < n; i++) {
    star_alphas[i] = star_kinetics[i]/max*128;
  }
}

/* trail */
#define TRAIL_LENGTH (60*1)
typedef value trail[TRAIL_LENGTH];
static trail * trailx = NULL;
static trail * traily = NULL;
static Uint32 trail_color;
static int trail_active = 0;

static void draw_trail_free (void) {
  align_free(traily);
  align_free(trailx);

  traily = NULL;
  trailx = NULL;
}

static void draw_trail_init (size_t n) {
  trailx =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(trail), ALLOC_PADDING);
  traily =
    align_padded_malloc(ALIGN_BOUNDARY, n*sizeof(trail), ALLOC_PADDING);

  if (trailx == NULL || traily == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }

  trail_color = SDL_MapRGB(screen->format, 0x7f, 0x7f, 0x7f);
}

static void draw_trail_reset (size_t n) {
  memset(trailx, 0, n*sizeof(trail));
  memset(traily, 0, n*sizeof(trail));
}

static inline void draw_trail_record (size_t i, value px, value py) {
  trailx[i][frame % TRAIL_LENGTH] = px;
  traily[i][frame % TRAIL_LENGTH] = py;
}

static void draw_trail_replay (size_t i, size_t n) {
  size_t j;

  for (j = 0; j < MIN(frame, TRAIL_LENGTH); j++) {
    SDL_Rect rect;
    value t[VECTOR_SIZE];

    t[0] = trailx[i][j];
    t[1] = traily[i][j];

    if (camera_mode == CAMERA_FOCUS) {
      t[0] += camera[0] - trailx[focus % n][j];
      t[1] += camera[1] - traily[focus % n][j];
    }

    draw_camera(t[0], t[1], &rect);

    rect.w = 1;
    rect.h = 1;

    SDL_FillRect(screen, &rect, trail_color);
  }
}

void draw_free (void) {
  draw_trail_free();
  draw_sprite_free();

  draw_font_free();

  SDL_FreeSurface(star);
  SDL_Quit();
}

void draw_init (int w, int h, int f, size_t n) {
  const SDL_VideoInfo * info;

  SDL_Init(SDL_INIT_EVERYTHING);

  info = SDL_GetVideoInfo();

  width  = w ? w : info->current_w;
  height = h ? h : info->current_h;
  fps    = f ? f : 60;
  scale  = MIN(width, height);

  screen = SDL_SetVideoMode(width, height, 0,
			    SDL_HWSURFACE | SDL_NOFRAME | SDL_DOUBLEBUF);
  SDL_WM_SetCaption("nbody", NULL);

  SDL_SetAlpha(screen, SDL_SRCALPHA, 0);
  SDL_ShowCursor(SDL_DISABLE);

  draw_font_init();

  zoom = value_literal(0.5);

  draw_sprite_init(n);
  draw_trail_init(n);

  draw_reset(n);
}

static unsigned int draw_handle_keypress (unsigned int app_state,
					  value * dt,
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
    *dt *= value_literal(2.0);
    break;
  case SDLK_MINUS:
    *dt *= value_literal(0.5);
    break;
  case SDLK_b:
    *dt *= value_literal(-1.0);
    break;
  case SDLK_f:
    camera_mode ^= 1;
    break;
  case SDLK_w:
    /* fall-through */
  case SDLK_UP:
    if (camera_mode == CAMERA_FREE)
      camera[1] -= value_literal(1.0)/zoom;
    else
      focus += 1;
    break;
  case SDLK_s:
    /* fall-through */
  case SDLK_DOWN:
    if (camera_mode == CAMERA_FREE)
      camera[1] += value_literal(1.0)/zoom;
    else
      focus -= 1;
    break;
  case SDLK_a:
    /* fall-through */
  case SDLK_LEFT:
    if (camera_mode == CAMERA_FREE)
      camera[0] -= value_literal(1.0)/zoom;
    else
      focus -= 1;
    break;
  case SDLK_d:
    /* fall-through */
  case SDLK_RIGHT:
    if (camera_mode == CAMERA_FREE)
      camera[0] += value_literal(1.0)/zoom;
    else
      focus += 1;
    break;
  case SDLK_t:
    trail_active ^= 1;
    break;
  case SDLK_z:
    zoom *= value_literal(2.0);
    break;
  case SDLK_x:
    zoom *= value_literal(0.5);
    break;
  default:
    break;
  }

  return app_state;
}

unsigned int draw_input (unsigned int app_state, value * dt) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      app_state |= EXIT;
      break;
    case SDL_KEYDOWN:
      app_state = draw_handle_keypress(app_state, dt, &event.key);
      break;
    default:
      break;
    }
  }

  return app_state;
}

static inline void draw_particle_2d (value px, value py, Uint8 alpha) {
  SDL_Rect rect;

  draw_camera(px, py, &rect);

  rect.w = star_w; rect.x -= rect.w/2;
  rect.h = star_h; rect.y -= rect.h/2;

  SDL_SetAlpha(star, SDL_RLEACCEL | SDL_SRCALPHA, alpha);
  SDL_BlitSurface(star, NULL, screen, &rect);
}

void draw_particles (value dt, size_t n,
		     const value * px, const value * py,
		     const value * vx, const value * vy,
		     const value * m) {
  size_t i;

  if (SDL_GetTicks() < draw_time + 1000/fps)
    return;

  draw_time = SDL_GetTicks();

  draw_sprite_calculate_alphas(n, vx, vy, m);

  SDL_FillRect(screen, NULL, 0);

  if (camera_mode == CAMERA_FOCUS) {
    camera[0] = px[focus % n];
    camera[1] = py[focus % n];
  }

  for (i = 0; i < n; i++)
    draw_trail_record(i, px[i], py[i]);

  for (i = 0; i < n; i++) {
    draw_particle_2d(px[i], py[i], star_alphas[i]);

    if (trail_active)
      draw_trail_replay(i, n);
  }

  draw_font_fps(n, dt);
  SDL_Flip(screen);

  frame += 1;
}

void draw_reset (size_t n) {
  camera[0] = value_literal(0.0);
  camera[1] = value_literal(0.0);

  focus = 0;
  frame = 0;
  draw_time = 0;

  draw_font_reset();

  draw_sprite_reset(n);
  draw_trail_reset(n);
}
