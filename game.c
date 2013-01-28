#include <math.h>
#ifndef M_PI
#define M_PI   3.14159265358979323846 /* pi */
#define M_PI_2 1.57079632679489661923 /* pi/2 */
#endif
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#ifdef DEBUG
#include <SDL/SDL_ttf.h>
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>



/*****************************************************************************/
/*** DECLARATIONS ***/

/* Functions */
int init();
int quit();
int init_video();
int quit_video();
int init_audio();
int quit_audio();

/* Structs */
typedef struct {
  float position[3];
  float velocity[3];
  float rotation;
  float orientation[6];
} Player;

#define ENTITY_FINISH 1
#define ENTITY_GROWLER 2
typedef struct {
  int type;
  float position[3];
} Entity;


/*****************************************************************************/
/*** DEFINITIONS ***/

/* Variables */
#ifdef DEBUG
TTF_Font *font;
#endif

#define NUM_AUDIO (8)
#define AUDIO_HEARTBEAT 0
#define AUDIO_BELL 1
#define AUDIO_GROWL 2

SDL_Surface *screen = NULL;
ALCdevice *sound_dev = NULL;
ALCcontext *sound_ctx = NULL;
ALuint sound_bufs[NUM_AUDIO];
ALuint sound_srcs[NUM_AUDIO];

Player player = {{0,0,0}, {0,0,0}, M_PI_2, {0,0,-1, 0,1,0}};
Entity finish = {ENTITY_FINISH, {0,0,-100}};
Entity growler = {ENTITY_GROWLER, {0,0,100}};


/*****************************************************************************/
/*** GAME LOOP ***/
void game_loop() {
  Uint32 pframe_time = SDL_GetTicks();
  Uint32 frame_time;
  int isRotate = 0, isMovement = 0;
  while(1) {
    frame_time = SDL_GetTicks();
    int frame_diff = frame_time - pframe_time;
    if (SDL_GetTicks() > 10000)
      return;
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      Uint8 keymask = event.key.keysym.sym;
      if (event.type == SDL_KEYDOWN) {
	if (keymask == SDLK_UP    || keymask == SDLK_w) isMovement = 1;
	if (keymask == SDLK_DOWN  || keymask == SDLK_s) isMovement = -1;
	if (keymask == SDLK_LEFT  || keymask == SDLK_a) isRotate = 1;
	if (keymask == SDLK_RIGHT || keymask == SDLK_d) isRotate = -1;
	if (keymask == SDLK_ESCAPE)
	  return;
      }
      if (event.type == SDL_KEYUP) {
	if (keymask == SDLK_UP    || keymask == SDLK_w) isMovement = 0;
	if (keymask == SDLK_DOWN  || keymask == SDLK_s) isMovement = 0;
	if (keymask == SDLK_LEFT  || keymask == SDLK_a) isRotate = 0;
	if (keymask == SDLK_RIGHT || keymask == SDLK_d) isRotate = 0;
      }
      if (event.type == SDL_QUIT)
	return;
    }
    if (isRotate) {
      player.rotation += frame_diff*0.002*isRotate;
      player.orientation[0] = cos(player.rotation);
      player.orientation[2] = sin(player.rotation);
      alListenerfv(AL_ORIENTATION, player.orientation);
    }
    if (isMovement) {
      player.velocity[0] = player.orientation[0]*frame_diff*0.002*isMovement;
      player.velocity[2] = player.orientation[2]*frame_diff*0.002*isMovement;
      player.position[0] += player.velocity[0];
      player.position[2] += player.velocity[2];
      alListenerfv(AL_POSITION, player.position);
    } else {
      player.velocity[0] = 0.0f;
      player.velocity[2] = 0.0f;
    }
    alListenerfv(AL_VELOCITY, player.velocity);
#ifdef DEBUG
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));
    char text[80];
    sprintf(text, "Position: %f,%f,%f", player.position[0],
	    player.position[1], player.position[2]);
    SDL_Rect pos = {0,0,0,0};
    SDL_Surface *texts = TTF_RenderText_Solid(font, text, 
					     (SDL_Color){255,255,255});
    SDL_BlitSurface(texts, NULL, screen, &pos);
    sprintf(text, "Velocity: %f,%f,%f", player.velocity[0],
	    player.velocity[1], player.velocity[2]);
    texts = TTF_RenderText_Solid(font, text, (SDL_Color){255,255,255});
    pos = (SDL_Rect){0,64,0,0};
    SDL_BlitSurface(texts, NULL, screen, &pos);
    sprintf(text, "Rotation: %f", player.rotation);
    texts = TTF_RenderText_Solid(font, text, (SDL_Color){255,255,255});
    pos = (SDL_Rect){0,128,0,0};
    SDL_BlitSurface(texts, NULL, screen, &pos);
    sprintf(text, "Orientation: %f,%f,%f", player.orientation[0],
	    player.orientation[1], player.orientation[2]);
    texts = TTF_RenderText_Solid(font, text, (SDL_Color){255,255,255});
    pos = (SDL_Rect){0,192,0,0};
    SDL_BlitSurface(texts, NULL, screen, &pos);
    SDL_Flip(screen);
