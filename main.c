#include<ncurses.h>
#include<time.h>
#include<stdbool.h>
#include<stdlib.h>
#include "helper.c"
#include "configReader.c"

#define FRAME_TIME 50
#define BORDER -1
#define EMPTY 0
#define PLAYER 1
#define STAR 2
#define HUNTER 3

typedef struct gameState
{
	int frameCounter;
	int playerSpeed;
	int timer;
	int starsLeft;
	int starSpawnTimer;
} gameState;

typedef struct bird
{
	int x;
	int y;
	int xDirection;
	int yDirection;
	int hp;
} bird;

typedef struct star
{
	int x;
	int y;
	int speed;
} star;

typedef struct star_array
{
	int firstFreeIndex;
	int capacity;
	star** stars;
} starArray;

typedef struct hunter
{
	float x;
	float y;
	float speed;
	float xDirection;
	float yDirection;
	int xDest;
	int yDest;
	int bouncesLeft;
} hunter;

typedef struct hunter_array
{
	int firstFreeIndex;
	int capacity;
	hunter** hunters;
} hunterArray;

void spawnStar(starArray* starArr, int windowXLength, int** occupancyCheck)
{
	if(starArr->firstFreeIndex == starArr->capacity)
	{
		starArr->stars = realloc(starArr->stars, sizeof(star*) * (starArr->capacity * 2));
		for(int i = starArr->capacity; i < starArr->capacity * 2; i++)
		{
			starArr->stars[i] = malloc(sizeof(star));
		}
		starArr->capacity *= 2;
	}

	star** stars = starArr->stars;
	int starIndex = starArr->firstFreeIndex;
	int randX = rand() % (windowXLength-2);
	stars[starArr->firstFreeIndex]->x = randX + 1;
	stars[starArr->firstFreeIndex]->y = 1;
	stars[starArr->firstFreeIndex]->speed = (rand() % 1) + 1;
	starArr->firstFreeIndex++;
	occupancyCheck[stars[starIndex]->y][stars[starIndex]->x] = STAR;
}

void deleteStar(starArray* starArr, int index)
{
	if(starArr->firstFreeIndex <= 0) { return; }
	if(starArr->firstFreeIndex > 1)
	{
		star* indexStar = starArr->stars[index];
		star* lastStar = starArr->stars[starArr->firstFreeIndex-1];
		starArr->stars[index] = lastStar;
		starArr->stars[starArr->firstFreeIndex-1] = indexStar;
	}
	starArr->firstFreeIndex--;
}

void despawnStarAt(starArray* starArr, int y, int x, int** occupancyCheck, WINDOW* playWindow)
{
	for(int i = 0; i < starArr->firstFreeIndex; i++)
	{
		int starX = starArr->stars[i]->x;
		int starY = starArr->stars[i]->y;
		if(starX == x && starY == y)
		{
			deleteStar(starArr, i);
			mvwaddch(playWindow, starY, starX, ' ');
			occupancyCheck[starY][starX] = EMPTY;
		}
	}
}

void spawnHunter(hunterArray* hunterArr, bird* playerBird, gameConfig* config, int** occupancyCheck)
{
	int hunterIndex = hunterArr->firstFreeIndex;
	if(hunterArr->firstFreeIndex == hunterArr->capacity)
	{
		hunterArr->hunters = realloc(hunterArr->hunters, sizeof(hunter*) * hunterArr->capacity * 2);
		for(int i = hunterArr->firstFreeIndex; i < hunterArr->capacity * 2; i++)
		{
			hunterArr->hunters[i] = malloc(sizeof(hunter));
		}
		hunterArr->capacity *= 2;
	}

	hunter** hunters = hunterArr->hunters;
	int randBorder = rand() % 4;
	int spawnX = -1;
	int spawnY = -1;
	if(randBorder%2 == 0)
	{
		spawnX = randBorder == 0 ? 2 : config->mapSizeX;
		spawnY = rand() % config->mapSizeY;
	}
	else
	{
		spawnX = rand() % config->mapSizeX;
		spawnY = randBorder == 0 ? 2 : config->mapSizeY;
	}
	hunters[hunterIndex]->x = spawnX;
	hunters[hunterIndex]->y = spawnY;
	hunters[hunterIndex]->speed = (rand() % 1) + 1;
	hunters[hunterIndex]->bouncesLeft = (rand() % 10) + 5;
	hunterArr->firstFreeIndex++;
	occupancyCheck[(int)hunters[hunterIndex]->y][(int)hunters[hunterIndex]->x] = HUNTER;
	updateHunterDirection(hunters[hunterIndex], playerBird);
}

