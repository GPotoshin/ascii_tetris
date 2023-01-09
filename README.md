# ASCII TETRIS GAME
Download with `git clone https://github.com/GPotoshin/ascii_tetris`
Run `make` to compile that project.
You might need to download `ncurses` library on your machine.

<div align="left">
	<img src="http://1189.codelabs.ru/0x14/gp/img/tetris.jpg">
</div>

# How my code is organized
## Data
Each figure is an array of four pointers to arrays of 8 numbers, which are allocated on stack.
```c
typedef char figure[8];
const figure O[4] = {
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1},
	{0, 0, 0, 1, 1, 0, 1, 1}
};
```
Those octates numbers are divided in pairs of relevant coordinates. Arrays are placed in order of rotation, so it can be easily done by adding or subtracting 1 in the ring of rests of 4.
## Tetris
The position is stored in global variable ```pos```, rotation in ```rot```, and ```figure *tetris``` is a pointer to the current figure displayed on the screen.
## Program structure
```c
int main () {
	if (init())
		goto _bailout;
	refresh();
	getch(); // wait to start
	mvprintw (1, 51, "press 'x' to exit        ");
	mvaddfig (pos.y, pos.x, tetris[rot], 'c');
	// set game not to wait for input
	nodelay (stdscr, TRUE); 
	
	while (!quite) {
		run();
	}

_bailout:
	// set back to default settings
	move (0, 0);
	nodelay(stdscr, FALSE);
	curs_set (1);
	endwin();
	system ("stty sane");
	return 0;
}
```
Despite that goto constructions are considered a bad habit, their main idea was not to use them instead of `if`, `else`, `for` and `while` etc. In C we do not have exceptions and goto can easily blend into code to satisfy that need.
## Functions 
```c
char mod (char a, char b)
```
Unfortunately % is not the same thing as taking rest of division
```c
void mvaddfig (int y, int x, figure fig, char ch)
```
I'm trying to keep the same style of naming functions as in `ncurses` library. It draws figure at a right spot
```c
void removefig (int y, int x, figure fig)
```
Removes figure while keeping background filling
```c
figure *genfig ()
```
Returns random figure
```c
bool checkpos (int y, int x, figure fig)
```
Checks whether the figure can be placed there
```c
int init ()
```
It draws board and texts, sets up ncurses
```c
int getpoints (char linecount)
```
Your level based on number of cleaned lines
```c
void end ()
```
Draws end logo
```c
void run ()
```
Loop function with events, button checks and gameplay