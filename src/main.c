#include "curses.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "sys/time.h"

#define WHITE_PAIR 1
#define BLACK_PAIR 2

const int height = 25;
const int width  = 100;


struct timeval curr, prev;
bool quite = false;
int lines = 0;
int level = 0;
int points = 0;
int interval = 800;

typedef struct {
	char y;
	char x;
} vec2;

vec2 pos = {1, 32};


typedef char figure[8];
// data (y, x) pairs:
const figure O[4] = {
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1}
};

const figure J[4] = {
	{0, 0, 1, 0, 1, 1, 1, 2},
	{0, 1, 0, 2, 1, 1, 2, 1},
	{1, 0, 1, 1, 1, 2, 2, 2},
	{0, 1, 1, 1, 2, 1, 2, 0}
};

const figure L[4] = {
	{0, 2, 1, 0, 1, 1, 1, 2},
	{0, 0, 0, 1, 1, 1, 2, 1},
	{1, 0, 1, 1, 1, 2, 2, 0},
	{0, 1, 1, 1, 2, 1, 2, 2}
};

const figure T[4] = {
	{0, 1, 1, 0, 1, 1, 1, 2},
	{0, 1, 1, 1, 1, 2, 2, 1},
	{1, 0, 1, 1, 1, 2, 2, 1},
	{0, 1, 1, 0, 1, 1, 2, 1}
};

const figure Z[4] = {
	{0, 0, 0, 1, 1, 1, 1, 2},
	{0, 2, 1, 1, 1, 2, 2, 1},
	{1, 0, 1, 1, 2, 1, 2, 2},
	{0, 1, 1, 0, 1, 1, 2, 0}
};

const figure S[4] = {
	{0, 1, 0, 2, 1, 0, 1, 1},
	{0, 1, 1, 1, 1, 2, 2, 2},
	{1, 1, 1, 2, 2, 0, 2, 1},
	{0, 0, 1, 0, 1, 1, 2, 1}
};

const figure I[4] = {
	{1, 0, 1, 1, 1, 2, 1, 3},
	{0, 2, 1, 2, 2, 2, 3, 2},
	{2, 0, 2, 1, 2, 2, 2, 3},
	{0, 1, 1, 1, 2, 1, 3, 1}
};

figure *tetris;
char rot = 0;


char mod (char a, char b) {
	a %= b;
	if (a < 0)
		return a+b;
	else
		return a;
}

void mvaddfig (int y, int x, figure fig, char ch) {
	attron (COLOR_PAIR(WHITE_PAIR));
	for (int i = 0; i < 8; i += 2) {
		mvaddch (fig[i] + y, fig[i+1]*2+x, ch);
		mvaddch (fig[i] + y, fig[i+1]*2+x+1, ch);
	}
	refresh();
	attron(COLOR_PAIR(BLACK_PAIR));
}

void removefig (int y, int x, figure fig) {
	for (int i = 0; i < 8; i += 2) {
		mvaddch (fig[i]+y, fig[i+1]*2+x, ' ');
		mvaddch (fig[i]+y, fig[i+1]*2+x+1, '.');
	}
	refresh();
}

figure *genfig () {
	switch (rand()%7) {
	case 0:
		return (figure *)O;
	case 1:
		return (figure *)L;
	case 2:
		return (figure *)J;
	case 3:
		return (figure *)I;
	case 4:
		return (figure *)S;
	case 5:
		return (figure *)T;
	case 6:
		return (figure *)Z;
	}
	return (figure *) S;
}

bool checkpos (int y, int x, figure fig) {
	for (int i = 0; i < 8; i+=2) {
		char ch = mvinch(fig[i]+y, fig[i+1]*2+x) & A_CHARTEXT;
		if (ch == '<' || ch == '>' || ch == '!' || ch == 't' || ch == '='
			|| ch == '/' || ch == '\\') {
			return false;
		}

		ch = mvinch (fig[i], fig[i+1]*2+x+1) & A_CHARTEXT;
		if (ch == '<' || ch == '>' || ch == '!' || ch == 't' || ch == '='
			|| ch == '/' || ch == '\\') {
			return false;
		}
	}
	return true;
}

int init () {
	initscr();
	noecho();
	raw();
	curs_set (0);

	if (!has_colors()) {
		endwin();
		return 1;
	}

	start_color();
	init_pair (WHITE_PAIR, COLOR_WHITE, COLOR_WHITE);
	init_pair (BLACK_PAIR, COLOR_WHITE, COLOR_BLACK);

	attron (COLOR_PAIR(BLACK_PAIR));

	for (int i = 1; i < 21; i++) {
		mvaddch (i, 24, '<');
		mvaddch (i, 25, '!');
		for (int j = 26; j < 46; j += 2) {
			mvaddch (i, j, ' ');
			mvaddch (i, j+1, '.');
		}
		mvaddch (i, 46, '!');
		mvaddch (i, 47, '>');
	}
	mvaddch (21, 24, '<');
	mvaddch (21, 25, '!');
	for (int i = 26; i < 46; i++)
		mvaddch (21, i, '=');
	mvaddch (21, 46, '!');
	mvaddch (21, 47, '>');
	for (int i = 26; i < 46; i++) {
		mvaddch (22, i, '\\');
		mvaddch (22, ++i, '/');
	}

	mvprintw (1, 1, "LINES: 0");
	mvprintw (3, 1, "LEVEL: 0");
	mvprintw (5, 1, "POINTS: 0");
	mvprintw (1, 51, "press any button to start");

	gettimeofday(&prev, NULL);
	srand (prev.tv_sec);
	tetris = genfig();
	return 0;
}