void updateHunterDirection(hunter* hunter, bird* playerBird)
{
	float xDiff = (float)playerBird->x - hunter->x;
	float yDiff = (float)playerBird->y - hunter->y;
	float maxDiff = max2f(abs(xDiff), abs(yDiff));
	xDiff /= maxDiff;
	yDiff /= maxDiff;

	hunter->xDirection = xDiff;
	hunter->yDirection = yDiff;

	hunter->yDest = playerBird->y;
	hunter->xDest = playerBird->x;
}

void deleteHunter(hunterArray* hunterArr, int index)
{
	if(hunterArr->firstFreeIndex <= 0) { return; }
	if(hunterArr->firstFreeIndex > 1)
	{
		hunter* indexHunter = hunterArr->hunters[index];
		hunter* lastHunter = hunterArr->hunters[hunterArr->firstFreeIndex-1];
		hunterArr->hunters[index] = lastHunter;
		hunterArr->hunters[hunterArr->firstFreeIndex-1] = indexHunter;
	}
	hunterArr->firstFreeIndex--;
}

void calculateInputs(char action, gameConfig* config, gameState* state, bird* playerBird)
{
		switch(action)
		{
			case 'a':
				playerBird->yDirection = 0;
				playerBird->xDirection = -1;
				break;
			case 'w':
				playerBird->yDirection = -1;
				playerBird->xDirection = 0;
				break;
			case 's':
				playerBird->yDirection = 1;
				playerBird->xDirection = 0;
				break;
			case 'd':
				playerBird->yDirection = 0;
				playerBird->xDirection = 1;
				break;
			case'o':
				state->playerSpeed -= 1;
				state->playerSpeed = max(state->playerSpeed, config->playerMinSpeed);
				break;
			case'p':
				state->playerSpeed += 1;
				state->playerSpeed = min(state->playerSpeed, config->playerMaxSpeed);
				break;
		}
}

void updateStars(WINDOW* playWindow, starArray* starArr, gameConfig* config, int** occupancyCheck)
{
	for(int i = 0; i < starArr->firstFreeIndex; i++)
	{
		star* currentStar = starArr->stars[i];
		if(occupancyCheck[currentStar->y][currentStar->x] == STAR)
		{
			occupancyCheck[currentStar->y][currentStar->x] = EMPTY;
			mvwaddch(playWindow, currentStar->y, currentStar->x, ' ');
		}

		currentStar->y += currentStar->speed;
		if(currentStar->y >= config->mapSizeY)
		{
			/*int randX = rand() % (mainXLength-2);
			currentStar->x = randX + 1;
			currentStar->y = 3;*/
			deleteStar(starArr, i);
			continue;
		}
		occupancyCheck[currentStar->y][currentStar->x] = STAR;
		mvwaddch(playWindow, currentStar->y, currentStar->x, '*');
	}
}

void updateHunters(WINDOW* playWindow, gameConfig* config, hunterArray* hunterArr, bird* playerBird, int** occupancyCheck)
{
	for(int i = 0; i < hunterArr->firstFreeIndex; i++)
	{
		hunter* currentHunter = hunterArr->hunters[i];

		//if(occupancyCheck[(int)currentHunter->y][(int)currentHunter->x] != PLAYER)
		//{
			mvwaddch(playWindow, currentHunter->y, currentHunter->x, ' ');
		//}
		// if(occupancyCheck[(int)currentHunter->y][(int)currentHunter->x] == HUNTER)
		// {
		// 	occupancyCheck[(int)currentHunter->y][(int)currentHunter->x] = EMPTY;
		// }	

		currentHunter->x += currentHunter->xDirection * currentHunter->speed;
		currentHunter->y += currentHunter->yDirection * currentHunter->speed;

		currentHunter->x = boundf(currentHunter->x, 1, config->mapSizeX);
		currentHunter->y = boundf(currentHunter->y, 1, config->mapSizeY);

		if(currentHunter->x == 1 || currentHunter->x == config->mapSizeX
			|| currentHunter->y == 1 || currentHunter->y == config->mapSizeY)
		{
			if(currentHunter->bouncesLeft <= 0) 
			{
				int x = currentHunter->x;
				int y = currentHunter->y;
				deleteHunter(hunterArr, i); 
				mvwaddch(playWindow, y, x,  ' ');
				continue;
			}
			updateHunterDirection(currentHunter, playerBird);
			currentHunter->bouncesLeft--;
		}

		// bool isInDistance = isInRange(currentHunter->x, currentHunter->y, 
		// 						(float)currentHunter->xDest,  (float)currentHunter->yDest, 2);
		// if(isInDistance)
		// {
		// 	//updateHunterDirection(currentHunter, playerBird);
		// }

		//char temp = (int)currentHunter->yDest + '0';
		//char temp2 = (int)distance + '0';
		//mvwaddch(playWindow, currentHunter->y-2, currentHunter->x, temp);
		//mvwaddch(playWindow, currentHunter->y-1, currentHunter->x, distance + '0');
		mvwaddch(playWindow, (int)currentHunter->y, (int)currentHunter->x, '@');
		occupancyCheck[(int)currentHunter->y][(int)currentHunter->x] = HUNTER;
	}
}

