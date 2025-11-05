#include "Aquarium.h"
#include <cstdlib>


string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::ColorfulFish:
            return "ColorfulFish";
        case AquariumCreatureType::FastFish:
            return "FastFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    this->move();
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;

    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    } else {
        ofSetColor(currentColor); 
    }

    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }

    ofSetColor(ofColor::white); // Reset color
}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

// ColorfulFish - behaves like normal fish
ColorfulFish::ColorfulFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_creatureType = AquariumCreatureType::ColorfulFish;
    m_value = 3; // Worth 3 points
    setCollisionRadius(70); // Larger to match 140x140 sprite size
    // Randomize wobble so fish don't sync
    m_wobblePhase = ofRandom(0, TWO_PI);
    m_wobbleSpeed = ofRandom(0.6f, 1.4f);
    m_wobbleAngleAmp = ofRandom(0.08f, 0.16f); // ~5-9 degrees
}

void ColorfulFish::move() {
    // Smooth curvy movement: gently bend direction over time
    float t = ofGetElapsedTimef();
    float bend = sinf(t * m_wobbleSpeed + m_wobblePhase) * m_wobbleAngleAmp; // radians
    // rotate current direction by small bend
    float c = cosf(bend);
    float s = sinf(bend);
    float ndx = m_dx * c - m_dy * s;
    float ndy = m_dx * s + m_dy * c;
    m_dx = ndx; m_dy = ndy;
    normalize();

    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void ColorfulFish::draw() const {
    ofLogVerbose() << "ColorfulFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}

// FastFish - behaves like BiggerFish (slower, boss-like)
FastFish::FastFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    
    setCollisionRadius(30); // Small collision radius
    m_value = 10; // High value - boss fish
    m_creatureType = AquariumCreatureType::FastFish;
}

void FastFish::move() {
    // If we have a target, steer smoothly towards it
    if (m_hasTarget) {
        float vx = m_targetX - m_x;
        float vy = m_targetY - m_y;
        float len = sqrtf(vx * vx + vy * vy);
        if (len > 0.0001f) {
            vx /= len; vy /= len;
            const float steer = 0.12f; // turning responsiveness
            m_dx = (1.0f - steer) * m_dx + steer * vx;
            m_dy = (1.0f - steer) * m_dy + steer * vy;
            normalize();
        }
    }

    // Move a bit faster than before
    const float homingSpeedFactor = 0.95f; // slightly faster; still under player speed
    m_x += m_dx * (m_speed * homingSpeedFactor);
    m_y += m_dy * (m_speed * homingSpeedFactor);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void FastFish::draw() const {
    ofLogVerbose() << "FastFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}



// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_colorful_fish = std::make_shared<GameSprite>("sprites/colorfulFish.png", 140, 140);
    this->m_fast_fish = std::make_shared<GameSprite>("sprites/fastFish.png", 60, 60);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
        case AquariumCreatureType::ColorfulFish:
            return std::make_shared<GameSprite>(*this->m_colorful_fish);
        case AquariumCreatureType::FastFish:
            return std::make_shared<GameSprite>(*this->m_fast_fish);
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);
        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        // If this is a FastFish, choose the nearest target (other fish or the player if available)
        if (auto npc = std::dynamic_pointer_cast<NPCreature>(creature)) {
            if (npc->GetType() == AquariumCreatureType::FastFish) {
                if (auto ff = std::dynamic_pointer_cast<FastFish>(creature)) {
                    ofVec2f bestPos = m_hasPlayerTarget ? m_playerTarget : ofVec2f(ff->getX(), ff->getY());
                    float bestDist2 = m_hasPlayerTarget ? (m_playerTarget.x - ff->getX()) * (m_playerTarget.x - ff->getX()) +
                                                          (m_playerTarget.y - ff->getY()) * (m_playerTarget.y - ff->getY())
                                                       : std::numeric_limits<float>::max();
                    // consider all other creatures as potential prey
                    for (auto& other : m_creatures) {
                        if (other == creature) continue;
                        auto otherNpc = std::dynamic_pointer_cast<NPCreature>(other);
                        if (!otherNpc) continue;
                        float dx = other->getX() - ff->getX();
                        float dy = other->getY() - ff->getY();
                        float d2 = dx * dx + dy * dy;
                        if (d2 < bestDist2) {
                            bestDist2 = d2;
                            bestPos.set(other->getX(), other->getY());
                        }
                    }
                    ff->setTarget(bestPos.x, bestPos.y);
                }
            }
        }
        creature->move();
    }
    
    // FastFish can eat smaller fish (visual effect only)
    this->HandleFastFishEating();
    
    this->Repopulate();
}