int getpoints (char linecount) {
	switch (linecount) {
		case 1:
			return 40 * (level + 1);

		case 2:
			return 100 * (level + 1);

		case 3:
			return 300 * (level + 1);

		case 4:
			return 1200 * (level + 1);
	}
	return 0;
}

void fixburngen () {
	mvaddfig (pos.y, pos.x, tetris[rot], 't');

	char linecount = 0;
	for (int i = 0; i < 8; i += 2) {
		int y = (tetris[rot])[i] + pos.y;
		bool compline = true;
		for (int j = 26; j < 46; j++) {
			if ((mvinch (y, j) & A_CHARTEXT) != 't') {
				compline = false;
				break;
			}
		}
		if (compline) {
			lines++;
			linecount++;
			for (int j = y; j >= 2; j--) {
				for (int k = 26; k < 46; k++) {
					char up = mvinch (j-1, k) & A_CHARTEXT;
					if (up == 't')
						attron (COLOR_PAIR(WHITE_PAIR));
					else
						attron (COLOR_PAIR(BLACK_PAIR));
					mvaddch (j, k, up);
				}
				attron (COLOR_PAIR(BLACK_PAIR));
			}
		}
	}
	mvprintw (1, 8, "%d", lines);
	points += getpoints (linecount);
	mvprintw (5, 9, "%d", points);
	while (lines >= level*(level+1)*5 + 10*(level+1))
		level++;
	mvprintw (3, 8, "%d", level);
	interval = 800/(1.0 + level/8.0);
	pos = (vec2) {1, 32};
	rot = 0;
	tetris = genfig();
	mvaddfig (pos.y, pos.x, tetris[rot], 'c');
}


void end () {
	quite = true;
	nodelay(stdscr, false);
	for (int i = 7; i < 12; i++) {
		mvprintw (i, 51, "{}");
	mvprintw (i, 60, "{}");
	mvprintw (i, 67, "{}");
	mvprintw (i, 71, "{}");
	}
	mvprintw (7, 53, "=====");
	mvprintw (9, 53, "=====");
	mvprintw (11, 53, "=====");
	for (int i = 0; i < 5; i++) {
		mvaddch (7+i, 62+i, '\\');
	}
	mvprintw (7, 73, "===\\\\");
	mvprintw (8, 77, "{}");
	mvprintw (9, 77, "{}");
	mvprintw (10, 77, "{}");
	mvprintw (11, 73, "===//");
	refresh();
	getch();
}

void run () {
	char ch = getch();
	switch (ch) {
		case 'a':
			if (checkpos (pos.y, pos.x-2, tetris[rot])) {
				removefig (pos.y, pos.x, tetris[rot]);
				pos.x -= 2;
				mvaddfig (pos.y, pos.x, tetris[rot], 'c');
				refresh();
			}
			break;

		case 'd':
			if (checkpos (pos.y, pos.x+2, tetris[rot])) {
				removefig (pos.y, pos.x, tetris[rot]);
				pos.x += 2;
				mvaddfig (pos.y, pos.x, tetris[rot], 'c');
				refresh();
			}
			break;

		case 'q':
			if (checkpos (pos.y, pos.x, tetris[mod (rot-1, 4)])) {
				removefig (pos.y, pos.x, tetris[rot]);
				rot = mod (rot-1, 4);
				mvaddfig (pos.y, pos.x, tetris[rot], 'c');
				refresh();
			}
			break;

		case 'e':
			if (checkpos (pos.y, pos.x, tetris[mod (rot+1, 4)])) {
				removefig (pos.y, pos.x, tetris[rot]);
				rot = mod (rot+1, 4);
				mvaddfig (pos.y, pos.x, tetris[rot], 'c');
				refresh();
			}
			break;

		case 's':
			if (checkpos (pos.y+1, pos.x, tetris[rot])) {
				removefig (pos.y, pos.x, tetris[rot]);
				mvaddfig (++pos.y, pos.x, tetris[rot], 'c');
			}
			else {
				fixburngen();
				if (!checkpos (pos.y, pos.x, tetris[rot])) {
					end();
					getch();
					return;
				}
			}

			refresh();
			break;

		case ' ':
			while (checkpos (pos.y+1, pos.x, tetris[rot])) {
					removefig (pos.y, pos.x, tetris[rot]);
					mvaddfig (++pos.y, pos.x, tetris[rot], 'c');
			}
			fixburngen();
			if (!checkpos (pos.y, pos.x, tetris[rot])) {
				end();
				getch();
				return;
			}
			refresh();
			break;

		case 'x':
			quite = true;
	}

	gettimeofday(&curr, NULL);
	if ((curr.tv_sec - prev.tv_sec)*1000 + (curr.tv_usec - prev.tv_usec)/1000 >= interval) {
		if (checkpos (pos.y+1, pos.x, tetris[rot])) {
			removefig (pos.y, pos.x, tetris[rot]);
			mvaddfig (++pos.y, pos.x, tetris[rot], 'c');
		}
		else {
			fixburngen();
			if (!checkpos (pos.y, pos.x, tetris[rot])) {
				end();
				getch();
				return;
			}
		}
		refresh();
		prev = curr;
	}

}


int main () {
	if (init())
		goto _bailout;
	refresh();
	getch();
	mvprintw (1, 51, "press 'x' to exit        ");
	mvaddfig (pos.y, pos.x, tetris[rot], 'c');
	nodelay (stdscr, TRUE);
	while (!quite) {
		run();
	}

_bailout:
	move (0, 0);
	nodelay(stdscr, FALSE);
	curs_set (1);
	endwin();
	system ("stty sane");
	return 0;
}
