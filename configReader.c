#include <stdio.h>
#include<stdbool.h>
#define LINE_LENGTH 256

typedef struct gameConfig
{
    int starGoal;
    int starsMinSpawnTime;
    int starsMaxSpawnTime;
    int timeAvailable;
    int playerMinSpeed;
    int playerMaxSpeed;
} gameConfig;

bool areStringsSame(char* s1, char* s2)
{   
    int i = 0;
    while(s1[i] != 0 || s2[i] != 0)
    {
        if(s1[i] != s2[i]) {return false; }
        i++;
    }
    return true;
}

int GetIntValue(char* string)
{
    int value = 0;
    for(int i = 0; string[i] != 0; i++)
    {
        value *= 10;
        value += string[i] - '0';
    }
    return value;
}

void SetValue(gameConfig* config, char* name, char* value)
{
    if(areStringsSame(name, "stars_goal")) { config->starGoal = GetIntValue(value); }
    if(areStringsSame(name, "stars_min_spawn_time")) { config->starsMinSpawnTime = GetIntValue(value); }
    if(areStringsSame(name, "stars_max_spawn_time")) { config->starsMaxSpawnTime = GetIntValue(value); }
    if(areStringsSame(name, "time_available")) { config->timeAvailable = GetIntValue(value); }
    if(areStringsSame(name, "player_min_speed")) { config->playerMinSpeed = GetIntValue(value); }
    if(areStringsSame(name, "player_max_speed")) { config->playerMaxSpeed = GetIntValue(value); }
}

void ProcessLine(gameConfig* config, char* line, char* name, char* value)
{
    for(int i = 0; i < LINE_LENGTH; i++)
    {
        name[i] = ' ';
        value[i] = ' ';
    }

    int i = 0;
    while(line[i] != ':')
    {
        if(line[i] == EOF || line[i] == '\n') { break; }
        name[i] = line[i];
        i++;
    }
    name[i] = '\0';

    int j = 0;
    i++;
    while(line[i])
    {
        if(line[i] == EOF || line[i] == '\n') { break; }
        value[j] = line[i];
        i++;
        j++;
    }
    value[j] = '\0';

    SetValue(config, name, value);
}

void InitializeGameConfig(gameConfig* config, const char* filename)
{
    FILE* file = fopen(filename, "r");
    char* line =  malloc(LINE_LENGTH);
    
    char* name = malloc(LINE_LENGTH);
    char* value = malloc(LINE_LENGTH);

    while(fgets(line, LINE_LENGTH, file))
    {
        ProcessLine(config, line, name, value);
    }

    free(name);
    free(value);
}
