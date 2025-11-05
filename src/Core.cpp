#include "Core.h"


// Creature Inherited Base Behavior
void Creature::setBounds(int w, int h) { m_width = w; m_height = h; }
void Creature::normalize() {
    float length = std::sqrt(m_dx * m_dx + m_dy * m_dy);
    if (length != 0) {
        m_dx /= length;
        m_dy /= length;
    }
}

void Creature::bounce() {
    // Prevent creatures from leaving the aquarium bounds and make them bounce off the walls.
    // Use collision radius as a margin so sprites don't get stuck halfway off-screen.
    if (m_width <= 0 || m_height <= 0) return; // bounds not set

    float margin = m_collisionRadius;
    float minX = margin;
    float maxX = m_width - margin;
    float minY = margin;
    float maxY = m_height - margin;

    bool bounced = false;

    if (m_x < minX) {
        m_x = minX;
        m_dx = -m_dx;
        bounced = true;
    } else if (m_x > maxX) {
        m_x = maxX;
        m_dx = -m_dx;
        bounced = true;
    }

    if (m_y < minY) {
        m_y = minY;
        m_dy = -m_dy;
        bounced = true;
    } else if (m_y > maxY) {
        m_y = maxY;
        m_dy = -m_dy;
        bounced = true;
    }

    // If we bounced, normalize direction again to keep movement stable
    if (bounced) {
        normalize();
    }

    // If for some reason direction becomes zero, pick a small random direction
    if (m_dx == 0.0f && m_dy == 0.0f) {
        m_dx = 1.0f;
        m_dy = 0.0f;
        normalize();
    }

    // Update sprite flip state for horizontal direction
    if (m_dx < 0) setFlipped(true);
    else setFlipped(false);
}


void GameEvent::print() const {
        
        switch (type) {
            case GameEventType::NONE:
                ofLogVerbose() << "No event." << std::endl;
                break;
            case GameEventType::COLLISION:
                ofLogVerbose() << "Collision event between creatures at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ") and ("
                << creatureB->getX() << ", " << creatureB->getY() << ")." << std::endl;
                break;
            case GameEventType::CREATURE_ADDED:
                ofLogVerbose() << "Creature added at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ")." << std::endl;
                break;
            case GameEventType::CREATURE_REMOVED:
                ofLogVerbose() << "Creature removed at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ")." << std::endl;
                break;
            case GameEventType::GAME_OVER:
                ofLogVerbose() << "Game Over event." << std::endl;
                break;
            case GameEventType::NEW_LEVEL:
                ofLogVerbose() << "New Game level" << std::endl;
            default:
                ofLogVerbose() << "Unknown event type." << std::endl;
                break;
        }
};

// collision detection between two creatures
bool checkCollision(std::shared_ptr<Creature> a, std::shared_ptr<Creature> b) {
    if (!a || !b) return false;

    // Use circle-based collision detection based on each creature's collision radius
    float dx = a->getX() - b->getX();
    float dy = a->getY() - b->getY();
    float distSq = dx * dx + dy * dy;
    float radiusSum = a->getCollisionRadius() + b->getCollisionRadius();
    return distSq <= (radiusSum * radiusSum);
};


string GameSceneKindToString(GameSceneKind t){
    switch(t)
    {
        case GameSceneKind::GAME_INTRO: return "GAME_INTRO";
        case GameSceneKind::AQUARIUM_GAME: return "AQUARIUM_GAME";
        case GameSceneKind::GAME_OVER: return "GAME_OVER";
    };
    return "UNKNOWN"; // Default case
};

std::shared_ptr<GameScene> GameSceneManager::GetScene(string name){
    if(!this->HasScenes()){return nullptr;}
    for(std::shared_ptr<GameScene> scene : this->m_scenes){
        if(scene->GetName() == name){
            return scene;
        }
    }
    return nullptr;
}

void GameSceneManager::Transition(string name){
    if(!this->HasScenes()){return;} // no need to do anything if nothing inside
    std::shared_ptr<GameScene> newScene = this->GetScene(name);
    if(newScene == nullptr){return;} // i dont have the scene so time to leave
    if(newScene->GetName() == this->m_active_scene->GetName()){return;} // another do nothing since active scene is already pulled
    this->m_active_scene = newScene; // now we keep it since this is a valid transition
    return;
}

void GameSceneManager::AddScene(std::shared_ptr<GameScene> newScene){
    if(this->GetScene(newScene->GetName()) != nullptr){
        return; // this scene already exist and shouldnt be added again
    }
    this->m_scenes.push_back(newScene);
    if(m_active_scene == nullptr){
        this->m_active_scene = newScene; // need to place in active scene as its the only one in existance right now
    }
    return;
}

std::shared_ptr<GameScene> GameSceneManager::GetActiveScene(){
    return this->m_active_scene;
}

string GameSceneManager::GetActiveSceneName(){
    if(this->m_active_scene == nullptr){return "";} // something to handle missing activate scenes
    return this->m_active_scene->GetName();
}

void GameSceneManager::UpdateActiveScene(){
    if(!this->HasScenes()){return;} // make sure we have a scene before we try to paint
    this->m_active_scene->Update();

}

void GameSceneManager::DrawActiveScene(){
    if(!this->HasScenes()){return;} // make sure we have something before Drawing it
    this->m_active_scene->Draw();
}


void GameIntroScene::Update(){

}

void GameIntroScene::Draw(){
    this->m_banner->draw(0,0);
    
    // Add helpful text overlay on intro screen
    ofTrueTypeFont font;
    font.load("Verdana.ttf", 18, true, true);
    
    // Draw "Press SPACE to start" text with glow effect
    ofSetColor(0, 0, 0, 180);
    string startMsg = "Press SPACE to Start";
    float msgWidth = font.stringWidth(startMsg);
    float msgX = ofGetWidth() / 2 - msgWidth / 2;
    float msgY = ofGetHeight() - 100;
    
    // Shadow/glow
    font.drawString(startMsg, msgX + 2, msgY + 2);
    
    // Main text
    ofSetColor(255, 255, 100);
    font.drawString(startMsg, msgX, msgY);
    
    // Add controls hint
    ofTrueTypeFont smallFont;
    smallFont.load("Verdana.ttf", 14, true, true);
    ofSetColor(200, 200, 200);
    string controlsHint = "Press C anytime to view controls";
    float hintWidth = smallFont.stringWidth(controlsHint);
    smallFont.drawString(controlsHint, ofGetWidth() / 2 - hintWidth / 2, msgY + 35);
}

void GameOverScene::Update(){

}

void GameOverScene::Draw(){
    ofBackgroundGradient(ofColor::red, ofColor::black);
    this->m_banner->draw(0,0);

}