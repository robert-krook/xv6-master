#include "types.h"
#include "user.h"

int test_draw_line(void);
int draw_grid(void);
int draw_food(unsigned long *);
int draw_snake(int, int*, int*);

int main(void) {
	int k;

	//initial starting snake coordinates
	int x = 320;
	int y = 240;

	// variables used for generating random food locations
	unsigned long randstate = 1;
	/*int xfood;
	int x1food;
	int yfood;
	int y1food;
	*/
	
	//int length = 0;

	int curdir = 0;

	init_graphics();
	draw_grid();

	//after initialize starting snake, set starting direction to up
	draw_snake(curdir, &x, &y);
	curdir = -30;
	
	draw_food(&randstate);

	blit();

	while(1){
		k = getkey();
		if(k == -1){
			if(draw_snake(curdir, &x, &y) < 0)		
				break;
		} else {
			//checking corner case of if user enters opposite direction currently going in 
			if((curdir == -27 && k != -28) || (curdir == -28 && k != -27) || (curdir == -29 && k != -30) || (curdir == -30 && k != -29))
				curdir = k;

			if(draw_snake(curdir, &x, &y) < 0)
				break;		
		}
	}
	
	exit_graphics();
	exit();	//will issue page fault in xv6 if don't call exit() at end of program
	
	//return 0;
}


//Method to test the functionality of my draw_line system call
int test_draw_line(void) {
	draw_line(0, 0, 34, 60, 6);
	draw_line(50, 0, 50, 50, 8);
	return 0;
}


int draw_grid(void) {
	
	int i;	
	for(i = 0; i < 75; i++){
		draw_line(0, i, 639, i, 4);
	}

	for(; i < 405; i++) {
		draw_line(0, i, 99, i, 4);
		draw_line(539, i, 639, i, 4); 
	}

	for(; i < 480; i++) {
		draw_line(0, i, 639, i, 4);
	}
	
	blit();
	
	return 0;
}

//Only keeping track of single pixel snake but drawing double pixel snake
int draw_snake(int dir, int *x, int *y) {
	int flag = 0;
	
	//checks if initializing the snake at the start of the game
	if(dir == 0){
		draw_line(*x, *y, (*x)+1, *y, 2);
		blit();
		return 0;
	}

	switch (dir) {
		case -27:	
			(*x)++;
			draw_line(*x, *y, *x, (*y)-1, 2);
			blit();
			
			if((*x) >= 539){
				flag = -1;
			} 

			break;
		case -28:	
			(*x)--;
			draw_line(*x, *y, *x, (*y)-1, 2);
			blit();
			
			if((*x) <= 99){
				flag = -1;
			}
			
			break; 	
		case -29:	
			(*y)++;
			draw_line(*x, *y, (*x)+1, *y, 2);
			blit();
			
			if((*y) >= 405) {
				flag = -1;
			}

			break;
		case -30:	
			(*y)--;
			draw_line(*x, *y, (*x)+1, *y, 2);
			blit();
			
			if((*y) <= 74) {
				flag = -1;
			}
			
			break;
	}

	return flag;
}
int draw_food(unsigned long *randstate ) {
	
	int valid = 0;
	
	while(valid == 0){
		*randstate = (*randstate * 1664525 + 1013904223);
	
		//calculate a random location for x and y value and offset by distance of out of bounds area
		//subtract 1 from each mod so can fill in 4 pixels around randomly selected coordinate
		int xfood = 100 + (*randstate % 438);
		int yfood = 75 + (*randstate % 329);
	
		int xxfood = xfood++;
		int yyfood = yfood++;
		
		if((xxfood >= 539) || (yyfood >= 405)) {
			continue;
		}

		draw_line(xfood, yfood, xxfood, yfood, 7);
		draw_line(xfood, yyfood, xxfood, yyfood, 7);
		blit();
		valid = 1;

	}
	
	return 0;
}
