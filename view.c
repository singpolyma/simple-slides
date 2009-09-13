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
	struct frame first_frame = blank_frame;
	struct frame *current_frame = &first_frame;

	while(fgets(line, PATH_MAX+10, stdin)) {
		if(line[0] && line[0] != '\n') {
			load_frame(line, current_frame);
			current_frame->next = xmalloc(sizeof(*current_frame));
			current_frame->next->prev = current_frame;
			current_frame = current_frame->next;
		}
	}

	return EXIT_SUCCESS;
}
