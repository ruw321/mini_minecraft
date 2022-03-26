#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

// Update the Player's velocity and acceleration based on the
// state of the inputs.
void Player::processInputs(InputBundle &inputs) {
    // this variable makes sure to halt the movement
    bool nothingClicked = true;
    if (inputs.flight_mode) {
        if (inputs.wPressed) {
            m_acceleration += m_forward;
        } else if (inputs.sPressed) {
            m_acceleration += -m_forward;
        } else if (inputs.dPressed) {
            m_acceleration += m_right;
        } else if (inputs.aPressed) {
            m_acceleration += -m_right;
        } else if (inputs.ePressed) {
            m_acceleration += m_up;
        } else if (inputs.qPressed) {
            m_acceleration += -m_up;
        } else {
            m_velocity = glm::vec3(0.f);
            m_acceleration = glm::vec3(0.f);
        }
    } else {
        if (inputs.wPressed) {
            m_acceleration += glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
            nothingClicked = false;
        }
        if (inputs.sPressed) {
            m_acceleration += -glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z));
            nothingClicked = false;
        }
        if (inputs.aPressed) {
            m_acceleration += -glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
            nothingClicked = false;
        }
        if (inputs.dPressed) {
            m_acceleration += glm::normalize(glm::vec3(m_right.x, 0, m_right.z));
            nothingClicked = false;
        }
        if (inputs.spacePressed) {
            // jumping doesn't work yet
            std::cout << "jump" << std::endl;
            m_velocity = 20.f * m_up;
            nothingClicked = false;
            std::cout << m_velocity.y << std::endl;
        }
        if (nothingClicked){
            m_velocity = glm::vec3(0.f);
            m_acceleration = glm::vec3(0.f);
        }
    }
}

// Update the Player's position based on its acceleration
// and velocity, and also perform collision detection.
void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &inputs) {
    // the player's velocity is reduced to less than 100% of its current value every frame (simulates friction + drag) before acceleration is added to it.
    m_velocity *= 0.9f;
    m_velocity += m_acceleration * dT;
    glm::vec3 gravity = glm::vec3(0.0f, -9.8f, 0.0f);
    glm::vec3 rayDirection = m_velocity * dT;
    std::cout << "computePhysics is called" << std::endl;

    // only perform collision detection when you are not in flight mode
    if (!inputs.flight_mode) {
        if (!isOnGround(terrain, inputs)) {
            std::cout << "function is called" << std::endl;
            // drop to the ground
            m_acceleration = gravity;
            m_velocity = m_acceleration * dT;
        } else if (isOnGround(terrain, inputs) && !inputs.spacePressed){
            std::cout << "not supposed to reach here" << std::endl;
            m_acceleration[1] = 0.f;
            m_velocity[1] = 0.f;
        }
        rayDirection = m_velocity * dT;
        detectCollision(&rayDirection, terrain);
    }
    std::cout << rayDirection.y << std::endl;
    this->moveAlongVector(rayDirection);
}

bool Player::isOnGround(const Terrain &terrain, InputBundle &inputs) {
    // player position is 0.5 away from the vertex
    // chose bottomleft because I wanted to add 1 to get all the other three vertices
    glm::vec3 bottomLeft = this->m_position - glm::vec3(0.5f, 0, 0.5f);
    // traverse the four bottom vertices
    for (int x = 0; x < 2; x++) {
        for (int z = 0; z < 2; z++) {
            glm::vec3 vertexPos = glm::vec3(floor(bottomLeft.x) + x, floor(bottomLeft.y - 0.005f),
                    floor(bottomLeft.z) + z);
            // as long as one of the vertex is on a block that is not empty
            // player is on the ground
            if (terrain.getBlockAt(vertexPos) != EMPTY) {
                if (!inputs.spacePressed) {
                    std::cout << "should reach here" << std::endl;
                    m_acceleration.y = 0.f;
                    m_velocity.y = 0.f;
                }
                return true;
            }
        }
    }
    return false;
}

// copied directly from the slide
bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    // TA said I need to offset by a very small number to avoid bugs
    *out_dist = glm::min(maxLen, curr_t) - 0.00001;
    return false;
}


void Player::detectCollision(glm::vec3 *rayDirection, const Terrain &terrain) {
    glm::vec3 bottomLeftVertex = this->m_position - glm::vec3(0.5f, 0.f, 0.5f);
    glm::ivec3 out_blockHit = glm::ivec3();
    float out_dist = 0.f;

    for (int x = 0; x <= 1; x++) {
        for (int z = 0; z >= -1; z--) {
            for (int y = 0; y <= 2; y++) {
                glm::vec3 rayOrigin = bottomLeftVertex + glm::vec3(x, y, z);
                if (gridMarch(rayOrigin, *rayDirection, terrain, &out_dist, &out_blockHit)) {
                    std::cout << "collided..." << std::endl;
                    float distance = glm::min(out_dist - 0.005f, glm::abs(glm::length(this->m_position - glm::vec3(out_blockHit))));
                    *rayDirection = distance * glm::normalize(*rayDirection);
                }
            }
        }
    }
}

// remove the block currently overlapping the center of the screen when the left mouse button is pressed
void Player::removeBlock(Terrain *terrain) {
    glm::vec3 rayOrigin = m_camera.mcr_position;
    // block is within 3 units of distance from the camera
    glm::vec3 rayDirection = 3.f * glm::normalize(this->m_forward);
    glm::ivec3 outBlockHit = glm::ivec3();
    float outDist = 0.f;
    // if we found a block
    if (gridMarch(rayOrigin, rayDirection, *terrain, &outDist, &outBlockHit)) {
        terrain->setBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z, EMPTY);
    }
}

void Player::placeBlock(Terrain *terrain) {
    glm::vec3 rayOrigin = m_camera.mcr_position;
    // block is within 3 units of distance from the camera
    glm::vec3 rayDirection = 3.f * glm::normalize(this->m_forward);
    glm::ivec3 outBlockHit = glm::ivec3();
    float outDist = 0.f;
    // if there is no block
    if (!gridMarch(rayOrigin, rayDirection, *terrain, &outDist, &outBlockHit)) {
        outBlockHit = this->m_camera.mcr_position + 3.f * glm::normalize(this->m_forward);
        // now we are placing a stone, we can change it later if we want to
        terrain->setBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z, STONE);
    }
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
