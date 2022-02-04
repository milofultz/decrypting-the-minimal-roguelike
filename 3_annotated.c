// http://www.locklessinc.com/articles/512byte_roguelike/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>

#define SIZEX	128
#define	SIZEY	64
static char map[SIZEY][SIZEX];

#define MMAX	50
static int mx[MMAX];
static int my[MMAX];
static char mc[MMAX];
static int mhp[MMAX];

#define FLOOR   '.'
#define PLAYER  '@'
#define MONSTER 'm'

// Within these regions, draw this char
static void draw_region(int x1, int x2, int y1, int y2, char c)
{
	int x, y;
	
	for (y = y1; y < y2; y++)
	{
		for (x = x1; x < x2; x++)
		{
			map[y][x] = c;
		}
	}
}

// Get a random number within the bounds of v
static int rr(int v1, int v2, int r)
{
	return (r % (v1 - v2)) + v1;
}

// Add character within X/Y (within limit MMAX characters in total)
static void addm(int x, int y, int hp, char c)
{
	int i;
	
	for (i = 0; i < MMAX; i++)
	{
		// If no character exists
		if (!mc[i])
		{
			mx[i] = x;
			my[i] = y;
			mhp[i] = hp;
			mc[i] = c;
			return;
		}
	}
}

// Get the character ID at X/Y coord; -1 if none
static int getm(int x, int y)
{
	int i;
	for (i = 0; i < MMAX; i++)
	{
		// If character exists at coordinates, return which one
		if ((mx[i] == x) && (my[i] == y)) return i;
	}
	
	return -1;
}

// Place character at random X/Y coord if nothing is there
static void rand_place(char c, int hp)
{
	do
	{
		// Get a random coordinate within the bounds
		int x = rand() & (SIZEX - 1);
		int y = rand() & (SIZEY - 1);
		
		if (map[y][x] == FLOOR)
		{
			map[y][x] = c;
			addm(x, y, hp, c);
			return;
		}
	}
	while(1);
}


static void init_map(void)
{
	int i, x1, x2, y1, y2;
	
	// initialize the whole map with # (walls)
	draw_region(0, SIZEX - 1, 0, SIZEY, '#');
	
	for (i = 0; i < 200; i++)
	{
		// Get a random number between 0 and RAND_MAX
		int r = rand();
		// Set coordinates as a random number within the bounds
		x1 = rr(1, SIZEX - 20,r);
		x2 = rr(x1 + 5, x1 + 19,r);
		y1 = rr(1, SIZEY - 8,r);
		y2 = rr(y1 + 3, y1 + 7,r);
		
		// draw floor at these coordinates
		draw_region(x1, x2, y1, y2, FLOOR);
	}
	
	// Place the main character (@) somewhere on the map with 5 hp
	rand_place(PLAYER, 5);
	
	// Place ten enemies on the map
	for (i = 0; i < 10; i++)
	{
		rand_place(MONSTER, 2);
	}
}

// Draw each row of the map on to the screen
static void draw_screen(void)
{
	int y;
	
	clear();
	
	/* Dump map */
	for (y = 0; y < SIZEY; y++)
	{
		mvaddstr(y, 0, map[y]);
	}
}

// Return position adjustment based on position of player and monster
static int sgn(int x)
{
	return (x > 0) - (x < 0);
}

int main(void)
{
	int c;          // declare user input
	int i = 0;      //
	srand(time(0)); // use current time as seed
	init_map();     //
	
	initscr(); // initialize curses
	cbreak();  // allow user input to be used immediately https://linux.die.net/man/3/cbreak
	
	while (1) // main game loop
	{
		int nx = mx[i], ny = my[i]; // character (player or monster) position
		if (!i)                     // if it is the user's turn
		{
			draw_screen();  // draw the screen
			c = 0;          // Initialize input char
			
			while (!(c=getch())); // wait for user input
			
			// move character based on input
			if ((c == '1') || (c == '4') || (c == '7')) nx -= 1;
			if ((c == '3') || (c == '6') || (c == '9')) nx += 1;
			if ((c == '7') || (c == '8') || (c == '9')) ny -= 1;
			if ((c == '1') || (c == '2') || (c == '3')) ny += 1;
			
			// if an enemy is at the current coordinate
			if (map[ny][nx] == MONSTER)
			{
				// figure out which one it is
				int j = getm(nx, ny);
				
				// take away one hp from that monster
				mhp[j]--;
				
				// if at zero
				if (!mhp[j])
				{
					// set the character at the map to be ground
					map[ny][nx] = FLOOR;
					// remove the monster from the map
					mc[j] = 0;
				}
			}
		}
		
		// If a monster's turn
		if (mc[i] == MONSTER)
		{
			// Move the monster one space closer to the player if possible
			nx += sgn(mx[0]-nx);
			ny += sgn(my[0]-ny);
			
			// If the space they are moving to is the player
			if (map[ny][nx] == PLAYER)
			{
				// Take one hp from the player
				mhp[0]--;
				
				// If the player takes 5 hits
				if (!mhp[0])
				{
					// end game and quit
					endwin();
					exit(0);
				}
			}
		}

		// If character exists and there is nothing in the moved-to space
		if (mc[i] && (map[ny][nx] == FLOOR))
		{
			// Set old position to ground (.)
			map[my[i]][mx[i]] = FLOOR;

			// Set new position
			mx[i] = nx;
			my[i] = ny;

			// Set new map character
			map[ny][nx] = mc[i];
		}
		
		// Iterate to next character
		i++;
		
		// If at max characters, start over for new turn at player
		if (i == MMAX) i = 0;
	}
	
	return(0);
}