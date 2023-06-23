#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include "libraries/PerlinNoise-master/PerlinNoise.hpp"
#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 1080
#define colSize 1080
#define rowSize 1080
#define SQUARE_COL(intensity) Color(255 * intensity, 0, 0)
using namespace std;
using namespace sf;
using namespace siv;
////////////////////////////////////////////////
RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Smooth Life");
RenderTexture texture;
float SQUARE_SIZE = (float)SCREEN_WIDTH / (float)colSize;
Clock simClock, frameClock;
float timeFrameStep = 60.0f;
float FPS = 0;
Time updateInterval = seconds(1.0f / timeFrameStep);
Time frameUpdateInterval = seconds(1.0f);
Time timeEnd, frameTimeEnd;
int frameCount = 0;
float grid[rowSize * colSize] = {0};
float gridBuffer[rowSize * colSize] = {0};
int lastHovered = -1;
bool pause = true;
bool run = true;
VertexArray square(Quads, 4 * rowSize * colSize);
int imageCount = 0;
/////////////Propriatary blob Variables///////////
float radiusOuter = 21;
float radiusInner = radiusOuter / 3;
float alpha = 0.028;
// float alpha = 0.147;
float birth1 = 0.278;
float birth2 = 0.365;
float death1 = 0.267;
float death2 = 0.445;
/////////////////////////////////////////////////

void addQuad(Vector2f pos, VertexArray *va, Color color, float size, int *index);
void clearBuffer();
void addQuad(Vector2f pos, VertexArray *va, float size, int *index);
float valueAt(int row, int column, bool buffer);
void setCell(int row, int column, float value, bool bufferSet);
int aliveNeighbors(int row, int column);
void step();
void update();
void init();
void eventLoop();
void printGrid(bool buffer);
int loop_mod(int dividend, int divisor);
float sigma1(float x, float a);
float sigma2(float x, float a, float b);
float sigmaM(float x, float y, float m);
float transitionFunction(float n, float m);
float clamp(float x, float high, float low);

int main()
{
    srand(time(NULL));
    simClock.restart();
    frameClock.restart();
    texture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
    init();
    texture.draw(square);
    texture.display();
    while (window.isOpen())
    {
        eventLoop();
        if (run)
        {
            if (!pause)
            {
                step();
                update();
                texture.clear(Color::Transparent);
                texture.draw(square);
                string url = "./output_frames/" + to_string(imageCount) + ".png";
                texture.getTexture().copyToImage().saveToFile(url);
                imageCount++;
            }
            window.clear();
            texture.display();
            window.draw(Sprite(texture.getTexture()));
            window.display();
            frameCount++;
            frameTimeEnd = frameClock.getElapsedTime();
            if (frameTimeEnd >= frameUpdateInterval)
            {
                FPS = frameCount / frameTimeEnd.asSeconds();
                frameCount = 0;
                frameTimeEnd = Time::Zero;
                frameClock.restart();
                cout << FPS << endl;
            }
            timeEnd = simClock.getElapsedTime();
            // if (!pause && timeEnd >= updateInterval)
            // {
            //     step();
            //     update();
            //     simClock.restart();
            // }
        }
        else
        {
            break;
        }
    }
    return 0;
}

