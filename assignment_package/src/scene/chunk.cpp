#include "chunk.h"


Chunk::Chunk(OpenGLContext* context) : Drawable(context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

void Chunk::createVBOdata(){
    std::vector<glm::vec4> posAndColor;
    std::vector<GLuint> idx;

    //front 1, back 2, left 3, right 4, up 5, down 6
    std::vector<std::pair<int, int>> blockFaceNeedRender;
    int currentIdx = 0;

    for (int i = 0; i <65536; i++){
        int zpos = floor(i / (16 * 256.f));
        int ypos = floor((i - zpos * 16 * 256) / 16.f);
        int xpos = i - zpos * 16 * 256 - ypos * 16;
        glm::vec4 currentPos = glm::vec4(xpos, ypos, zpos, 1);
        BlockType currentType = this->m_blocks[xpos + ypos * 16 + zpos * 16 * 256];
        if (currentType == EMPTY){
            continue;
        }


        //front
        int newZpos = zpos + 1;
        if (newZpos < 16 && this->m_blocks[xpos + ypos * 16 +
                newZpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 1});

            glm::vec4 offset[4] = {glm::vec4(0, 0, 1, 0),
                                  glm::vec4(0, 1, 1, 0),
                                  glm::vec4(1, 1, 1, 0),
                                  glm::vec4(1, 0, 1, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }
            }
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 1);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx + 3);
            currentIdx += 4;

        }

        //back
        newZpos = zpos - 1;
        if (newZpos > 0 && this->m_blocks[xpos + ypos * 16 +
                newZpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 2});

            glm::vec4 offset[4] = {glm::vec4(0, 0, 0, 0),
                                  glm::vec4(0, 1, 0, 0),
                                  glm::vec4(1, 1, 0, 0),
                                  glm::vec4(1, 0, 0, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }

            }
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 1);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx + 3);
            currentIdx += 4;
        }

        //left
        int newXpos = xpos + 1;
        if (newXpos < 16 && this->m_blocks[newXpos + ypos * 16 +
                zpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 3});

            glm::vec4 offset[4] = {glm::vec4(0, 0, 0, 0),
                                  glm::vec4(0, 1, 0, 0),
                                  glm::vec4(1, 1, 0, 0),
                                  glm::vec4(1, 0, 0, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }

            }
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 1);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx + 3);
            currentIdx += 4;
        }

        //right
        newXpos = xpos - 1;
        if (newXpos > 0 && this->m_blocks[newXpos + ypos * 16 +
                zpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 4});
            glm::vec4 offset[4] = {glm::vec4(1, 0, 0, 0),
                                  glm::vec4(1, 1, 0, 0),
                                  glm::vec4(1, 1, 1, 0),
                                  glm::vec4(1, 0, 1, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }

            }
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 1);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx + 3);
            currentIdx += 4;
        }

        //up
        int newYpos = ypos + 1;
        if (newYpos < 256 && this->m_blocks[xpos + newYpos * 16 +
                zpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 5});
            glm::vec4 offset[4] = {glm::vec4(0, 0, 1, 0),
                                  glm::vec4(1, 0, 1, 0),
                                  glm::vec4(1, 1, 1, 0),
                                  glm::vec4(0, 1, 1, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }
            }
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 1);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx);
            idx.push_back(currentIdx + 2);
            idx.push_back(currentIdx + 3);
            currentIdx += 4;
        }

        //down
        newYpos = ypos - 1;
        if (newYpos > 0 && this->m_blocks[xpos + newYpos * 16 +
                zpos * 16 * 256] == EMPTY){
            blockFaceNeedRender.push_back({i, 6});

            glm::vec4 offset[4] = {glm::vec4(0, 0, 0, 0),
                                  glm::vec4(1, 0, 0, 0),
                                  glm::vec4(1, 1, 0, 0),
                                  glm::vec4(0, 1, 0, 0)};

            for (int j = 0; j < 4; j++){
                posAndColor.push_back(offset[j] + currentPos);
                switch(currentType) {
                    case GRASS:
                        posAndColor.push_back(glm::vec4
                                              (95.f, 159.f, 53.f, 0)
                                              / 255.f);
                        break;
                    case DIRT:
                        posAndColor.push_back(glm::vec4
                                          (121.f, 85.f, 58.f, 0)
                                          / 255.f);
                        break;
                    case STONE:
                        posAndColor.push_back(glm::vec4(0.5, 0.5, 0.5, 0)
                                              );
                        break;
                    case WATER:
                        posAndColor.push_back(glm::vec4
                                          (0.f, 0.f, 0.75, 0)
                                          );
                        break;
                    case SNOW:
                        posAndColor.push_back(glm::vec4
                                      (0.f, 0.f, 0.f, 0)
                                      );
                        break;
                    case SAND:
                        posAndColor.push_back(glm::vec4
                                  (1.f, 0.95, 0.9, 0)
                                  );
                        break;
                    default:
                        posAndColor.push_back(glm::vec4
                              (1.f, 0.f, 1.f, 0)
                              );
                        break;
                }
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
    m_count = idx.size();

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                             idx.size() * sizeof (GLuint), idx.data(),
                             GL_STATIC_DRAW);

    generateInterleave();
    bindInterleave();
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             posAndColor.size() * sizeof (glm::vec4),
                             posAndColor.data(), GL_STATIC_DRAW);





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