void updatePlayerBird(WINDOW* playWindow, gameState* state, gameConfig* config, bird* playerBird, starArray* starArr, int** occupancyCheck)
{
	mvwaddch(playWindow, playerBird->y, playerBird->x, ' ');
	playerBird->x += playerBird->xDirection * state->playerSpeed;
	playerBird->y += playerBird->yDirection * state->playerSpeed;

	playerBird->x = bound(playerBird->x, 1, config->mapSizeX);
	playerBird->y = bound(playerBird->y, 1, config->mapSizeY);

	if(playerBird->y == 1 || playerBird->y == config->mapSizeY)
	{
		playerBird->yDirection *= -1;
			//playerBird->hp--;
	}

	if(playerBird->x == 1 || playerBird->x == config->mapSizeX)
	{
		playerBird->xDirection *= -1;
			//playerBird->hp--;
	}

	int currentCell = occupancyCheck[playerBird->y][playerBird->x];
	if(currentCell == STAR)
	{
		despawnStarAt(starArr, playerBird->y, playerBird->x, occupancyCheck, playWindow);
		state->starsLeft--;
	}
	if(currentCell == HUNTER)
	{
		playerBird->hp--;
	}
	mvwaddch(playWindow, playerBird->y, playerBird->x, '%');
	occupancyCheck[playerBird->y][playerBird->x] = PLAYER;
}

bool gameLoop(WINDOW* playWindow, WINDOW* statsWindow, gameConfig* config, gameState* state, bird* playerBird, hunterArray* hunterArr, starArray* starArr, int mainXLength, int gameYSize, int** occupancyCheck)
{
    char action = wgetch(playWindow);
	if(action == 'q') { return false; }
	calculateInputs(action, config, state, playerBird);

	updatePlayerBird(playWindow, state, config, playerBird, starArr, occupancyCheck);

	state->starSpawnTimer -= FRAME_TIME;
	if(state->starSpawnTimer <= 0)
	{
		state->starSpawnTimer = (rand() % (config->starsMaxSpawnTime - config->starsMinSpawnTime)) + config->starsMinSpawnTime;
		spawnStar(starArr, mainXLength, occupancyCheck);
	}

	updateStars(playWindow, starArr, config, occupancyCheck);
	updateHunters(playWindow, config, hunterArr, playerBird, occupancyCheck);

	wrefresh(playWindow);

	state->frameCounter++;
	paintStats(statsWindow, playerBird, state, starArr, action);
	state->timer -= FRAME_TIME;
	return true;
}

void paintStats(WINDOW* statsWindow, bird* playerBird, gameState* state, starArray* starArr, char action, int gameYSize, int mainYLength)
{
	mvwprintw(statsWindow,1,2,"Current Speed: %d", state->playerSpeed);
	mvwprintw(statsWindow,2,2, "Life Force:  %d", playerBird->hp);
	mvwprintw(statsWindow,3,2,"Stars Left: %d", state->starsLeft);
	time_t currentTime = time(0);
	mvwprintw(statsWindow,4,2,"Time Left: %d", (int)state->timer/1000);

	//hunter* test = hunterArr->hunters[0];
	//float distance = isInRange(test->x, test->y, (float)test->xDest,  (float)test->yDest, 2);
	//mvwprintw(statsWindow,mainYLength-gameYSize-5,2,"Distance: :           ");
	//mvwprintw(statsWindow,mainYLength-gameYSize-5,2,"Distance: : %d", (int)distance);
	mvwprintw(statsWindow,mainYLength-gameYSize-4,2,"Stars Capacity: %d", starArr->capacity);
	mvwprintw(statsWindow,mainYLength-gameYSize-3,2,"Current Action: %d",(int)action);
	mvwprintw(statsWindow,mainYLength-gameYSize-2,2,"Frame Count: %d",state->frameCounter);
	wrefresh(statsWindow);
}

