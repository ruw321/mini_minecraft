#include "terrain.h"
#include "cube.h"
#include "biomes.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(),
      m_generatedTerrain(),
//      m_geomCube(context),
      mp_context(context)
{}

Terrain::~Terrain() {
//    m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}

bool Terrain::hasZoneAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return m_generatedTerrain.find(toKey(64 * xFloor, 64 * zFloor)) != m_generatedTerrain.end();
}

uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, enum BlockType t)
{

    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::setBlockAt(glm::vec3 p, enum BlockType t) {
    setBlockAt(glm::round(p.x), glm::round(p.y), glm::round(p.z), t);
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {

    Chunk *chunk = newChunkBuffer[toKey(x, z)].get();

    if (hasChunkAt(x, z + 16)) {
        auto &chunkNorth = getChunkAt(x, z + 16);
        chunk->linkNeighbor(chunkNorth, ZPOS);
    }
    if (hasChunkAt(x, z - 16)) {
        auto &chunkSouth = getChunkAt(x, z - 16);
        chunk->linkNeighbor(chunkSouth, ZNEG);
    }
    if (hasChunkAt(x + 16, z)) {
        auto &chunkEast = getChunkAt(x + 16, z);
        chunk->linkNeighbor(chunkEast, XPOS);
    }
    if (hasChunkAt(x - 16, z)) {
        auto &chunkWest = getChunkAt(x - 16, z);
        chunk->linkNeighbor(chunkWest, XNEG);
    }

    if (hasNewChunkAt(x, z + 16)) {
        auto &chunkNorth = getNewChunkAt(x, z + 16);
        chunk->linkNeighbor(chunkNorth, ZPOS);
    }
    if (hasNewChunkAt(x, z - 16)) {
        auto &chunkSouth = getNewChunkAt(x, z - 16);
        chunk->linkNeighbor(chunkSouth, ZNEG);
    }
    if (hasNewChunkAt(x + 16, z)) {
        auto &chunkEast = getNewChunkAt(x + 16, z);
        chunk->linkNeighbor(chunkEast, XPOS);
    }
    if (hasNewChunkAt(x - 16, z)) {
        auto &chunkWest = getNewChunkAt(x - 16, z);
        chunk->linkNeighbor(chunkWest, XNEG);
    }


    return chunk;

}



// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {

//    chunksWithBlockDataMutex.lock();

    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
                shaderProgram->drawInterleave(*chunk, 0);
            }
        }
    }
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
                shaderProgram->drawInterleave_transparent(*chunk, 0);
            }
        }
    }

//    chunksWithBlockDataMutex.unlock();

}

void Terrain::CreateTestScene()
{

}




void Terrain::fillColumn(Chunk *chunk, int x, int z) {

    int map_x = chunk->m_pos[0] + x;
    int map_z = chunk->m_pos[1] + z;

    int maxHeight = mountain(glm::vec2(map_x, map_z));
    float moist = moisture(glm::vec2(map_x, map_z) / 500.f);

    int caveCeilHeight = caveCeil(glm::vec2(map_x, map_z));
    int caveFloorHeight = caveCeil(glm::vec2(map_x, map_z));

    // Cave
    for (int k = 0; k < 64; k++) { // start from the ground
        chunk->setBlockAt(x, k, z, BEDROCK);
    }

    for (int k = 64; k < 64 + caveFloorHeight; k++) {
        chunk->setBlockAt(x, k, z, STONE);
    }

    for (int k = 64; k < 75; k++) {
        if (chunk->getBlockAt(x, k, z) == EMPTY) {
            chunk->setBlockAt(x, k, z, LAVA);
        }
    }

    for (int k = 127; k > 127 - caveCeilHeight; k--) {
        if ( k <= 64 + caveFloorHeight) {
            break;
        }
        chunk->setBlockAt(x, k, z, STONE);
    }


    for (int k = 128; k <= maxHeight; k++) {
        if (moist > 0){
            chunk->setBlockAt(x, k, z, BlockType(k, maxHeight, GRASSLAND));
        }else{
            chunk->setBlockAt(x, k, z, BlockType(k, maxHeight, SANDLAND));
        }

    }

    for (int k = maxHeight+1; k < 138; k++) {
        if (moist > 0){
            chunk->setBlockAt(x, k, z, WATER);
        }else{
            chunk->setBlockAt(x, k, z, SAND);
        }
    }

    if (map_x > 44 && map_x < 52 && map_z > 44 && map_z < 52) {
        for (int k = 100; k < 150; k++) {
            chunk->setBlockAt(x, k, z, EMPTY);
        }
    }
}


