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

   std::vector<glm::vec4> VBOdata_transparent;
   std::vector<GLuint> idx_transparent;

//    std::unordered_map<BlockType, glm::vec4> blockColorMp = {

//        {GRASS, glm::vec4(95.f, 159.f, 53.f, 0) / 255.f},
//        {DIRT, glm::vec4(121.f, 85.f, 58.f, 0) / 255.f},
//        {STONE, glm::vec4(0.5, 0.5, 0.5f, 0)},
//        {WATER, glm::vec4(0.f, 0.f, 0.75f, 0)},
//        {SNOW, glm::vec4(1.f, 1.f, 1.f, 0)},
//        {SAND, glm::vec4(1.f, 0.95, 0.9f, 0)}

//    };

   //front 1, back 2, left 3, right 4, up 5, down 6
//    std::vector<std::pair<int, int>> blockFaceNeedRender;

    int currentIdx = 0;
    int currentIdx_transparent = 0;
    for (int z = 0; z < 16; z++){
        for (int y = 0; y < 256; y++){
            for (int x = 0; x < 16; x++){

                BlockType current = getBlockAt(x, y, z);
                glm::vec4 currentPos = glm::vec4(x, y, z, 0);
                if (current != EMPTY){

                    for (BlockFace neighborFace : adjacentFaces){
                        if (current == BAMBOO &&
                                (neighborFace.direction == YPOS || neighborFace.direction == YNEG)){
                            continue;
                        }
                        if (current == PAD){
                            if (neighborFace.direction != YNEG){
                                continue;
                            }
                        }
                        if (noNormal.find(current) != noNormal.end()) {
                            if (neighborFace.directionVec == glm::vec3(1, 0, 0)){
                                continue;
                            }
                            if (neighborFace.directionVec == glm::vec3(-1, 0, 0)){
                                continue;
                            }
                            if (neighborFace.directionVec == glm::vec3(0, -1, 0)){
                                continue;
                            }
                            if (neighborFace.directionVec == glm::vec3(0, 0, 1)){
                                continue;
                            }
                            if (neighborFace.directionVec == glm::vec3(0, 0, -1)){
                                continue;
                            }
                        }
                        glm::vec3 neighborPos = neighborFace.directionVec
                                + glm::vec3(x, y, z);
//                        std::cout<<getBlockAt(0, 0, 0)<<std::endl;
                        BlockType neighborType;
                        if ((x == 0 || z == 0 || x == 15 || z == 15) and neighborFace.direction != YPOS and neighborFace.direction != YNEG){
                            Chunk* neighborChunk = m_neighbors[neighborFace.direction];
                            if (neighborChunk == nullptr){
                                neighborType = EMPTY;
//                                continue;
                            }
                            else{
                                if (neighborFace.direction == XNEG && x == 0){
                                    neighborType = neighborChunk->getBlockAt(int(15),
                                                                             int(neighborPos.y),
                                                                             int(neighborPos.z));
                                } else if (neighborFace.direction == XPOS && x == 15){
                                    neighborType = neighborChunk->getBlockAt(int(0),
                                                                             int(neighborPos.y),
                                                                             int(neighborPos.z));

                                } else if (neighborFace.direction == ZPOS && z == 15){
                                    neighborType = neighborChunk->getBlockAt(int(neighborPos.x),
                                                                             int(neighborPos.y),
                                                                             int(0));
                                } else if (neighborFace.direction == ZNEG && z == 0){
                                    neighborType = neighborChunk->getBlockAt(int(neighborPos.x),
                                                                             int(neighborPos.y),
                                                                             int(15));
                                }
                                else{
                                    neighborType = getBlockAt(int(neighborPos.x),
                                                                    int(neighborPos.y),
                                                                    int(neighborPos.z));
                                }
                            }


                        }else{
                            neighborType = getBlockAt(int(neighborPos.x),
                                                            int(neighborPos.y),
                                                            int(neighborPos.z));
                        }

                        if (transparentBlock.find(current) == transparentBlock.end()) {
                            if (neighborType == EMPTY || transparentBlock.find(neighborType) != transparentBlock.end()){
                                for (int i = 0; i < 4; i++){

                                    VBOdata.push_back(neighborFace.vertices[i].m_pos + currentPos);
                                    if (usingBetterTexture.find(current) == usingBetterTexture.end()){
                                        VBOdata.push_back(glm::vec4(neighborFace.vertices[i].m_uv +
                                                                blockFaceUV[current][neighborFace.direction], 0, 0));
                                    }else{
                                        VBOdata.push_back(glm::vec4(neighborFace.vertices[i].m_uv +
                                                                blockFaceUV[current][neighborFace.direction], 0.2, 0));
                                    }

                                    VBOdata.push_back(glm::vec4(neighborFace.directionVec, 0.5));

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
                       else  { // Water
                            if (current != WATER || neighborType == EMPTY){
                                if (diagnalBlock.find(current) == diagnalBlock.end()){
                                    float offset = 0;
                                    if (current == CACTUS || current == BAMBOO){
                                        offset = 0.2;
                                    }
                                    else{
                                        offset = 0;
                                    }
                                    for (int i = 0; i < 4; i++){

                                        VBOdata_transparent.push_back(neighborFace.vertices[i].m_pos + currentPos+ glm::vec4(-neighborFace.directionVec, 0) * offset);
                                        if (usingBetterTexture.find(current) == usingBetterTexture.end()){
                                            VBOdata_transparent.push_back(glm::vec4(neighborFace.vertices[i].m_uv +
                                                                    blockFaceUV[current][neighborFace.direction], 0, 0));
                                        }else{

                                            VBOdata_transparent.push_back(glm::vec4(neighborFace.vertices[i].m_uv +
                                                                    blockFaceUV[current][neighborFace.direction], 0.2, 0));
                                        }
                                        if (noNormal.find(current) != noNormal.end()){
                                            VBOdata_transparent.push_back(glm::vec4(neighborFace.directionVec, 0));
                                        }else{
                                            VBOdata_transparent.push_back(glm::vec4(neighborFace.directionVec, 0.5));
                                        }

                                    }
                                }else{
                                    int diagIdx;
                                    if (neighborFace.direction == XPOS){
                                        diagIdx = 0;
                                    }else if (neighborFace.direction == XNEG){
                                        diagIdx = 1;
                                    }else{
                                        continue;
                                    }
                                    for (int i = 0; i < 4; i++){
                                        VBOdata_transparent.push_back(diagnalFaces[diagIdx].vertices[i].m_pos + currentPos);
                                        if (usingBetterTexture.find(current) == usingBetterTexture.end()){
                                            VBOdata_transparent.push_back(glm::vec4(diagnalFaces[diagIdx].vertices[i].m_uv +
                                                                    blockFaceUV[current][diagnalFaces[diagIdx].direction], 0, 0));
                                        }else{

                                            VBOdata_transparent.push_back(glm::vec4(diagnalFaces[diagIdx].vertices[i].m_uv +
                                                                    blockFaceUV[current][diagnalFaces[diagIdx].direction], 0.2, 0));
                                        }
                                        if (noNormal.find(current) != noNormal.end()){
                                            VBOdata_transparent.push_back(glm::vec4(diagnalFaces[diagIdx].directionVec, 0));
                                        }else{
                                            VBOdata_transparent.push_back(glm::vec4(diagnalFaces[diagIdx].directionVec, 0.5));
                                        }
                                    }
                                }
                                idx_transparent.push_back(currentIdx_transparent);
                                idx_transparent.push_back(currentIdx_transparent + 1);
                                idx_transparent.push_back(currentIdx_transparent + 2);
                                idx_transparent.push_back(currentIdx_transparent);
                                idx_transparent.push_back(currentIdx_transparent + 2);
                                idx_transparent.push_back(currentIdx_transparent + 3);
                                currentIdx_transparent += 4;

                            }
                       }

                    }

                }
            }
        }
    }





   this->m_count = idx.size();
   this->m_count_transparent = idx_transparent.size();

   this->m_VBOdata.idx = idx;
   this->m_VBOdata.data = VBOdata;

   this->m_VBOdata_transparent.idx = idx_transparent;
   this->m_VBOdata_transparent.data = VBOdata_transparent;


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



   generateIdx_transparent();
   bindIdx_transparent();
   mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                            this->m_VBOdata_transparent.idx.size() * sizeof (GLuint),
                            this->m_VBOdata_transparent.idx.data(),
                            GL_STATIC_DRAW);

   generateInterleave_transparent();
   bindInterleave_transparent();
   mp_context->glBufferData(GL_ARRAY_BUFFER,
                            this->m_VBOdata_transparent.data.size() * sizeof (glm::vec4),
                            this->m_VBOdata_transparent.data.data(),
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
   if (y < 0 || y > 255){
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
