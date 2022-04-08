#ifndef BIOMES_H
#define BIOMES_H

#include "la.h"

enum BiomeType: unsigned char
{
    GRASSLAND,
    MOUNTAIN,
    SANDLAND,
};

float random1(glm::vec2 p);

glm::vec2 random2(glm::vec2 p);

glm::vec2 rotate(glm::vec2 p, float deg);

float WorleyNoise(glm::vec2 uv);

float surflet(glm::vec2 P, glm::vec2 gridPoint);


float PerlinNoise(glm::vec2 uv);


glm::vec2 hash(glm::vec2 p);

float SimplexNoise(glm::vec2 p);

float bilerpNoise(glm::vec2 uv);

float fbm(glm::vec2 uv);


float ridge(glm::vec2 uv);

float mountain(glm::vec2 uv);

float moisture(glm::vec2 uv);

float temperature(glm::vec2 uv);

float plain(glm::vec2 uv);

float caveCeil(glm::vec2 uv);

float caveFloor(glm::vec2 uv);

#endif
