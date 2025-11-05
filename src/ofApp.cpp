#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    ofSetBackgroundColor(ofColor::blue);
    backgroundImage.load("background.png");
    backgroundImage.resize(ofGetWindowWidth(), ofGetWindowHeight());

    // load background ambient music (preload to avoid first-play hitch)
    // Try .wav first, then .ogg as fallback
    if (!bgMusic.load("ambient.wav")) {
        ofLogWarning() << "Failed to load ambient.wav, trying ambient.ogg";
        if (!bgMusic.load("ambient.ogg")) {
            ofLogError() << "Failed to load both ambient.wav and ambient.ogg";
        } else {
            ofLogNotice() << "Successfully loaded ambient.ogg";
        }
    } else {
        ofLogNotice() << "Successfully loaded ambient.wav";
    }
    bgMusic.setLoop(true);
    bgMusic.setMultiPlay(false);
    bgMusic.setVolume(0.6f); // reasonable volume
    // Music will play when entering the aquarium scene


    std::shared_ptr<Aquarium> myAquarium;
    std::shared_ptr<PlayerCreature> player;

    // make the game scene manager 
    gameManager = std::make_unique<GameSceneManager>();


    // first we make the intro scene 
    gameManager->AddScene(std::make_shared<GameIntroScene>(
        GameSceneKindToString(GameSceneKind::GAME_INTRO),
        std::make_shared<GameSprite>("title.png", ofGetWindowWidth(), ofGetWindowHeight())
    ));

    //AquariumSpriteManager
    spriteManager = std::make_shared<AquariumSpriteManager>();

    // Lets setup the aquarium
    myAquarium = std::make_shared<Aquarium>(ofGetWindowWidth(), ofGetWindowHeight(), spriteManager);
    player = std::make_shared<PlayerCreature>(ofGetWindowWidth()/2 - 50, ofGetWindowHeight()/2 - 50, DEFAULT_SPEED, this->spriteManager->GetSprite(AquariumCreatureType::NPCreature));
    player->setDirection(0, 0); // Initially stationary
    player->setBounds(ofGetWindowWidth() - 20, ofGetWindowHeight() - 20);


    myAquarium->addAquariumLevel(std::make_shared<Level_0>(0, 25));
    myAquarium->addAquariumLevel(std::make_shared<Level_1>(1, 50));
    myAquarium->addAquariumLevel(std::make_shared<Level_2>(2, 200));
    myAquarium->Repopulate(); // initial population

    // now that we are mostly set, lets pass the player and the aquarium downstream
    gameManager->AddScene(std::make_shared<AquariumGameScene>(
        std::move(player), std::move(myAquarium), GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)
    )); // player and aquarium are owned by the scene moving forward

    // Load font for game over message
    gameOverTitle.load("Verdana.ttf", 12, true, true);
    gameOverTitle.setLineHeight(34.0f);
    gameOverTitle.setLetterSpacing(1.035);

    // Load fonts for controls overlay
    controlsTitleFont.load("Verdana.ttf", 24, true, true);
    controlsTitleFont.setLineHeight(30.0f);
    controlsFont.load("Verdana.ttf", 14, true, true);
    controlsFont.setLineHeight(20.0f);


    gameManager->AddScene(std::make_shared<GameOverScene>(
        GameSceneKindToString(GameSceneKind::GAME_OVER),
        std::make_shared<GameSprite>("game-over.png", ofGetWindowWidth(), ofGetWindowHeight())
    ));

    // Initialize bubbles
    for(int i = 0; i < 20; i++){
        Bubble b;
        b.pos.set(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
        b.size = ofRandom(3, 10);
        b.speed = ofRandom(0.5, 2.0);
        b.wobble = ofRandom(0, TWO_PI);
        bubbles.push_back(b);
    }

    ofSetLogLevel(OF_LOG_NOTICE); // Set default log level
}