#endif
    pframe_time = frame_time;
  }
}


/*****************************************************************************/
/*** MAIN ***/
int main(int argc, char *argv[])
{
  /* INIT */
  int error = init();
  if (error != 0)
    return 1;
  /* GAME */
  game_loop();
  /* QUIT */
  quit();
}


/*****************************************************************************/
/*** SETUP CODE ***/

int init() {
  int error = 0;
  /* INIT */
  error = SDL_Init(SDL_INIT_VIDEO);
  if (error == -1) { fprintf(stderr, "UNABLE TO INITIALIZE CONTROL\n");
    return quit();}
#ifdef DEBUG
  error = TTF_Init();
  if (error != 0) {fprintf(stderr,"UNABLE TO INITIALIZE FONT (disable DEBUG\n");
    return quit();}
  font = TTF_OpenFont("FreeSans.ttf", 48);
  if (!font) { fprintf(stderr, "UNABLE TO INITIALIZE FONT (disable DEBUG\n");
    return quit();}
#endif
  error = init_video();
  if (error) { fprintf(stderr, "UNABLE TO INITIALIZE VIDEO\n");
    return quit();}
  error = init_audio();
  if (error) { fprintf(stderr, "UNABLE TO INITIALIZE AUDIO\n");
    return quit();}
  return 0;
}

int quit() {
  quit_audio();
  quit_video();
#ifdef DEBUG
  TTF_Quit();
#endif
  SDL_Quit();
  return 1;
}

/* Setup the visual */
int init_video() {
  // hide the cursor
  SDL_ShowCursor(SDL_DISABLE);
  // get the 'ideally-sized' screen
  const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
  if (!vinfo)
    return 1;
  SDL_Rect **modes = SDL_ListModes(vinfo->vfmt, SDL_FULLSCREEN);
  if (!modes[0])
    return 1;
  screen = SDL_SetVideoMode(modes[0]->w,modes[0]->h, 8, 
					 SDL_FULLSCREEN);
  if (!screen)
    return 1;
  // black out the screen
  SDL_FillRect(screen, NULL, SDL_MapRGB(vinfo->vfmt, 0,0,0));
  // GOOD TO GO!
  return 0;
}
int quit_video() { return 1; }


/* Setup the audial */
int init_audio() {
  //ALenum error;
  // load ALUT
  if (alutInitWithoutContext(NULL,NULL) != AL_TRUE)
    return quit_audio();
  // create the device
  sound_dev = alcOpenDevice(NULL);
  if (!sound_dev)
    return quit_audio();
  // create the context
  sound_ctx = alcCreateContext(sound_dev, NULL);
  if (!sound_ctx)
    return quit_audio();
  // make the context current
  if (alcMakeContextCurrent(sound_ctx) != ALC_TRUE)
    return quit_audio();
  // generate and fill buffers
  alGetError();alutGetError();
  sound_bufs[AUDIO_HEARTBEAT]=alutCreateBufferFromFile("./audio/heartbeat.wav");
  sound_bufs[AUDIO_BELL] =    alutCreateBufferFromFile("./audio/bell.wav");
  sound_bufs[AUDIO_GROWL] =   alutCreateBufferFromFile("./audio/growl.wav");
  if (alGetError()!=AL_NO_ERROR || alutGetError()!=ALUT_ERROR_NO_ERROR)
    return quit_audio();
  // generate and setup sources
  alGenSources(NUM_AUDIO, sound_srcs);
  for (int i = 0; i < NUM_AUDIO; i++)
    alSourcei(sound_srcs[i], AL_BUFFER, sound_bufs[i]);
  if (alGetError() != AL_NO_ERROR)
    return quit_audio();
  // GOOD TO GO!
  return 0;
}
int quit_audio() {
  alDeleteBuffers(NUM_AUDIO, sound_bufs);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(sound_ctx);
  alcCloseDevice(sound_dev);
  alutExit();
  return 1;
}