int** initializeOccupancyMap(int mainXLength, int gameYSize)
{
	int** occupancyCheck = malloc(sizeof(int*) * gameYSize);
	for(int i = 0; i < gameYSize; i++)
	{
		occupancyCheck[i] = malloc(sizeof(int) * mainXLength);
	}

	for(int j = 0; j < gameYSize; j++)
	{
		for(int i = 0; i < mainXLength; i++)
		{
			if(j == 0 || i == 0 || j == gameYSize-1 || i == mainXLength-1)
			{
				occupancyCheck[j][i] = BORDER;
			}
		}
	}
	return occupancyCheck;
}

bird* initializePlayerBird(int gameYSize, int mainXLength)
{
	bird* playerBird = malloc(sizeof(bird));
	playerBird->y = gameYSize/2;
	playerBird->x = mainXLength/2;
	playerBird->hp = 5;
	return playerBird;
}

starArray* initializeStarArray()
{
	starArray* starArr = malloc(sizeof(starArray));

	starArr->stars = malloc(sizeof(star*) * 5);
	for(int i = 0; i < 5; i++)
	{
		starArr->stars[i] = malloc(sizeof(star));
	}
	starArr->capacity = 5;
	starArr->firstFreeIndex = 0;
	return starArr;
}

hunterArray* initializeHunterArray()
{
	hunterArray* hunterArr = malloc(sizeof(hunterArray));

	hunterArr->hunters = malloc(sizeof(hunter*) * 5);
	for(int i = 0; i < 5; i++)
	{
		hunterArr->hunters[i] = malloc(sizeof(hunter));
	}
	hunterArr->capacity = 5;
	hunterArr->firstFreeIndex = 0;
	return hunterArr;
}

gameState* initializeGameState(gameConfig* config)
{
	gameState* state = malloc(sizeof(gameState));
	state->frameCounter = 0;
	state->playerSpeed = 1;
	state->starsLeft = config->starGoal;
	int starMinSpawnTime = config->starsMinSpawnTime * 1000;
	int starMaxSpawnTime = config->starsMaxSpawnTime * 1000;
	state->starSpawnTimer = (rand() % (starMaxSpawnTime - starMinSpawnTime)) + starMinSpawnTime;
	state->timer = config->timeAvailable * 1000;
	return state;
}

int main(void)
{
	gameConfig* config = malloc(sizeof(gameConfig));
	InitializeGameConfig(config, "config.ini");

    WINDOW* parentWindow = initscr();
    int mainXLength = -1;
	int mainYLength = -1;
    getmaxyx(stdscr, mainYLength, mainXLength);

	if(config->mapSizeX > mainXLength-2)
	{
		config->mapSizeX = mainXLength-2;
	}

    int mainGameYSize = 2*mainYLength/3;

	if(config->mapSizeY > mainGameYSize-2)
	{
		config->mapSizeY = mainGameYSize-2;
	}

    WINDOW* playWindow = subwin(parentWindow ,config->mapSizeY+2, config->mapSizeX+2, 0, 0);
    WINDOW* statsWindow = subwin(parentWindow, mainYLength - config->mapSizeY+2, config->mapSizeX+2, config->mapSizeY+2, 0);
    
    noecho();
	curs_set(0);
	//cbreak() enable q to quit
    nodelay(playWindow, true);

	srand(time(NULL));
    
	wborder(playWindow, '*', '*', '*', '*', '*', '*', '*', '*');
    box(statsWindow, '|', '~');
    refresh();

	int** occupancyCheck = initializeOccupancyMap(mainXLength, mainGameYSize);

	bird* playerBird = initializePlayerBird(mainGameYSize, mainXLength);

	starArray* starArr = initializeStarArray();
	hunterArray* hunterArr = initializeHunterArray();

	for(int i = 0; i < 3; i++)
	{
		spawnHunter(hunterArr, playerBird, config, occupancyCheck);
	}

	gameState* state = initializeGameState(config);

	bool quit = false;
	while(!quit)
	{
		clock_t start = clock();
		quit = !gameLoop(playWindow, 
			statsWindow, 
			config, state, 
			playerBird, 
			hunterArr, starArr, 
			mainXLength, mainGameYSize, 
			occupancyCheck);
		clock_t end = clock();
    	int frameTime = (int)(end - start)/(CLOCKS_PER_SEC/1000000);
		frameTime = 0;
    	usleep((FRAME_TIME * 1000) - frameTime);
	}

	getch();
	endwin();
	return 0;
}