//--------------------------------------------------------------
void ofApp::update() {

    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::GAME_OVER)){
        return; //stop updating if game is over or exiting
    }

    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)){
        auto gameScene = std::static_pointer_cast<AquariumGameScene>(gameManager->GetActiveScene());
        if(gameScene->GetLastEvent() != nullptr && gameScene->GetLastEvent()->isGameOver()){
            gameManager->Transition(GameSceneKindToString(GameSceneKind::GAME_OVER));
            // stop ambient music when game ends
            if (bgMusic.isPlaying()) bgMusic.stop();
            return;
        }
    }

    gameManager->UpdateActiveScene();

    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)){
        auto gameScene = std::static_pointer_cast<AquariumGameScene>(gameManager->GetActiveScene());
        auto player = gameScene->GetPlayer();

        float deltaTime = 1.0f / 60.0f; 
        
        // Track player score to detect consumption and increment combo
        static int lastScore = 0;
        int currentScore = player->getScore();
        if(currentScore > lastScore){
            // Player just consumed something!
            comboCount++;
            comboTimer = comboResetTime; // reset timer
            
            // Trigger screen shake
            shakeIntensity = 5.0f;
            shakeDuration = 0.15f; // shake for 0.15 seconds
            
            // Spawn ripple at player position
            Ripple r;
            r.pos.set(player->getX(), player->getY());
            r.radius = 0;
            r.alpha = 255;
            r.maxRadius = 80;
            ripples.push_back(r);
            
            // Spawn particle burst
            for(int i = 0; i < 10; i++){
                Particle p;
                p.pos.set(player->getX(), player->getY());
                float angle = ofRandom(TWO_PI);
                float speed = ofRandom(2, 5);
                p.vel.set(cos(angle) * speed, sin(angle) * speed);
                p.alpha = 255;
                p.size = ofRandom(3, 8);
                particles.push_back(p);
            }
            
            lastScore = currentScore;
        } else {
            lastScore = currentScore;
        }
        
        // Update combo timer
        if(comboTimer > 0){
            comboTimer -= deltaTime;
            if(comboTimer <= 0){
                comboCount = 0; // reset combo
            }
        }
        
        // Update screen shake
        if(shakeDuration > 0){
            shakeDuration -= deltaTime;
            shakeOffset.x = ofRandom(-shakeIntensity, shakeIntensity);
            shakeOffset.y = ofRandom(-shakeIntensity, shakeIntensity);
        } else {
            shakeOffset.set(0, 0);
        }

        if(powerUpActive){
            powerUpCharge -= powerUpDepletionRate * deltaTime;
            if(powerUpCharge <= 0.0f){
                powerUpCharge = 0.0f;
                powerUpActive = false; // stops boost when empty
                player->currentColor = ofColor::white; // reset color
            } else {
                // boost player speed
                player->setSpeed(boostedSpeed);

                // rainbow color effect
                float time = ofGetElapsedTimef();
                player->currentColor.setHsb(fmod(time * 100, 255), 255, 255); 
            }
        } else {
            // powerup recharge
            powerUpCharge += powerUpRechargeRate * deltaTime;
            if(powerUpCharge > powerUpMax) powerUpCharge = powerUpMax;

            player->setSpeed(DEFAULT_SPEED);
            player->currentColor = ofColor::white; // reset to normal
        }
        
        // Update bubbles
        for(auto& bubble : bubbles){
            bubble.pos.y -= bubble.speed;
            bubble.wobble += 0.05;
            bubble.pos.x += sin(bubble.wobble) * 0.5;
            
            // Respawn at bottom when reaching top
            if(bubble.pos.y < -20){
                bubble.pos.y = ofGetHeight() + 20;
                bubble.pos.x = ofRandom(ofGetWidth());
            }
        }
        
        // Update ripples
        for(auto it = ripples.begin(); it != ripples.end();){
            it->radius += 3.0f;
            it->alpha -= 8.0f;
            if(it->alpha <= 0 || it->radius > it->maxRadius){
                it = ripples.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update particles
        for(auto it = particles.begin(); it != particles.end();){
            it->pos += it->vel;
            it->vel.y += 0.15; // gravity
            it->alpha -= 5.0f;
            if(it->alpha <= 0){
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update water overlay pulse
        waterOverlayPulse += deltaTime * 0.5;
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    
    // Apply screen shake offset
    ofTranslate(shakeOffset.x, shakeOffset.y);
    
    backgroundImage.draw(0, 0);
    gameManager->DrawActiveScene();

if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)){
    
    // Draw bubbles
    ofSetColor(255, 255, 255, 180); // Made more visible
    for(const auto& bubble : bubbles){
        ofDrawCircle(bubble.pos.x, bubble.pos.y, bubble.size);
    }
    
    // Draw ripples
    ofNoFill();
    for(const auto& ripple : ripples){
        ofSetColor(100, 200, 255, ripple.alpha);
        ofSetLineWidth(2);
        ofDrawCircle(ripple.pos.x, ripple.pos.y, ripple.radius);
    }
    ofFill();
    ofSetLineWidth(1);
    
    // Draw particles
    for(const auto& particle : particles){
        ofSetColor(255, 255, 150, particle.alpha);
        ofDrawCircle(particle.pos.x, particle.pos.y, particle.size);
    }
    
    // Draw water color overlay with pulse
    float pulseAlpha = 10 + sin(waterOverlayPulse) * 5;
    ofSetColor(0, 100, 150, pulseAlpha);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    
    float barWidth = 200.0f;
    float barHeight = 20.0f;
    float x = ofGetWidth() / 2 - barWidth / 2;
    float y = 20.0f;

    ofSetColor(100, 100, 100);
    ofDrawRectangle(x, y, barWidth, barHeight);

    ofSetColor(50, 200, 50); //green
    ofDrawRectangle(x, y, barWidth * (powerUpCharge / 100.0f), barHeight);

    ofNoFill();
    ofSetColor(255);
    ofDrawRectangle(x, y, barWidth, barHeight);
    ofFill();
    
    // Draw combo counter
    if(comboCount > 1){
        ofSetColor(255, 255, 0); // yellow
        string comboText = "COMBO x" + ofToString(comboCount);
        float comboSize = 20 + comboCount * 2; // size grows with combo
        
        ofTrueTypeFont comboFont;
        comboFont.load("Verdana.ttf", comboSize, true, true);
        
        float comboWidth = comboFont.stringWidth(comboText);
        float comboX = ofGetWidth() / 2 - comboWidth / 2;
        float comboY = 80;
        
        // Shadow
        ofSetColor(0, 0, 0, 180);
        comboFont.drawString(comboText, comboX + 2, comboY + 2);
        
        // Main text with pulsing effect
        float pulse = sin(ofGetElapsedTimef() * 10) * 0.5 + 0.5;
        ofSetColor(255, 255, 0, 150 + pulse * 105);
        comboFont.drawString(comboText, comboX, comboY);
    }
    
    // Draw controls overlay
    if(showControlsOverlay){
        // Semi-transparent dark overlay
        ofSetColor(0, 0, 0, overlayAlpha);
        float overlayWidth = 500;
        float overlayHeight = 380;
        float overlayX = ofGetWidth() / 2 - overlayWidth / 2;
        float overlayY = ofGetHeight() / 2 - overlayHeight / 2;
        
        // Draw rounded rectangle background
        ofDrawRectRounded(overlayX, overlayY, overlayWidth, overlayHeight, 15);
        
        // Draw border
        ofNoFill();
        ofSetColor(100, 200, 255, 255);
        ofSetLineWidth(3);
        ofDrawRectRounded(overlayX, overlayY, overlayWidth, overlayHeight, 15);
        ofFill();
        ofSetLineWidth(1);
        
        // Title
        ofSetColor(100, 200, 255);
        string title = "CONTROLS";
        float titleWidth = controlsTitleFont.stringWidth(title);
        controlsTitleFont.drawString(title, overlayX + overlayWidth/2 - titleWidth/2, overlayY + 45);
        
        // Draw controls text
        ofSetColor(255, 255, 255);
        float textX = overlayX + 50;
        float textY = overlayY + 90;
        float lineSpacing = 35;
        
        controlsFont.drawString("MOVEMENT", textX, textY);
        textY += lineSpacing;
        
        ofSetColor(200, 200, 200);
        controlsFont.drawString("Arrow Keys / WASD - Move your fish", textX + 20, textY);
        textY += lineSpacing;
        
        ofSetColor(255, 255, 255);
        controlsFont.drawString("POWER-UP", textX, textY);
        textY += lineSpacing;
        
        ofSetColor(200, 200, 200);
        controlsFont.drawString("Hold P - Activate speed boost", textX + 20, textY);
        textY += lineSpacing;
        controlsFont.drawString("(Depletes green bar, recharges when off)", textX + 20, textY);
        textY += lineSpacing + 10;
        
        ofSetColor(255, 255, 255);
        controlsFont.drawString("OBJECTIVE", textX, textY);
        textY += lineSpacing;
        
        ofSetColor(200, 200, 200);
        controlsFont.drawString("Consume smaller creatures to score points", textX + 20, textY);
        textY += lineSpacing;
        controlsFont.drawString("Reach target score to advance levels", textX + 20, textY);
        textY += lineSpacing;
        controlsFont.drawString("Avoid or consume larger fish!", textX + 20, textY);
        
        // Close instruction at bottom
        textY = overlayY + overlayHeight - 40;
        ofSetColor(255, 255, 100);
        string closeMsg = "Press C to close this overlay";
        float closeMsgWidth = controlsFont.stringWidth(closeMsg);
        controlsFont.drawString(closeMsg, overlayX + overlayWidth/2 - closeMsgWidth/2, textY);
    }
}

    ofPopMatrix(); // End screen shake transform

}

//--------------------------------------------------------------
void ofApp::exit(){
    if (bgMusic.isPlaying()) bgMusic.stop();
    bgMusic.unload();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if(key == 'p'){ //activate boost
    if(powerUpCharge > 0.0f){
        powerUpActive = true; //start boost
    }
}
  
  if(key == 'c' || key == 'C'){ //toggle controls overlay
    showControlsOverlay = !showControlsOverlay;
    return;
  }

    if (lastEvent.isGameExit()) { 
        ofLogNotice() << "Game has ended. Press ESC to exit." << std::endl;
        return; // Ignore other keys after game over
    }
    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)){
        auto gameScene = std::static_pointer_cast<AquariumGameScene>(gameManager->GetActiveScene());
        switch(key){
            case OF_KEY_UP:
                gameScene->GetPlayer()->setDirection(gameScene->GetPlayer()->isXDirectionActive()?gameScene->GetPlayer()->getDx():0, -1);
                break;
                case OF_KEY_DOWN:
                gameScene->GetPlayer()->setDirection(gameScene->GetPlayer()->isXDirectionActive()?gameScene->GetPlayer()->getDx():0, 1);
                break;
            case OF_KEY_LEFT:
                gameScene->GetPlayer()->setDirection(-1, gameScene->GetPlayer()->isYDirectionActive()?gameScene->GetPlayer()->getDy():0);
                gameScene->GetPlayer()->setFlipped(true);
                break;
                case OF_KEY_RIGHT:
                gameScene->GetPlayer()->setDirection(1, gameScene->GetPlayer()->isYDirectionActive()?gameScene->GetPlayer()->getDy():0);
                gameScene->GetPlayer()->setFlipped(false);
                break;
            default:
                break;
        }
    
        
    
        gameScene->GetPlayer()->move();
        return;

    }

    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::GAME_INTRO)){
        switch (key)
        {
        case OF_KEY_SPACE:
            gameManager->Transition(GameSceneKindToString(GameSceneKind::AQUARIUM_GAME));
            // start ambient music when entering aquarium scene
            if (!bgMusic.isPlaying()) {
                bgMusic.play();
                ofLogNotice() << "Playing ambient music now!";
            }
            break;
        
        default:
            break;
        }
    }


    
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  
if(key == 'p'){
    powerUpActive = false; //stops boost when key released
}

    if(gameManager->GetActiveSceneName() == GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)){
        auto gameScene = std::static_pointer_cast<AquariumGameScene>(gameManager->GetActiveScene());
    if( key == OF_KEY_UP || key == OF_KEY_DOWN){
        gameScene->GetPlayer()->setDirection(gameScene->GetPlayer()->isXDirectionActive()?gameScene->GetPlayer()->getDx():0, 0);
        gameScene->GetPlayer()->move();
        return;
    }
    
    if(key == OF_KEY_LEFT || key == OF_KEY_RIGHT){
        gameScene->GetPlayer()->setDirection(0, gameScene->GetPlayer()->isYDirectionActive()?gameScene->GetPlayer()->getDy():0);
        gameScene->GetPlayer()->move();
        return;
    }

    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    backgroundImage.resize(w, h);
    auto aquariumScene = std::static_pointer_cast<AquariumGameScene>(gameManager->GetScene(GameSceneKindToString(GameSceneKind::AQUARIUM_GAME)));
    aquariumScene->GetAquarium()->setBounds(w,h);
    aquariumScene->GetPlayer()->setBounds(w - 20, h - 20);

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
