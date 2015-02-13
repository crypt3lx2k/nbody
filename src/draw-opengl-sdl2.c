#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fontconfig/fontconfig.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>

#include "align_malloc.h"
#include "draw.h"

#define EXPAND_STR(x) STR(x)
#define STR(x) #x

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define DEBUG 1
#ifdef DEBUG
#define CHECK_GL() 				\
  do {						\
    GLenum e = glGetError();			\
    if (e != GL_NO_ERROR) {			\
      fprintf(stderr, "GL ERROR: %s:%d %d\n",	\
	      __FILE__, __LINE__, e);		\
      exit(EXIT_FAILURE);			\
    }						\
  } while (0)

#define CHECK_GLSL(s)				\
  do {						\
    GLint e;					\
    glGetShaderiv(s, GL_COMPILE_STATUS, &e);	\
						\
    if (e != GL_TRUE) {				\
      char buffer[512];				\
						\
      glGetShaderInfoLog(s, 512, NULL, buffer);	\
						\
      fprintf(stderr, "GL ERROR: %s:%d %s\n",	\
	      __FILE__, __LINE__, buffer);	\
      exit(EXIT_FAILURE);			\
    }						\
  } while (0)
#else
#define CHECK_GL()
#define CHECK_GLSL(s)
#endif

typedef double ticks;

static inline ticks get_ticks (void) {
  struct timespec now;

  (void) clock_gettime(CLOCK_MONOTONIC, &now);

  return 1e3*now.tv_sec + 1e-6*now.tv_nsec;
}

/* main window */
static SDL_Window * draw_window;
static SDL_GLContext draw_window_context;
static int draw_window_width;
static int draw_window_height;
static int draw_window_scale;
static int draw_window_fps;

static size_t draw_window_frame;
static ticks draw_window_time;

static void draw_window_free (void) {
  SDL_GL_DeleteContext(draw_window_context);
  SDL_DestroyWindow(draw_window);
}

static void draw_window_init (int w, int h, int f) {
  SDL_DisplayMode info;

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GetCurrentDisplayMode(0, &info);
  draw_window_width  = w ? w : info.w;
  draw_window_height = h ? h : info.h;
  draw_window_fps    = f ? f : info.refresh_rate;
  draw_window_fps    = draw_window_fps ? draw_window_fps : 60;
  draw_window_scale  = MIN(draw_window_width, draw_window_height);

  draw_window = SDL_CreateWindow("nbody",
				 0,
				 0,
				 draw_window_width, draw_window_height,
				 SDL_WINDOW_OPENGL |
				 SDL_WINDOW_BORDERLESS);
  SDL_GetWindowSize(draw_window,
		    &draw_window_width, &draw_window_height);

  draw_window_context = SDL_GL_CreateContext(draw_window);

  glewExperimental = GL_TRUE;
  glewInit();
}

static void draw_window_reset (void) {
  draw_window_time = 0;
  draw_window_frame = 0;
}

/* glsl shaders */
static GLuint draw_shader;

#define GLSL(x) "#version 150\n" #x

static const char draw_shader_vertex[] = GLSL (
  in float vertex_x;
  in float vertex_y;

  in float vertex_Ek;

  out float frag_Ek;

  uniform mat4 camera_mvp;

  void main () {
    gl_Position = camera_mvp*vec4(vertex_x, vertex_y, 0.0, 1.0);
    frag_Ek = vertex_Ek;
  }
);

static const char draw_shader_fragment[] = GLSL (
  in float frag_Ek;
  out vec4 frag_colour;

  uniform sampler2D star_tex;

  void main () {
    frag_colour = texture(star_tex, gl_PointCoord) * vec4(1.0, 1.0, 1.0, frag_Ek);
  }
);

static GLuint draw_shader_compile (GLenum type, const char * src) {
  GLuint s = glCreateShader(type);

  glShaderSource(s, 1, &src, NULL); CHECK_GL();
  glCompileShader(s); CHECK_GLSL(s);

  return s;
}

static void draw_shader_free (void) {
  glDeleteProgram(draw_shader); CHECK_GL();
}

