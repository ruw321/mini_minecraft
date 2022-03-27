#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context), mp_context(context)
{}

Terrain::~Terrain() {
    m_geomCube.destroyVBOdata();
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
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    for (int i = x; i < x + 16; i++) {
        for (int j = z; j < z + 16; j++) {
            fillColumn(i, j);
        }
    }

    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {

    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);

                chunk->createVBOdata();
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
                shaderProgram->drawInterleave(*chunk);
            }
        }
    }


    /*
    m_geomCube.clearOffsetBuf();
    m_geomCube.clearColorBuf();

    std::vector<glm::vec3> offsets, colors;

    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                // the base code draws chunks by iterating each block
                for(int i = 0; i < 16; ++i) {
                    for(int j = 0; j < 256; ++j) {
                        for(int k = 0; k < 16; ++k) {
                            enum BlockType t = chunk->getBlockAt(i, j, k);

                            if(t != EMPTY) {
                                offsets.push_back(glm::vec3(i+x, j, k+z));
                                switch(t) {
                                    case GRASS:
                                        colors.push_back(glm::vec3(95.f, 159.f, 53.f) / 255.f);
                                        break;
                                    case DIRT:
                                        colors.push_back(glm::vec3(121.f, 85.f, 58.f) / 255.f);
                                        break;
                                    case STONE:
                                        colors.push_back(glm::vec3(0.5f));
                                        break;
                                    case WATER:
                                        colors.push_back(glm::vec3(0.f, 0.f, 0.75f));
                                        break;
                                    case SNOW:
                                        colors.push_back(glm::vec3(1.f, 1.f, 1.f));
                                        break;
                                    case SAND:
                                        colors.push_back(glm::vec3(1.f, 0.95f, 0.9f));
                                        break;
                                    default:
                                        // Other block types are not yet handled, so we default to debug purple
                                        colors.push_back(glm::vec3(1.f, 0.f, 1.f));
                                        break;
                                }
                            }
                        }
                    }
                }

            }
        }
    }

    m_geomCube.createInstancedVBOdata(offsets, colors);
    shaderProgram->drawInstanced(m_geomCube);

    */
}

void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
    m_geomCube.createVBOdata();

}

void Terrain::initializeChunk(Chunk *chunk) {

    int x = chunk->m_pos[0];
    int z = chunk->m_pos[1];

    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        chunk->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        chunk->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        chunk->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        chunk->linkNeighbor(chunkWest, XNEG);
    }


}

void Terrain::updateTerrian(glm::vec3 p) {

    int r = 0; // 5x5
    std::vector<glm::ivec2> currSourrondingZones = getSurroundingZones(p.x, p.z, r);

    std::vector<Chunk*> newChunks;

    for (glm::ivec2 zone : currSourrondingZones) {
        int zone_x = zone[0];
        int zone_z = zone[1];
        if (!hasZoneAt(zone_x, zone_z)) {
            m_generatedTerrain.insert(toKey(zone_x, zone_z));
            for (int x = zone_x; x < zone_x + 64; x += 16)
            {
                for (int z = zone_z; z < zone_z + 64; z += 16)
                {
                    uPtr<Chunk> newChunk = mkU<Chunk>(mp_context, x, z);
                    m_chunks[toKey(x, z)] = move(newChunk);
                    newChunks.push_back(m_chunks[toKey(x, z)].get());
                }
            }
        }
    }




    for (Chunk *chunk : newChunks) {


        this->BlockTypeWorkers.push_back(std::thread(&Terrain::BlockTypeWorker, this, chunk->m_pos));
    }

    for (auto &thread : this->BlockTypeWorkers) {
        if(thread.joinable()) {
            thread.join();
        }
    }

    this->BlockTypeWorkers.clear();

}

void Terrain::fillColumn(int x, int z) {
    int maxHeight = mountain(glm::vec2(x,z));
    for (int k = 0; k <= maxHeight; k++) {
        setBlockAt(x, k, z, BlockType(k, maxHeight, GRASSLAND));
    }
    for (int k = maxHeight + 1; k < 138; k++) {
        setBlockAt(x, k, z, WATER);
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


/*
Milestone 2
*/


void Terrain::BlockTypeWorker(glm::ivec2 m_pos)
{
    int chunk_x = m_pos[0];
    int chunk_z = m_pos[1];

//    std::cout << "x" << chunk->m_pos[0] << "z" << chunk->m_pos[1] << std::endl;

    for (int x = chunk_x; x < chunk_x + 16; ++x) {
        for (int z = chunk_z; z < chunk_z + 16; ++z) {
            fillColumn(x, z);
        }
    }

    // createBlocks(chunkPos.x, chunkPos.y, chunk.get());

    BlockTypeMutex.lock();


    // BlockTypeChunks[toKey(chunk->getChunkPos().x, chunk->getChunkPos().y)] = move(chunk);


    BlockTypeMutex.unlock();
}


