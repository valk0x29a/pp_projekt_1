#include<ncurses.h>
#include<time.h>
#include<stdbool.h>
#include<stdlib.h>

void prepareBoard(int y, int x, int gameYSize)
{
	for(int i = 0; i < gameYSize; i++)
	{
		for(int j = 0; j < x; j++)
		{
			if(i == 0 || j == 0 || i == gameYSize-1 || j == x-1) 
			{ 
				mvaddch(i, j, '*'); 
			}
		}
	}
	for(int i = gameYSize; i < y; i++)
	{
		for(int j = 0; j < x; j++)
		{
			if(i == gameYSize || j == 0 || i == y-1 || j == x-1) 
			{ 
				mvaddch(i, j, '*'); 
			}
		}
	}
}

int main(void)
{
    WINDOW* parentWindow = initscr();
    int mainXLength = -1;
	int mainYLength = -1;
    getmaxyx(stdscr, mainYLength, mainXLength);
    int gameYSize = 2*mainYLength/3;
    WINDOW* playWindow = subwin(parentWindow, gameYSize, mainXLength, 0, 0);
    WINDOW* statsWindow = subwin(parentWindow, mainYLength - gameYSize, mainXLength, gameYSize, 0);
    
    //cbreak();
    noecho();
	curs_set(0);
    nodelay(playWindow, true);
	srand(time(NULL));
	//printw("Hello World!");


    mvwprintw(playWindow, gameYSize-1,1,"%d",mainYLength);
    
    box(playWindow, '*', '*');
    box(statsWindow, '|', '~');
    refresh();

	bool quit = false;
	int frameCounter = 0;
	int starXPos = 5;
	int sstarXPos = 3;
    //mvwprintw(statsWindow, 1, 2,"%s","Hello Stats");
    //refresh();
	while(!quit)
	{
		clock_t start = clock();
        char action = wgetch(playWindow);

		frameCounter++;
		clock_t secondsEnd = clock();
		mvwprintw(statsWindow,1,2,"%d",frameCounter);
		mvwprintw(statsWindow,2,2,"%d",(int)action);
		//mvwaddch(statsWindow, 2, 2, );
		wrefresh(statsWindow);
		if(frameCounter % gameYSize == 1)
		{
			int randX = rand() % (mainXLength-2);
			starXPos = randX + 1;
		}
		if(frameCounter % gameYSize > 1)
		{
			int y = frameCounter % gameYSize;
			mvwaddch(playWindow, y-1, starXPos, ' ');
			mvwaddch(playWindow, y, starXPos, '*');
		}
		
		if((frameCounter + 5) % gameYSize == 1)
		{
			int randX = rand() % (mainXLength-2);
			sstarXPos = randX + 1;
		}
		if((frameCounter + 5) % gameYSize > 1)
		{
			int y = (frameCounter + 5) % gameYSize;
			mvwaddch(playWindow, y-1, sstarXPos, ' ');
			mvwaddch(playWindow, y, sstarXPos, '*');
		}
		wrefresh(playWindow);
        clock_t end = clock();
        //mvwprintw(statsWindow,4 ,2,"%d",(int)(end - start)/(CLOCKS_PER_SEC/1000));
        //wrefresh(statsWindow);
        int frameTime = (int)(end - start)/(CLOCKS_PER_SEC/1000);
        usleep((33 - frameTime) * 1000);
	}
	
	//mvaddch(gameYSize+1,1,'c');
	//refresh();
	getch();
	endwin();
	return 0;
}