static void draw_shader_init (void) {
  GLuint vs, fs;

  vs = draw_shader_compile(GL_VERTEX_SHADER, draw_shader_vertex);
  fs = draw_shader_compile(GL_FRAGMENT_SHADER, draw_shader_fragment);

  draw_shader = glCreateProgram(); CHECK_GL();
  glAttachShader(draw_shader, vs); CHECK_GL();
  glAttachShader(draw_shader, fs); CHECK_GL();

  glBindAttribLocation(draw_shader, 0, "vertex_x"); CHECK_GL();
  glBindAttribLocation(draw_shader, 1, "vertex_y"); CHECK_GL();
  glBindAttribLocation(draw_shader, 2, "vertex_Ek"); CHECK_GL();

  glBindFragDataLocation(draw_shader, 0, "frag_colour"); CHECK_GL();

  glLinkProgram(draw_shader); CHECK_GL();

  glDeleteShader(fs); CHECK_GL();
  glDeleteShader(vs); CHECK_GL();
}

/* font */
static TTF_Font * draw_font;
static SDL_Color draw_font_color = {255, 255, 255, 255};
#define DRAW_FONT_SAMPLES 32
static ticks draw_font_times[DRAW_FONT_SAMPLES];
static ticks draw_font_prev_time;

static GLuint draw_font_tex[1];
static GLuint draw_font_vbo[1];
static GLuint draw_font_vao[1];

static GLuint draw_font_shader;

static const char draw_font_shader_vertex[] = GLSL (
  in vec2 position;
  in vec2 texcoord;

  out vec2 texcoord_i;

  void main () {
    gl_Position = vec4(position, 0.0, 1.0);
    texcoord_i = texcoord;
  }
);

static const char draw_font_shader_fragment[] = GLSL (
  in vec2 texcoord_i;
  out vec4 frag_colour;

  uniform sampler2D text_tex;

  void main () {
    frag_colour = texture(text_tex, texcoord_i);
  }
);

static void draw_font_free (void) {
  glDeleteProgram(draw_font_shader); CHECK_GL();

  glDeleteBuffers(1, draw_font_vbo); CHECK_GL();
  glDeleteVertexArrays(1, draw_font_vao); CHECK_GL();

  glDeleteTextures(1, draw_font_tex); CHECK_GL();

  TTF_CloseFont(draw_font);
  TTF_Quit();
  FcFini();
}

static TTF_Font * draw_font_find (char * font) {
  FcChar8 * filename;

  FcResult res;
  FcPattern * pattern_font;
  FcPattern * pattern_file;

  TTF_Font * r;

  pattern_font = FcNameParse((FcChar8 *) font);
  FcConfigSubstitute(NULL, pattern_font, FcMatchPattern);
  FcDefaultSubstitute(pattern_font);

  pattern_file = FcFontMatch(NULL, pattern_font, &res);
  FcPatternGetString(pattern_file, FC_FILE, 0, &filename);

  r = TTF_OpenFont((char *) filename, 8);

  FcPatternDestroy(pattern_file);
  FcPatternDestroy(pattern_font);

  return r;
}

static void draw_font_init_glsl (void) {
  GLuint vs, fs;

  vs = draw_shader_compile(GL_VERTEX_SHADER, draw_font_shader_vertex);
  fs = draw_shader_compile(GL_FRAGMENT_SHADER, draw_font_shader_fragment);

  draw_font_shader = glCreateProgram(); CHECK_GL();
  glAttachShader(draw_font_shader, vs); CHECK_GL();
  glAttachShader(draw_font_shader, fs); CHECK_GL();

  glBindAttribLocation(draw_font_shader, 0, "position"); CHECK_GL();
  glBindAttribLocation(draw_font_shader, 1, "texcoord"); CHECK_GL();

  glBindFragDataLocation(draw_font_shader, 0, "frag_colour"); CHECK_GL();

  glLinkProgram(draw_font_shader); CHECK_GL();

  glDeleteShader(fs); CHECK_GL();
  glDeleteShader(vs); CHECK_GL();
}

static void draw_font_init_gl (void) {
  glGenTextures(1, draw_font_tex); CHECK_GL();

  glGenBuffers(1, draw_font_vbo); CHECK_GL();
  glGenVertexArrays(1, draw_font_vao); CHECK_GL();

  glBindVertexArray(draw_font_vao[0]); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, draw_font_vbo[0]); CHECK_GL();

  glUseProgram(draw_font_shader); CHECK_GL();

  glEnableVertexAttribArray(0); CHECK_GL();
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0); CHECK_GL();

  glEnableVertexAttribArray(1); CHECK_GL();
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (GLvoid *) (2*sizeof(GLfloat))); CHECK_GL();

  glUseProgram(0); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL();
  glBindVertexArray(0); CHECK_GL();
}

