#include "chunk.h"
#include <iostream>


Chunk::Chunk(OpenGLContext* context, int x, int z)
      : Drawable(context),
        m_blocks(),
        m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
        m_pos(glm::ivec2(x,z))

{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

Chunk::~Chunk(){}

void Chunk::createVBOdata() {
    std::vector<glm::vec4> VBOdata;
    std::vector<GLuint> idx;

    std::unordered_map<BlockType, glm::vec4> blockColorMp = {

        {GRASS, glm::vec4(95.f, 159.f, 53.f, 0) / 255.f},
        {DIRT, glm::vec4(121.f, 85.f, 58.f, 0) / 255.f},
        {STONE, glm::vec4(0.5, 0.5, 0.5f, 0)},
        {WATER, glm::vec4(0.f, 0.f, 0.75f, 0)},
        {SNOW, glm::vec4(1.f, 1.f, 1.f, 0)},
        {SAND, glm::vec4(1.f, 0.95, 0.9f, 0)}

    };

    //front 1, back 2, left 3, right 4, up 5, down 6
//    std::vector<std::pair<int, int>> blockFaceNeedRender;
    int currentIdx = 0;
    for (int z = 0; z < 16; z++){
        for (int y = 0; y < 256; y++){
            for (int x = 0; x < 16; x++){
                BlockType current = getBlockAt(x, y, z);
                glm::vec4 currentPos = glm::vec4(x, y, z, 0);
                if (current != EMPTY){
                    for (BlockFace neighborFace : adjacentFaces){

                        glm::vec3 neighborPos = neighborFace.directionVec
                                + glm::vec3(x, y, z);

                        BlockType neighborType = getBlockAt(int(neighborPos.x),
                                                            int(neighborPos.y),
                                                            int(neighborPos.z));
                        if (neighborType == EMPTY){
                            for (int i = 0; i < 4; i++){

                                VBOdata.push_back(neighborFace.vertices[i].m_pos + currentPos);
                                VBOdata.push_back(blockColorMp[current]);
                                VBOdata.push_back(glm::vec4(neighborFace.directionVec, 0));

                            }
                            idx.push_back(currentIdx);
                            idx.push_back(currentIdx + 1);
                            idx.push_back(currentIdx + 2);
                            idx.push_back(currentIdx);
                            idx.push_back(currentIdx + 2);
                            idx.push_back(currentIdx + 3);
                            currentIdx += 4;
                        }

                    }
                }
            }
        }
    }

    this->m_count = idx.size();

    this->m_VBOdata.idx = idx;
    this->m_VBOdata.data = VBOdata;
}

void Chunk::sendVBOdata() {



    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             this->m_VBOdata.idx.size() * sizeof (GLuint),
                             this->m_VBOdata.idx.data(),
                             GL_STATIC_DRAW);

    generateInterleave();
    bindInterleave();
    mp_context->glBufferData(GL_ARRAY_BUFFER,

                             this->m_VBOdata.data.size() * sizeof (glm::vec4),
                             this->m_VBOdata.data.data(),
                             GL_STATIC_DRAW);

}


GLenum Chunk::drawMode(){
    return GL_TRIANGLES;
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if (x < 0 || x > 15 || y < 0 || y > 255 || z < 0 || z > 15) {
        return EMPTY;
    }
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}
