// C++ 11 required

#include <iostream>
#include <windows.h>
#include <math.h>
#include <chrono>
#include <string>

const int mapHeight=16;
const int mapWidth=16;

const float FOV = 65.0f;
const float precision = 0.015f;
const float playerspeed = 1.8f;
const float rotationspeed = 70;

int screenWidth= 240;
int screenHeight= 80;
int renderdistance = 20;

char level[mapWidth][mapHeight];
std::string stlevel;

float playerX=1.5f;
float playerY=1.5f;
float playerA=0.0f;


void debugLevelPrint();
void levelGen();
void stringMapInjector(std::string m);

float angleConverter(float angle);
float hypotenuse(float x, float y);
float rayDistance(float angle);
float degreeRadianConversions(float angle,bool type);
float playerDistance(float angle, float h);

int main()
{
    auto clk = std::chrono::high_resolution_clock::now();
    auto prevClk = clk;
    std::chrono::duration<double> tick;

    //level as a string
    stlevel += "################";
    stlevel += "#............###";
    stlevel += "#...#.......##.#";
    stlevel += "#..........##..#";
    stlevel += "#.........##...#";
    stlevel += "########.##....#";
    stlevel += "#......#.#.....#";
    stlevel += "#......#.#.....#";
    stlevel += "#..#.#.#.####.##";
    stlevel += "#..#.#.#.#.....#";
    stlevel += "#..#.#.#.#.....#";
    stlevel += "#..#.#.#.#.....#";
    stlevel += "#..#.#.#.#.....#";
    stlevel += "#..###.#.###.###";
    stlevel += "#..............#";
    stlevel += "################";


    srand(time(NULL));
    char *screen = new char [screenWidth*screenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,0,NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    if((stlevel.length()<mapWidth*mapHeight)||(stlevel.length()>mapWidth*mapHeight))
        levelGen();
    else
        stringMapInjector(stlevel);

    //debugLevelPrint();

    //std::cout<<rayDistance(playerA);


    //game loop
    while(true)
    {
        prevClk = clk;
        clk = std::chrono::high_resolution_clock::now();
        tick = clk-prevClk;
        float interval = tick.count(); // interval between frames in float

        if(GetAsyncKeyState((unsigned short)'A') & 0x8000)
        {
            playerA+=rotationspeed*interval;
            if(playerA>=360)
            {
                playerA=0;
            }
        }
        else if(GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            playerA-=rotationspeed*interval;
            if(playerA<0)
            {
                playerA=359;
            }
        }
        if(GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            float x = (playerspeed*cos(degreeRadianConversions(playerA,0)))*interval;
            float y = (playerspeed*sin(degreeRadianConversions(playerA,0)))*interval;
            if(level[long(playerX+x)][long(playerY+y)]!='#')
            {
                playerX += x;
                playerY += y;
            }
        }
        else if(GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            float x = (playerspeed*cos(degreeRadianConversions(playerA,0)))*interval;
            float y = (playerspeed*sin(degreeRadianConversions(playerA,0)))*interval;
            if(level[long(playerX-x)][long(playerY-y)]!='#')
            {
                playerX -= x;
                playerY -= y;
            }
        }
        //for each column of screen
        for(int c = 0;c<screenWidth;c++)
        {
            float rayangle = (playerA+(FOV/2.0f))+((FOV/static_cast<float>(screenWidth))*(-1.0f*c));
            float distance = rayDistance(rayangle);
            distance = playerDistance(rayangle,distance);
            float wallheight = ((screenHeight)/distance);
            int halfheight = round(wallheight);
            if(halfheight>=(screenHeight/2)-1)
                halfheight=(screenHeight/2)-1;
            //for each row of the column
            for(int r = 0;r<screenHeight;r++)
            {
                int row = (screenWidth*r)+c;
                if((r<(screenHeight/2-halfheight) && r<screenHeight/2)||(r>(screenHeight/2+halfheight) && r>screenHeight/2))
                {
                        screen[row] = ' ';
                }
                else if((r>(screenHeight/2-halfheight) && r<=screenHeight/2)||(r<(screenHeight/2+halfheight) && r>screenHeight/2))
                {

                    if (distance<=1)
                        screen[row]= '#';
                    if (distance>1 && distance<=2)
                        screen[row]= '*';
                    if (distance>=2 && distance<4)
                        screen[row]= '=';
                    if (distance>=4 && distance<6)
                        screen[row]= '+';
                    if (distance>=6 && distance<8)
                        screen[row]= ':';
                    if (distance>=8)
                        screen[row]= '.';

                    if(distance<0)
                        screen[row]= ' ';

                }
            }

        }
        //loop to display time between frames (ms)
        std::string fr = std::to_string(interval);
        for(int l=0;l<fr.length();l++)
        {
            screen[l] = fr[l];
        }

        //write screen to console
        screen[screenWidth*screenHeight-1]= '\0';
        WriteConsoleOutputCharacter(hConsole,(const char*)screen,screenWidth*screenHeight,{0,0},&dwBytesWritten);

    }


    return 0;
}


void levelGen()
{
    for(int y=0;y<mapHeight;y++)
    {
        for(int x=0;x<mapWidth;x++)
        {
            if(y==mapHeight-1 || y==0 || x==mapWidth-1 || x==0)
                level[x][y] = '#';
            else
                level[x][y] = '.';
        }

    }

}

// used only to print level when console handle isn't working
void debugLevelPrint()
{
    for(int y=0;y<mapHeight;y++)
    {
        for(int x=0;x<mapWidth;x++)
        {
            std::cout<<level[x][y];
        }
        std::cout<<"\n";
    }
}
//calculates distance from player to wall along ray, returns -1 if distance is over 16
float rayDistance(float angle)
{
    float distance=0;
    float rayX = precision*cos(degreeRadianConversions(angle,0));
    float x = rayX;
    float rayY = precision*sin(degreeRadianConversions(angle,0));
    float y = rayY;

    while(distance<renderdistance)
    {
        if(level[long(rayX+playerX)][long(rayY+playerY)] != '#')
        {
            rayX+=x;
            rayY+=y;
            distance = hypotenuse(rayX,rayY);
        }
        else
        {
            return hypotenuse(rayX,rayY);
        }
    }
    return -1;
}

float playerDistance(float angle, float h)
{
    float relativeangle = fabs(angle-playerA);
    return (h*cos(degreeRadianConversions(relativeangle,0)));
}



float angleConverter(float angle)
{
    if(angle<=180 && angle>90)
        return 180.0f-angle;
    else if(angle>180 && angle<=270)
        return (-1.0f*(angle-180.0f));
    else if(angle<360 && angle>270)
        return (-1.0f*(360.0f-angle));

    return angle;
}

float degreeRadianConversions(float angle,bool type)
{
    //from degrees to radians
    if(type==0)
    {
        return (angle*(3.14159f/180.0f));
    }
    //from radians to degrees
    else
    {
        return (angle*(180.0f/3.14159f));
    }

}

float hypotenuse(float x, float y)
{
    return sqrt((x*x)+(y*y));
}

void stringMapInjector(std::string m)
{
    for(int y=0;y<mapHeight;y++)
    {
        for(int x=0;x<mapWidth;x++)
        {
            level[x][y]= m[x+(y*mapWidth)];
        }
    }
}