static void draw_font_init (void) {

  FcInit();
  TTF_Init();

  draw_font = draw_font_find("mono");

  draw_font_init_glsl();
  draw_font_init_gl();
}

static void draw_font_reset (void) {
  draw_font_prev_time = 0;

  memset(draw_font_times, 0, DRAW_FONT_SAMPLES*sizeof(ticks));
}

static void draw_font_calc_box (int width, int height) {
  GLfloat w = width;
  GLfloat h = height;

  w *= 2.0f/draw_window_width;
  h *= 2.0f/draw_window_height;

#define TOP_LX -1.0f
#define TOP_LY  1.0f

#define TOP_L  TOP_LX + 0, TOP_LY + 0
#define TOP_R  TOP_LX + w, TOP_LY + 0
#define BOT_L  TOP_LX + 0, TOP_LY - h
#define BOT_R  TOP_LX + w, TOP_LY - h

#define TOP_L_TC 0.0f, 0.0f
#define TOP_R_TC 1.0f, 0.0f
#define BOT_L_TC 0.0f, 1.0f
#define BOT_R_TC 1.0f, 1.0f

  GLfloat draw_font_verts[] = {
    TOP_L, TOP_L_TC,
    BOT_R, BOT_R_TC,
    BOT_L, BOT_L_TC,

    TOP_L, TOP_L_TC,
    BOT_R, BOT_R_TC,
    TOP_R, TOP_R_TC
  };

#undef TOP_LX
#undef TOP_LY

#undef TOP_L
#undef TOP_R
#undef BOT_L
#undef BOT_R

#undef TOP_L_TC
#undef TOP_R_TC
#undef BOT_L_TC
#undef BOT_R_TC

  glBindBuffer(GL_ARRAY_BUFFER, draw_font_vbo[0]); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, sizeof(draw_font_verts), draw_font_verts, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL();
}

static void draw_font_fps (size_t n, value dt) {
#define BUFFER_SIZE 512
  size_t i, m;
  double fps = 0.0;

  char buffer[BUFFER_SIZE];
  SDL_Surface * temp;

  GLint sampler;

  m = MIN(draw_window_frame, DRAW_FONT_SAMPLES);

  draw_font_times[draw_window_frame % DRAW_FONT_SAMPLES] =
    draw_window_time - draw_font_prev_time;

  for (i = 0; i < m; i++)
    fps += 1000.0/draw_font_times[i];

  fps /= m;

  snprintf(buffer, BUFFER_SIZE,
	   "particles: %zu dt: %e\n"
	   "fps: %.0f frame: %zu",
	   n, dt, fps, draw_window_frame);

  temp = TTF_RenderText_Blended_Wrapped (
	   draw_font,
	   buffer,
	   draw_font_color,
	   BUFFER_SIZE
  );

  glBindVertexArray(draw_font_vao[0]); CHECK_GL();
  glUseProgram(draw_font_shader); CHECK_GL();

  sampler = glGetUniformLocation(draw_shader, "text_tex"); CHECK_GL();
  glUniform1i(sampler, 0); CHECK_GL();

  glActiveTexture(GL_TEXTURE0); CHECK_GL();
  glBindTexture(GL_TEXTURE_2D, draw_font_tex[0]); CHECK_GL();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, temp->w, temp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels); CHECK_GL(); 

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  draw_font_calc_box(temp->w, temp->h);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glUseProgram(0); CHECK_GL();
  glBindVertexArray(0); CHECK_GL();

  SDL_FreeSurface(temp);

  draw_font_prev_time = draw_window_time;
#undef BUFFER_SIZE
}

/* camera */
static value draw_camera[2];
static value draw_camera_zoom;
static size_t draw_camera_focus;

static GLfloat draw_camera_mvp[4][4];
static GLfloat draw_camera_mt1[4][4];
static GLfloat draw_camera_mt2[4][4];

static enum {
  CAMERA_FREE  = 0,
  CAMERA_FOCUS = 1
} draw_camera_mode;

static enum {
  CAMERA_MOVE_STOP  = 0,
  CAMERA_MOVE_UP    = 1 << 0,
  CAMERA_MOVE_DOWN  = 1 << 1,
  CAMERA_MOVE_LEFT  = 1 << 2,
  CAMERA_MOVE_RIGHT = 1 << 3,
  CAMERA_ZOOM_IN    = 1 << 4,
  CAMERA_ZOOM_OUT   = 1 << 5,
} draw_camera_move;

