#pragma once

#include "ofMain.h"
#include "Aquarium.h"

// Visual effects structures
struct Bubble {
    ofVec2f pos;
    float size;
    float speed;
    float wobble;
};

struct Ripple {
    ofVec2f pos;
    float radius;
    float alpha;
    float maxRadius;
};

struct Particle {
    ofVec2f pos;
    ofVec2f vel;
    float alpha;
    float size;
};

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
	
		
		char moveDirection;
		int DEFAULT_SPEED = 5;

        float powerUpCharge = 100.0f;         
        float powerUpMax = 100.0f;             
        float powerUpDepletionRate = 50.0f;    
        float powerUpRechargeRate = 25.0f;     
        bool powerUpActive = false;            
        int boostedSpeed = 12;                 
        
		float hue = 0.0f;


		AwaitFrames acuariumUpdate{5};

	ofTrueTypeFont gameOverTitle;
	ofTrueTypeFont controlsFont;
	ofTrueTypeFont controlsTitleFont;
	GameEvent lastEvent;


	ofImage backgroundImage;

    // Background ambient music for the aquarium scene
    ofSoundPlayer bgMusic;

	std::unique_ptr<GameSceneManager> gameManager;
	std::shared_ptr<AquariumSpriteManager> spriteManager;
	
	// Controls overlay
	bool showControlsOverlay = true;
	float overlayAlpha = 220.0f;
	
	// Combo system
	int comboCount = 0;
	float comboTimer = 0.0f;
	float comboResetTime = 2.0f; // seconds until combo resets
	
	// Screen shake
	float shakeIntensity = 0.0f;
	float shakeDuration = 0.0f;
	ofVec2f shakeOffset;
	
	// Visual effects
	std::vector<Bubble> bubbles;
	std::vector<Ripple> ripples;
	std::vector<Particle> particles;
	float waterOverlayPulse = 0.0f;
};
