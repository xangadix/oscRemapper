#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define HOST "localhost"
// listen on port 6666
#define LISTEN_PORT 6666
#define SENDER_PORT 7000
#define NUM_MSG_STRINGS 40

class testApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        void doFourClips();
		void joystickRemap( float m, string a );

		ofTrueTypeFont font;
		ofxOscReceiver receiver;
		ofxOscSender sender;

		int current_msg_string;
		string msg_strings[NUM_MSG_STRINGS];
		float timers[NUM_MSG_STRINGS];

		float signal_amplitude; // used to modulate the floats to resolume values
        float signal_dc; // adds or substracts from signal

        int deck_nr;
        int track_nr; // for running up and down the track
        int clip_nr;
        int layer_nr;

        string button_target;
        string mix_mode;
        string combosEnabled;
        string combos;

        float joy_axis_x; // extend to joy_1_axis_x ?
        float joy_axis_y;

        string addr; // current switch address
};
