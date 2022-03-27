#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, SAND
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

struct VertexData{
    glm::vec4 m_pos;
    glm::vec2 m_uv;

    VertexData(glm::vec4 pos, glm::vec2 uv)
        : m_pos(pos), m_uv(uv)
    {}
};

struct BlockFace{
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, glm::vec3 dirV, const VertexData &a,
              const VertexData &b, const VertexData &c,
              const VertexData &d)
        : direction(dir), directionVec(dirV), vertices{a, b, c, d}
    {}
};

#define BLK_UVX * 0.03125
#define BLK_UVY * 0.03125
#define BLK_UV 0.03125

const static std::array<BlockFace, 6> adjacentFaces{
    //+X
    BlockFace(XPOS, glm::vec3(1, 0, 0),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0, BLK_UV))),

    //-X
    BlockFace(XNEG, glm::vec3(-1, 0, 0),
              VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))),

    //+Y
    BlockFace(YPOS, glm::vec3(0, 1, 0),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))),
    //-Y
    BlockFace(YNEG, glm::vec3(0, -1, 0),
              VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, BLK_UV))),
    //+Z
    BlockFace(ZPOS, glm::vec3(0, 0, 1),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, BLK_UV))),

    //-Z
    BlockFace(ZNEG, glm::vec3(0, 0, -1),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0, BLK_UV)))



};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable

struct ChunkVBOData
{
    int64_t key;
    std::vector<glm::vec4> vec_data;
    std::vector<GLuint> vec_id;
};

class Chunk : public Drawable{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

public:

    Chunk(OpenGLContext* context, int x, int z);

    ~Chunk();
    void createVBOdata() override;
    GLenum drawMode() override;
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);


    /*
    Milestone 2
    */
    glm::ivec2 m_pos;
    ChunkVBOData VBOData;
};
