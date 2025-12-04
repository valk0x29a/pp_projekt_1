#include<stdbool.h>
#define BORDER -1
#define EMPTY 0
#define PLAYER 1
#define STAR 2
#define HUNTER 3

bool isAreaEmpty(int y, int x, int width, int height, int callerValue, int** occupancyCheck)
{
    for(int i = y; i < y + height; i++)
    {
        for(int j = x; j < x + width; j++)
        {
            int cellValue = occupancyCheck[i][j];
            if(cellValue != EMPTY && cellValue != callerValue)
            {
                return false;
            }
        }
    }
    return true;
}

void markEmpty(int y, int x, int width, int height, int** occupancyCheck)
{
    for(int i = y; i < y + height; i++)
    {
        for(int j = x; j < x + width; j++)
        {
            occupancyCheck[i][j] = EMPTY;
        }
    }
}