static void draw_camera_free (void) {
}

static void draw_camera_init (void) {
  draw_camera_mode = CAMERA_FREE;
  draw_camera_move = CAMERA_MOVE_STOP;

  draw_camera_zoom = value_literal(0.5);
}

static void draw_camera_reset (void) {
  draw_camera[0] = value_literal(0.0);
  draw_camera[1] = value_literal(0.0);

  draw_camera_focus = 0;
}

static void draw_camera_update (size_t n,
				const value * px, const value * py) {
  switch (draw_camera_mode) {
  case CAMERA_FREE:
    if (draw_camera_move & CAMERA_MOVE_UP)
      draw_camera[1] -= value_literal(0.05)/draw_camera_zoom;

    if (draw_camera_move & CAMERA_MOVE_DOWN)
      draw_camera[1] += value_literal(0.05)/draw_camera_zoom;

    if (draw_camera_move & CAMERA_MOVE_LEFT)
      draw_camera[0] -= value_literal(0.05)/draw_camera_zoom;

    if (draw_camera_move & CAMERA_MOVE_RIGHT)
      draw_camera[0] += value_literal(0.05)/draw_camera_zoom;
    break;
  case CAMERA_FOCUS:
    draw_camera[0] = px[draw_camera_focus % n];
    draw_camera[1] = py[draw_camera_focus % n];
    break;
  }

  if (draw_camera_move & CAMERA_ZOOM_IN)
    draw_camera_zoom *= value_literal(1.05);

  if (draw_camera_move & CAMERA_ZOOM_OUT)
    draw_camera_zoom /= value_literal(1.05);
}

static inline void draw_camera_mm4 (GLfloat A[4][4],
				    GLfloat B[4][4],
				    GLfloat C[4][4]) {
  size_t i, j, k;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      GLfloat s = 0.0f;

      for (k = 0; k < 4; k++)
	s += A[i][k] * B[k][j];

      C[i][j] = s;
    }
  }
}

static void draw_camera_upload_mvp (void) {
  GLint m;

  GLfloat left   = -(GLfloat) draw_window_width/draw_window_scale;
  GLfloat right  =  (GLfloat) draw_window_width/draw_window_scale;
  GLfloat bottom = -(GLfloat) draw_window_height/draw_window_scale;
  GLfloat top    =  (GLfloat) draw_window_height/draw_window_scale;
  GLfloat near   =  1.0f;
  GLfloat far    = -1.0f;

  GLfloat ortho[4][4] = {
    { 2.0f/(right-left),       0.0f,                0.0f,      0.0f },
    {       0.0f,        2.0f/(top-bottom),         0.0f,      0.0f },
    {       0.0f,              0.0f,         -2.0f/(far-near), 0.0f },
    {       0.0f,              0.0f,                0.0f,      1.0f }
  };

  GLfloat trans[4][4] = {
    { 1.0f, 0.0f, 0.0f, -draw_camera[0] },
    { 0.0f, 1.0f, 0.0f, -draw_camera[1] },
    { 0.0f, 0.0f, 1.0f,       0.0f      },
    { 0.0f, 0.0f, 0.0f,       1.0f      }
  };

  GLfloat scale[4][4] = {
    { draw_camera_zoom,       0.0f,       0.0f, 0.0f },
    {       0.0f,       draw_camera_zoom, 0.0f, 0.0f },
    {       0.0f,             0.0f,       1.0f, 0.0f },
    {       0.0f,             0.0f,       0.0f, 1.0f }
  };

  GLfloat flipv[4][4] = {
    { 1.0f,  0.0f,  0.0f, 0.0f },
    { 0.0f, -1.0f,  0.0f, 0.0f },
    { 0.0f,  0.0f, -1.0f, 0.0f },
    { 0.0f,  0.0f,  0.0f, 1.0f }
  };

  draw_camera_mm4(scale, trans, draw_camera_mt1);
  draw_camera_mm4(ortho, flipv, draw_camera_mt2);
  draw_camera_mm4(draw_camera_mt2, draw_camera_mt1, draw_camera_mvp);

  m = glGetUniformLocation(draw_shader, "camera_mvp");
  glUniformMatrix4fv(m, 1, GL_TRUE, draw_camera_mvp[0]); CHECK_GL();
}