void addQuad(Vector2f pos, VertexArray *va, Color color, float size, int *index)
{
    (*va)[*index].position = pos;
    (*va)[*index + 1].position = Vector2f(pos.x + size, pos.y);
    (*va)[*index + 2].position = Vector2f(pos.x + size, pos.y + size);
    (*va)[*index + 3].position = Vector2f(pos.x, pos.y + size);

    (*va)[*index].color = color;
    (*va)[*index + 1].color = color;
    (*va)[*index + 2].color = color;
    (*va)[*index + 3].color = color;
    *index += 4;
}
void clearBuffer()
{
    for (int i = 0; i < rowSize; i++)
        for (int j = 0; j < colSize; j++)
            gridBuffer[i * colSize + j] = false;
}
void addQuad(Vector2f pos, VertexArray *va, float size, int *index)
{
    (*va)[*index].position = pos;
    (*va)[*index + 1].position = Vector2f(pos.x + size, pos.y);
    (*va)[*index + 2].position = Vector2f(pos.x + size, pos.y + size);
    (*va)[*index + 3].position = Vector2f(pos.x, pos.y + size);

    float offset = (float)(rand()) / RAND_MAX;
    Color color(255, 255, 255);
    (*va)[*index].color = color;
    (*va)[*index + 1].color = color;
    (*va)[*index + 2].color = color;
    (*va)[*index + 3].color = color;
    *index += 4;
}
float valueAt(int row, int column, bool buffer)
{
    int adjustedRow = loop_mod(row, rowSize);
    int adjustedCol = loop_mod(column, colSize);
    if (!buffer)
        return grid[(adjustedRow * colSize + adjustedCol)];
    else
        return gridBuffer[(adjustedRow * colSize + adjustedCol)];
}
void setCell(int row, int column, float value, bool bufferSet)
{
    int firstVertex = (row * rowSize + column) * 4;
    if (bufferSet)
    {
        gridBuffer[(row * colSize + column)] = value;
    }
    else
    {
        grid[(row * colSize + column)] = value;
        square[firstVertex].color = SQUARE_COL(value);
        square[firstVertex + 1].color = SQUARE_COL(value);
        square[firstVertex + 2].color = SQUARE_COL(value);
        square[firstVertex + 3].color = SQUARE_COL(value);
    }
}
float sigma1(float x, float a)
{
    float value;
    value = 1.0f / (1.0f + expf(-(x - a) * 4 / alpha));
    return value;
}
float sigma2(float x, float a, float b)
{
    float value;
    value = sigma1(x, a) * (1 - sigma1(x, b));
    return value;
}
float sigmaM(float x, float y, float m)
{
    float value;
    value = x * (1 - sigma1(m, 0.5f)) + y * sigma1(m, 0.5f);
    return value;
}
float transitionFunction(float n, float m)
{
    float value;
    value = sigma2(n, sigmaM(birth1, death1, m), sigmaM(birth2, death2, m));
    return value;
}
float cellCheck(int row, int column)
{
    // m =innercircle sum, n= outercircle sum ,M =innercircle count, N= outercircle count
    float n = 0, m = 0, MMax = 0;
    float N = 0, M = 0, NMax = 0;
    for (int r = -(radiusOuter - 1); r <= (radiusOuter - 1); r++)
    {
        for (int c = -(radiusOuter - 1); c <= (radiusOuter - 1); c++)
        {
            int adjustedRow = r + row;
            int adjustedCol = c + column;
            if (r * r + c * c <= radiusInner * radiusInner)
            {
                m += valueAt(adjustedRow, adjustedCol, false);
                M += 1;
            }
            else if (r * r + c * c <= radiusOuter * radiusOuter)
            {
                n += valueAt(adjustedRow, adjustedCol, false);
                N += 1;
            }
        }
    }
    // Normalize n&m
    n /= N;
    m /= M;
    // Blob segment
    return transitionFunction(n, m);
}
void step()
{
    clearBuffer();
    for (int row = 0; row < rowSize; row++)
    {
        for (int column = 0; column < colSize; column++)
        {
            // Proprietary blob
            float value = 2.0f * cellCheck(row, column) - 1;
            setCell(row, column, value, true);
        }
    }
}
void update()
{
    for (int row = 0; row < rowSize; row++)
    {
        for (int column = 0; column < colSize; column++)
        {
            setCell(row, column, clamp((valueAt(row, column, false) + (1 / FPS) * valueAt(row, column, true)), 1, 0), false);
        }
    }
}
void init()
{
    int k = 0;
    PerlinNoise p(PerlinNoise::seed_type(time(0)));
    for (int i = 0; i < colSize; i++)
    {
        for (int j = 0; j < rowSize; j++)
        {
            Color c;
            // float intensity = (float)rand() / (float)RAND_MAX;
            float intensity = p.octave2D_01(j * 0.01f, i * 0.01f, 8);
            grid[i * colSize + j] = intensity;
            c = SQUARE_COL(intensity);
            addQuad(Vector2f((j * SQUARE_SIZE), (i * SQUARE_SIZE)), &square, c, SQUARE_SIZE, &k);
        }
    }
}
void eventLoop()
{
    Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case Event::Closed:
            window.close();
            run = false;
            break;
        case Event::MouseButtonPressed:
            if (window.hasFocus())
            {
                Vector2i mousepos = Mouse::getPosition(window);
                Vector2i index = Vector2i((int)(mousepos.x / SQUARE_SIZE), (int)(mousepos.y / SQUARE_SIZE));
                if (event.mouseButton.button == Mouse::Left)
                {
                    setCell(index.y, index.x, 1.0f, false);
                }
            }
            break;
        case Event::KeyPressed:
            if (event.key.code == Keyboard::Space)
            {
                step();
            }
            if (event.key.code == Keyboard::Enter)
            {
                update();
            }
            if (event.key.code == Keyboard::P)
                pause = !pause;
            if (event.key.code == Keyboard::Up)
            {
                if (timeFrameStep != 1)
                    timeFrameStep += 10;
                else
                    timeFrameStep = 10;
                updateInterval = seconds(1.0f / timeFrameStep);
            }
            if (event.key.code == Keyboard::Down)
            {
                if (timeFrameStep > 10)
                    timeFrameStep -= 10;
                else
                    timeFrameStep = 1;
                updateInterval = seconds(1.0f / timeFrameStep);
            }
            if (event.key.code == Keyboard::Q)
                printGrid(false);
            break;
        }
    }
}
void printGrid(bool buffer)
{
    for (int r = 0; r <= rowSize; r++)
    {
        for (int c = 0; c <= colSize; c++)
        {
            cout << valueAt(r, c, buffer) << ' ';
        }
        cout << endl;
    }
}
int loop_mod(int dividend, int divisor)
{
    int result = ((dividend % divisor) + divisor) % divisor;
    return result;
}
float clamp(float x, float high, float low)
{
    if (x > high)
        x = high;
    if (x < low)
        x = low;
    return x;
}
