#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "SDL.h"
#include "SDL_image.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DEFAULT_FRAME_LENGTH 2

struct frame {
	SDL_Surface *image;
	unsigned int length;
	char *transition;
	struct frame *next;
	struct frame *prev;
};
const struct frame blank_frame = {NULL,0,NULL,NULL,NULL};

/* Load the frame represented by the string line into frame */
void load_frame(char *line, struct frame *frame) {
	char *length, *transition;
	line[strlen(line)-1] = '\0'; /* chop */
	frame->length = DEFAULT_FRAME_LENGTH;
	if((length = strchr(line, ':'))) {
		*length = '\0';
		length++;
		/* Split string fully before we can use any parts */
		if((transition = strchr(length, ':'))) {
			*transition = '\0';
			transition++;
			frame->transition = strdup(transition);
			/* On error, transition will be NULL, which is "random", which is a decent fallback */
		}
		/* Try conversion to int, fallback to default */
		errno = 0;
		frame->length = strtol(length, NULL, 10);
		if(errno != 0 || frame->length == 0) {
			frame->length = DEFAULT_FRAME_LENGTH;
		}
	}
	frame->image = IMG_Load(line);
	if(!frame->image) {
		fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}
}

#define TIMER_EVENT 50
Uint32 timer_event(Uint32 interval, void *param) {
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = TIMER_EVENT;
	userevent.data1 = param;
	userevent.data2 = NULL;
	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return interval;
}

void *xmalloc(size_t s) {
	void *p = malloc(s);
	if(!p) {
		perror("xmalloc");
		abort();
	}
	return p;
}

int main(int argc, char *argv[]) {
	char line[PATH_MAX+10];
	SDL_Surface *screen;
	const SDL_VideoInfo* screen_info;
	struct frame first_frame = blank_frame;
	struct frame *current_frame = &first_frame;

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	atexit(SDL_Quit);

	while(fgets(line, PATH_MAX+10, stdin)) {
		if(line[0] && line[0] != '\n') {
			load_frame(line, current_frame);
			current_frame->next = xmalloc(sizeof(*current_frame));
			current_frame->next->prev = current_frame;
			current_frame = current_frame->next;
		}
	}

	/* Get screen resolution and initialize window */
	screen_info = SDL_GetVideoInfo();
	if(!screen_info) {
		fprintf(stderr, "SDL_GetVideoInfo: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if(!(screen = SDL_SetVideoMode(screen_info->current_w, screen_info->current_h, 0, SDL_HWSURFACE))) {
		fprintf(stderr, "Unable to init SDL screen: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	/* Fake the first timer event */
	timer_event(0, &first_frame);

	/* Wait for exit events */
	while(1) {
		SDL_Rect rect;
		SDL_Event event;
		event.type = 0;
		if(!SDL_WaitEvent(&event)) {
			fprintf(stderr, "Unable to get event: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		switch(event.type) {
			case SDL_USEREVENT:
				switch(event.user.code) {
					case TIMER_EVENT:
						current_frame = event.user.data1;
						/* This may already be the last slide */
						if(!current_frame) {
							goto mainloopend;
						}

						/* Calculate where to draw the image */
						rect.x = (screen->w/2)-current_frame->image->w;
						rect.y = (screen->h/2)-current_frame->image->h;
						rect.w = current_frame->image->w;
						rect.h = current_frame->image->h;

						/* Redraw dirtied background */
						if(SDL_FillRect(screen, &rect, 0) != 0) {
							fprintf(stderr, "Unable to draw rectangle: %s\n", SDL_GetError());
							exit(EXIT_FAILURE);
						}

						/* Draw image */
						if(SDL_BlitSurface(current_frame->image, NULL, screen, &rect) != 0) {
							fprintf(stderr, "Unable to blit frame: %s\n", SDL_GetError());
							exit(EXIT_FAILURE);
						}

						/* Update dirty rects */
						SDL_UpdateRects(screen, 1, &rect);

						/* Wait until this slide is done before advancing */
						SDL_AddTimer(current_frame->length*1000, &timer_event, current_frame->next);
						break;
				}
				break;
			case SDL_QUIT:
				goto mainloopend;
		}
	}
mainloopend:

	exit(EXIT_SUCCESS);
}