/* sprite */
static GLuint draw_sprite_tex[1];
static GLuint draw_sprite_vbo[3];
static GLuint draw_sprite_vao[1];

static value * draw_sprite_Eks;

static void draw_sprite_free (void) {
  align_free(draw_sprite_Eks);

  glDeleteBuffers(3, draw_sprite_vbo); CHECK_GL();
  glDeleteVertexArrays(1, draw_sprite_vao); CHECK_GL();

  glDeleteTextures(1, draw_sprite_tex); CHECK_GL();
  IMG_Quit();
}

static void draw_sprite_init_loadpng (void) {
  SDL_Surface * sprite;

  IMG_Init(IMG_INIT_PNG);
  glGenTextures(1, draw_sprite_tex); CHECK_GL();

  glActiveTexture(GL_TEXTURE0); CHECK_GL();
  glBindTexture(GL_TEXTURE_2D, draw_sprite_tex[0]); CHECK_GL();

  sprite = IMG_Load(EXPAND_STR(COMPILE_DIR) "/../sprites/star.png");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sprite->w, sprite->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, sprite->pixels); CHECK_GL();
  SDL_FreeSurface(sprite);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL();

  glBindTexture(GL_TEXTURE_2D, 0); CHECK_GL();
}

static void draw_sprite_init (size_t n) {
  draw_sprite_init_loadpng();

  glGenBuffers(3, draw_sprite_vbo); CHECK_GL();
  glGenVertexArrays(1, draw_sprite_vao); CHECK_GL();

  glBindVertexArray(draw_sprite_vao[0]); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[0]); CHECK_GL();
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), NULL, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[1]); CHECK_GL();
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), NULL, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[2]); CHECK_GL();
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), NULL, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL();

  glEnableVertexAttribArray(0); CHECK_GL();
  glEnableVertexAttribArray(1); CHECK_GL();
  glEnableVertexAttribArray(2); CHECK_GL();

  glBindVertexArray(0); CHECK_GL();

  draw_sprite_Eks = align_malloc(ALIGN_BOUNDARY, n*sizeof(value));

  if (draw_sprite_Eks == NULL) {
    perror(__func__);
    exit(EXIT_FAILURE);
  }
}

static void draw_sprite_load_calc (size_t n,
				   const value * vx, const value * vy,
				   const value * m) {
  size_t i;

  value max = 0.0;

  for (i = 0; i < n; i++) {
    value x = vx[i];
    value y = vy[i];
    value Ek;

    if (draw_camera_mode == CAMERA_FOCUS) {
      x -= vx[draw_camera_focus % n];
      y -= vy[draw_camera_focus % n];
    }

    Ek = value_literal(0.5)*m[i]*(x*x + y*y);

    if (Ek > max)
      max = Ek;

    draw_sprite_Eks[i] = Ek;
  }

  for (i = 0; i < n; i++) {
    draw_sprite_Eks[i] /= max;
    draw_sprite_Eks[i] /= value_literal(2.0);
    draw_sprite_Eks[i] +=
      value_literal(4.0)/value_literal(256.0);
  }
}

static void draw_sprite_load (size_t n,
			      const value * px, const value * py,
			      const value * vx, const value * vy,
			      const value * m) {
  GLfloat zoom = 21.0f*draw_camera_zoom;

  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[0]); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), px, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[1]); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), py, GL_STREAM_DRAW); CHECK_GL();

  draw_sprite_load_calc(n, vx, vy, m);
  glBindBuffer(GL_ARRAY_BUFFER, draw_sprite_vbo[2]); CHECK_GL();
  glBufferData(GL_ARRAY_BUFFER, n*sizeof(value), draw_sprite_Eks, GL_STREAM_DRAW); CHECK_GL();

  glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_GL();

  if (zoom < 9.0f)
    zoom = 9.0f;

  glPointSize(zoom); CHECK_GL();
}

static void draw_sprite_reset (size_t n) {
  memset(draw_sprite_Eks, 0, n*sizeof(value));
}

void draw_free (void) {
  draw_sprite_free();
  draw_camera_free();
  draw_font_free();
  draw_shader_free();
  draw_window_free();

  SDL_Quit();
}