void Aquarium::HandleFastFishEating() {
    std::vector<std::shared_ptr<Creature>> toRemove;
    
    // Find all FastFish
    for (auto& creature : m_creatures) {
        auto npcCreature = std::dynamic_pointer_cast<NPCreature>(creature);
        if (npcCreature && npcCreature->GetType() == AquariumCreatureType::FastFish) {
            // Check if FastFish collides with smaller fish
            for (auto& prey : m_creatures) {
                if (prey == creature) continue; // Skip self
                
                auto preyNpc = std::dynamic_pointer_cast<NPCreature>(prey);
                if (preyNpc) {
                    AquariumCreatureType preyType = preyNpc->GetType();
                    // FastFish can eat NPCreature, ColorfulFish, and BiggerFish (not other FastFish)
                    if (preyType == AquariumCreatureType::NPCreature || 
                        preyType == AquariumCreatureType::ColorfulFish ||
                        preyType == AquariumCreatureType::BiggerFish) {
                        
                        if (checkCollision(creature, prey)) {
                            toRemove.push_back(prey);
                            // Store position for particle effect
                            m_fastFishEatPositions.push_back(ofVec2f(prey->getX(), prey->getY()));
                            ofLogNotice() << "FastFish ate a " << AquariumCreatureTypeToString(preyType) << "!" << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    // Remove eaten fish and decrement level population without adding to score
    for (auto& fish : toRemove) {
        auto it = std::find(m_creatures.begin(), m_creatures.end(), fish);
        if (it != m_creatures.end()) {
            // Update level population counts so Repopulate can spawn replacements
            auto preyNpc = std::dynamic_pointer_cast<NPCreature>(fish);
            if (preyNpc && !m_aquariumlevels.empty()) {
                int idx = this->currentLevel % this->m_aquariumlevels.size();
                this->m_aquariumlevels.at(idx)->ConsumePopulation(preyNpc->GetType(), 0 /*no score*/);
            }
            m_creatures.erase(it);
        }
    }
}

std::vector<ofVec2f> Aquarium::GetAndClearFastFishEatPositions() {
    std::vector<ofVec2f> positions = m_fastFishEatPositions;
    m_fastFishEatPositions.clear();
    return positions;
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::ColorfulFish:
            this->addCreature(std::make_shared<ColorfulFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::ColorfulFish)));
            break;
        case AquariumCreatureType::FastFish:
            this->addCreature(std::make_shared<FastFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::FastFish)));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

//  Imlementation of the AquariumScene

void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;

    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if(event->creatureB != nullptr){
                event->print();
                
                // Check if it's a ColorfulFish - always edible, no damage
                auto npcCreature = std::static_pointer_cast<NPCreature>(event->creatureB);
                bool isColorfulFish = (npcCreature->GetType() == AquariumCreatureType::ColorfulFish);
                
                // ColorfulFish are always safe to eat, others require power check
                if(!isColorfulFish && this->m_player->getPower() < event->creatureB->getValue()){
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    this->m_player->loseLife(3*60); // 3 frames debounce, 3 seconds at 60fps
                    if(this->m_player->getLives() <= 0){
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                }
                else{
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());
                    if (this->m_player->getScore() % 25 == 0){
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                    
                }
                
                

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        // Update player position so FastFish can also target the player
        this->m_aquarium->SetPlayerTarget(this->m_player->getX(), this->m_player->getY());
        this->m_aquarium->update();
    }

}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

// Refactored Repopulate - single implementation for all levels
std::vector<AquariumCreatureType> AquariumLevel::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta > 0){
            ofLogVerbose() << "Repopulating " << delta << " creatures of type " 
                          << AquariumCreatureTypeToString(node->creatureType) << endl;
            for(int i = 0; i < delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}