// (2r+1)x(2r+1) surrounding zone
std::vector<glm::ivec2> Terrain::getSurroundingZones(int x, int z, int r)
{
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    int cur_x = 64 * xFloor;
    int cur_z = 64 * zFloor;

    std::vector<glm::ivec2> surroundingZones = {};
    int radius = r * 64;
    for (int i = cur_x - radius; i <= cur_x + radius; i += 64)
    {
        for (int j = cur_z - radius; j <= cur_z + radius; j+= 64)
        {
            surroundingZones.push_back(glm::ivec2(i, j));
        }
    }
    return surroundingZones;
}

BlockType Terrain::BlockType(int height, int maxHeight, enum BiomeType biome) {
    if (biome == SANDLAND){
        if (height <= 148) {
            return SAND;
        }
        else {
            if (maxHeight <= 185) {
                if (height == maxHeight) {
                    return REDSTONE;
                } else {
                    return REDSTONE;
                }
            }


        }
    }
    else if (biome == GRASSLAND){
        if (height <= 128) {
            return STONE;
        }
        else {
            if (maxHeight <= 145) {
                if (height == maxHeight) {
                    return GRASS;
                } else {
                    return DIRT;
                }
            }
            else {
                if (maxHeight <= 170) {
                    return STONE;
                } else {
                    if (height == maxHeight) {
                        return SNOW;
                    } else {
                        return STONE;
                    }
                }
            }
        }
    }
    else {
        return EMPTY;
    }
}


/*
Milestone 2
*/


bool Terrain::hasNewChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return newChunkBuffer.find(toKey(16 * xFloor, 16 * zFloor)) != newChunkBuffer.end();
}


uPtr<Chunk>& Terrain::getNewChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return newChunkBuffer[toKey(16 * xFloor, 16 * zFloor)];
}


void Terrain::updateTerrian(glm::vec3 currPlayerPos) {

    int r = 2; // 5x5
    std::vector<glm::ivec2> newZones = getSurroundingZones(currPlayerPos.x, currPlayerPos.z, r);

    for (glm::ivec2 newZone : newZones) {
        if (!hasZoneAt(newZone[0], newZone[1])) {
            m_generatedTerrain.insert(toKey(newZone.x, newZone.y));
            for (int x = 0; x < 64; x += 16) {
                for (int z = 0; z < 64; z += 16) {
                    newChunkBuffer[toKey(newZone.x + x, newZone.y + z)] = mkU<Chunk>(mp_context, newZone[0]+x, newZone[1]+z);
                }
            }
        }
    }


    for (auto & [key, chunk]: newChunkBuffer) {
        // link to neighbour
        instantiateChunkAt(chunk->m_pos[0], chunk->m_pos[1]);
    }

    for (auto & [key, chunk]: newChunkBuffer) {
        BlockTypeWorkers.push_back(std::thread(&Terrain::BlockTypeWorker, this, move(chunk)));
    }

    newChunkBuffer.clear(); // all uPtrs have been moved

    BlockTypeBufferMutex.lock();
    for (auto & [key, chunk] : BlockTypeBuffer) {
        VBOWorkers.push_back(std::thread(&Terrain::VBOWorker, this, move(chunk)));
    }
    BlockTypeBuffer.clear(); // all uPtrs have been moved
    BlockTypeBufferMutex.unlock();

    VBOdataBufferMutex.lock();
    for (auto & [key, chunk] : VBOdataBuffer) {
        chunk->sendVBOdata();
        // after vbo being sent to GPU, it should be considered as created, therefore store it in m_chunks
        m_chunks[key] = move(chunk);
    }
    VBOdataBuffer.clear(); // all uPtrs have been moved
    VBOdataBufferMutex.unlock();

}


void Terrain::BlockTypeWorker(uPtr<Chunk> chunk) {

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {

            fillColumn(chunk.get(), x, z);
        }
    }
    BlockTypeBufferMutex.lock();
    BlockTypeBuffer[toKey(chunk->m_pos[0], chunk->m_pos[1])] = move(chunk);
    BlockTypeBufferMutex.unlock();
}



void Terrain::VBOWorker(uPtr<Chunk> chunk) {
    chunk->createVBOdata();

    VBOdataBufferMutex.lock();
    VBOdataBuffer[toKey(chunk->m_pos[0], chunk->m_pos[1])] = move(chunk);
    VBOdataBufferMutex.unlock();
}




void Terrain::end() {
    for (auto &thread: BlockTypeWorkers) {
        thread.join();
    }
    for (auto &thread: VBOWorkers) {
        thread.join();
    }
}