void draw_init (int width, int height, int fps, size_t n) {
  SDL_Init(SDL_INIT_EVERYTHING);

  draw_window_init(width, height, fps);
  draw_shader_init();
  draw_font_init();
  draw_camera_init();
  draw_sprite_init(n);
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
    draw_camera_mode ^= 1;
    draw_camera_move = CAMERA_MOVE_STOP;
    break;
  case SDLK_w:
    /* fall-through */
  case SDLK_UP:
    if (draw_camera_mode == CAMERA_FREE)
      draw_camera_move |= CAMERA_MOVE_UP;
    else
      draw_camera_focus += 1;
    break;
  case SDLK_s:
    /* fall-through */
  case SDLK_DOWN:
    if (draw_camera_mode == CAMERA_FREE)
      draw_camera_move |= CAMERA_MOVE_DOWN;
    else
      draw_camera_focus -= 1;
    break;
  case SDLK_a:
    /* fall-through */
  case SDLK_LEFT:
    if (draw_camera_mode == CAMERA_FREE)
      draw_camera_move |= CAMERA_MOVE_LEFT;
    else
      draw_camera_focus -= 1;
    break;
  case SDLK_d:
    /* fall-through */
  case SDLK_RIGHT:
    if (draw_camera_mode == CAMERA_FREE)
      draw_camera_move |= CAMERA_MOVE_RIGHT;
    else
      draw_camera_focus += 1;
    break;
  case SDLK_z:
    draw_camera_move |= CAMERA_ZOOM_IN;
    break;
  case SDLK_x:
    draw_camera_move |= CAMERA_ZOOM_OUT;
    break;
  default:
    break;
  }

  return app_state;
}

static unsigned int draw_handle_keyrelease(unsigned int app_state,
					   SDL_KeyboardEvent * key) {
  switch (key->keysym.sym) {
  case SDLK_w:
    /* fall-through */
  case SDLK_UP:
    draw_camera_move &= ~CAMERA_MOVE_UP;
    break;
  case SDLK_s:
    /* fall-through */
  case SDLK_DOWN:
    draw_camera_move &= ~CAMERA_MOVE_DOWN;
    break;
  case SDLK_a:
    /* fall-through */
  case SDLK_LEFT:
    draw_camera_move &= ~CAMERA_MOVE_LEFT;
    break;
  case SDLK_d:
    /* fall-through */
  case SDLK_RIGHT:
    draw_camera_move &= ~CAMERA_MOVE_RIGHT;
    break;
  case SDLK_z:
    draw_camera_move &= ~CAMERA_ZOOM_IN;
    break;
  case SDLK_x:
    draw_camera_move &= ~CAMERA_ZOOM_OUT;
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
    case SDL_KEYUP:
      app_state = draw_handle_keyrelease(app_state, &event.key);
    default:
      break;
    }
  }

  return app_state;
}

void draw_particles (value dt, size_t n,
		     const value * px, const value * py,
		     const value * vx, const value * vy,
		     const value * m) {
  GLint sampler;

  draw_window_time = get_ticks();

  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT); CHECK_GL();

  glEnable(GL_BLEND); CHECK_GL();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECK_GL();

  glUseProgram(draw_shader); CHECK_GL();

  draw_camera_update(n, px, py);
  draw_camera_upload_mvp();

  glBindVertexArray(draw_sprite_vao[0]); CHECK_GL();
  draw_sprite_load(n, px, py, vx, vy, m);

  glEnable(GL_POINT_SPRITE); CHECK_GL();
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE); CHECK_GL();

  sampler = glGetUniformLocation(draw_shader, "star_tex"); CHECK_GL();
  glUniform1i(sampler, 0); CHECK_GL();

  glActiveTexture(GL_TEXTURE0); CHECK_GL();
  glBindTexture(GL_TEXTURE_2D, draw_sprite_tex[0]); CHECK_GL();

  glDrawArrays(GL_POINTS, 0, n); CHECK_GL();

  glBindTexture(GL_TEXTURE_2D, 0); CHECK_GL();

  glBindVertexArray(0); CHECK_GL();
  glUseProgram(0); CHECK_GL();
  glDisable(GL_POINT_SPRITE); CHECK_GL();

  draw_font_fps(n, dt);

  glDisable(GL_BLEND); CHECK_GL();

  SDL_GL_SwapWindow(draw_window);

  draw_window_frame += 1;
}

int draw_redraw (void) {
  return get_ticks() >= draw_window_time + 1000/draw_window_fps;
}

void draw_reset (size_t n) {
  draw_window_reset();
  draw_font_reset();
  draw_camera_reset();
  draw_sprite_reset(n);
}
