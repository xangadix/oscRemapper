#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// listen on the given port
	cout << "listening for osc messages on port " << LISTEN_PORT << "\n";
	receiver.setup(LISTEN_PORT);

    cout << "relay sender at " << HOST ":" << SENDER_PORT << "\n";
    sender.setup(HOST, SENDER_PORT);

	current_msg_string = 0;
	mouseX = 0;
	mouseY = 0;
	mouseButtonState = "";

	ofBackground(30, 30, 130);

    // init values
    signal_amplitude = 1; // used to modulate the floats to resolume values
    signal_dc = 0; // adds or substracts from signal
    track_nr = 1; // for running up and down the track
    clip_nr = 1;
    layer_nr = 1;
    button_target = "/activeclip/video/position/direction";

}

//--------------------------------------------------------------
void testApp::update(){

	// hide old messages
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		if(timers[i] < ofGetElapsedTimef()){
			msg_strings[i] = "";
		}
	}

	// check for waiting messages
	while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m; // received
		receiver.getNextMessage(&m);

		// check for mouse moved message
		if(m.getAddress() == "/mouse/position"){
			// both the arguments are int32's
			mouseX = m.getArgAsInt32(0);
			mouseY = m.getArgAsInt32(1);
		}
		// check for mouse button message
		else if(m.getAddress() == "/mouse/button"){
			// the single argument is a string
			mouseButtonState = m.getArgAsString(0);
		}
		else{

			// unrecognized message: display on the bottom of the screen
			string msg_string;
			msg_string = m.getAddress();

            msg_string += ": ";
			for(int i = 0; i < m.getNumArgs(); i++){
				// get the argument type
				msg_string += m.getArgTypeName(i);
				msg_string += ":";

				// display the argument - make sure we get the right type
				if(m.getArgType(i) == OFXOSC_TYPE_INT32){
					msg_string += ofToString(m.getArgAsInt32(i));
					//s.addIntArg( m.getArgAsInt32(i) );
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
				    // All joystick arguments are floats
                    joystickRemap( m.getArgAsFloat(i), ofToString( m.getAddress() ) );
					msg_string += ofToString( m.getArgAsFloat(i) );
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
					msg_string += m.getArgAsString(i);
					//s.addStringArg(  m.getArgAsString(i) );
				}
				else{
					msg_string += "unknown";
				}
			}

            // add target
            // msg_string += " => " + s.getAddress();

			// add to the list of strings to display
			msg_strings[current_msg_string] = msg_string;
			timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
			current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;

			// clear the next line
			msg_strings[current_msg_string] = "";
		}

	}
}


void testApp::joystickRemap( float m, string a ) {

    // ## remapping goes here
    // ## example: /oscjoy.0.axis.0 (float)
    // ## /oscjoy.0.
    // axis.0 (float)
    // axis.1 (float)
    // axis.2 (float)
    // axis.3 (float)
    // hat.0.y (float) .5
    // hat.0.x (float) .5
    // button.0 (float) (1/0)
    // button.1 (float) (1/0)
    // button.2 (float) (1/0)
    // button.3 (float) (1/0)
    // button.4 (float) (1/0)
    // button.5 (float) (1/0)
    // button.6 (float) (1/0)
    // button.7 (float) (1/0)
    // button.8 (float) (1/0)
    // button.9 (float) (1/0)

    // some resolume mappings
    // /composition/video/cross/values
    // /activelayer/video/position/values
    // /activeclip/video/position/values

    // composition/direction --> for whole compo

    // /track/
    // /track#/connect
    // /layer1/autopilot [1,2,3]
    // /layer1/clip
    // s.setAddress("/composition/video/scale/values");
    // "/composition/link" + num + "/values"; -> rotary dashboard (1-6?)

    // /layer1/clip2
    // /layer1/clip2/position/values

    // left hat & axis; select and switch in pairs of 2 (4 clips)
    // right axis, set active clip for toggling
    // buttons, move track/connect, reset?
    // bottom buttons, clear set 1 set 2 ( per 2 videos )

    if ( a == "/oscjoy.0.hat.0.x") {
       // declare a signal
        // write as ofIsStringInString(a, 'axix')
        ofxOscMessage s;
        s.addFloatArg( ( m * signal_amplitude ) + signal_dc );
        s.setAddress("/layer" + ofToString(layer_nr) + "/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer" + ofToString(layer_nr) + "/video/opacity/values: " << ofToString(( m * signal_amplitude ) + signal_dc) << "\n";
    }

    if ( a == "/oscjoy.0.hat.0.y") {
       // declare a signal
        // write as ofIsStringInString(a, 'axix')
        ofxOscMessage s;
        s.addFloatArg( ( m * signal_amplitude ) + signal_dc );
        s.setAddress("/layer" + ofToString(layer_nr + 1) + "/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer" + ofToString(layer_nr + 1) + "/video/opacity/values: " << ofToString(( m * signal_amplitude ) + signal_dc) << "\n";
    }

    if ( a == "/oscjoy.0.axis.0") {
        // declare a signal
        // write as ofIsStringInString(a, 'axix')
        ofxOscMessage s;
        s.addFloatArg( ( m * signal_amplitude ) + signal_dc );
        s.setAddress("/layer" + ofToString(layer_nr) + "/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer" + ofToString(layer_nr) + "/video/opacity/values: " << ofToString(( m * signal_amplitude ) + signal_dc) << "\n";
    }

    if ( a == "/oscjoy.0.axis.1") {
        ofxOscMessage s;
        s.addFloatArg( ( m * signal_amplitude ) + signal_dc );
        s.setAddress("/layer" + ofToString(layer_nr + 1) + "/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer" + ofToString(layer_nr + 1) + "/video/opacity/values: " << ofToString(( m * signal_amplitude ) + signal_dc) << "\n";
    }

    if ( a == "/oscjoy.0.axis.2") {
        if ( m < 0.05 ) {
            layer_nr++;

            // first select the layer
            ofxOscMessage s2;
            s2.setAddress("/layer" + ofToString(layer_nr) + "/select");
            s2.addFloatArg(1);
            sender.sendMessage(s2);

            // then select the layer and clip
            // resolume auto selects the active clip, which is annoying
            ofxOscMessage s;
            s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/preview");
            s.addFloatArg(1);
            sender.sendMessage(s);
            cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/preview\n";
        }

        if ( m > 0.95 ) {
             layer_nr--;

            // also select the layer
            ofxOscMessage s2;
            s2.setAddress("/layer" + ofToString(layer_nr) + "/select");
            s2.addFloatArg(1);
            sender.sendMessage(s2);

            // then select the layer and clip
            // resolume auto selects the active clip, which is annoying
            if (layer_nr < 1) layer_nr = 1;
            ofxOscMessage s;
            s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/preview");
            s.addFloatArg(1);
            sender.sendMessage(s);
            cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/preview\n";
        }
    }

    if ( a == "/oscjoy.0.axis.3") {
        if ( m > 0.95 ) {
            clip_nr++;
            ofxOscMessage s;
            s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/preview");
            s.addFloatArg(1);
            sender.sendMessage(s);
            cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/preview\n";
        }

        if ( m < 0.05 ) {
            clip_nr--;
            if (clip_nr < 1) clip_nr = 1;
            ofxOscMessage s;
            s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/preview");
            s.addFloatArg(1);
            sender.sendMessage(s);
            cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/preview\n";
        }
    }

    if ( a == "/oscjoy.0.hat.0.x") {
        //s.setAddress("/composition/video/rotatey/values");
    }

    if ( a == "/oscjoy.0.hat.0.y") {
        //s.setAddress("/composition/video/rotatey/values");
    }

    // switch tracks --> TEST
    if ( a == "/oscjoy.0.button.0") {

        if ( m == 1 ) {
            if ( button_target == "/activeclip/video/position/direction" ) {
                button_target = "/activelayer/video/position/direction";

            }else if ( button_target == "/activelayer/video/position/direction" ) {
                button_target = "/composition/direction";

            }else{
                button_target = "/activeclip/video/position/direction";
            }

            // button_target = "/composition/direction

            // To make this work throughout the composition
            // we need another way to rewind and randomize
            // random is now done through c++, not resolume

            cout << "switch to " << button_target << "\n";
        }
    }

    // switch tracks --> TEST
    if ( a == "/oscjoy.0.button.1") {
        //if ( m == 1 ) track_nr--;
        //if (track_nr < 1) track_nr = 1;
        //ofxOscMessage s;
        //s.setAddress("/track" + ofToString(track_nr) + "/connect");
        //s.addFloatArg(1);
        //sender.sendMessage(s);
        //cout << "/track" << ofToString(track_nr) << "/connect\n";
    }

    //if (ofIsStringInString(n, h))

    if ( a == "/oscjoy.0.button.2") {

        // if ( m == 1 ) track_nr++;
        //ofxOscMessage s;
        //s.setAddress("/track" + ofToString(track_nr) + "/connect");
        //s.addFloatArg(1);
        //sender.sendMessage(s);
        //cout << "/track" << ofToString(track_nr) << "/connect\n";

    }

    if ( a == "/oscjoy.0.button.3") {
    }

    // #####################
    // ### FRONT BUTTONS ###
    // #####################

    // front left top
    if ( a == "/oscjoy.0.button.4") {
        // Random position

        /*
        // Random while pressed through c++
        if ( m == 1 ) {
            through c++
            s.setAddress( button_target + "/video/position/values");
            s.addFloatArg( ofRandom(1) );
        }else{
            s.setAddress( button_target + "/video/position/direction");
            s.addIntArg(1);
        }
        sender.sendMessage(s);
        cout << "send random position\n";
        */

        ofxOscMessage s;
        if ( button_target == "/composition/direction") {
            s.setAddress( button_target );
            s.addIntArg(3);
            sender.sendMessage(s);
            cout << "send random \n";
            return;
        }

        string random_target;
        if ( button_target == "/activelayer/video/position/direction" ) random_target = "/activelayer/video/position/values";
        if ( button_target == "/activeclip/video/position/direction") random_target = "/activeclip/video/position/values";

        // Random while pressed through resolume
        // if ( m == 0 ) {
        s.setAddress( random_target );
        s.addFloatArg( ofRandom(1) );
        sender.sendMessage(s);
        cout << "send random \n";

    }

    // front left bottom
    if ( a == "/oscjoy.0.button.5") {

        // Rewind
        //
        //if ( m == 1 ) {
        //s.setAddress( button_target + "/video/position/values");
        //    s.addFloatArg( 0 );
        //    sender.sendMessage(s);
        //    cout << "send position 0\n";
        // }

        ofxOscMessage s;
        if ( button_target == "/composition/direction") {
            cout << "do nothing, composition \n";
            return;
        }

        string rewind_target;
        if ( button_target == "/activelayer/video/position/direction" ) rewind_target = "/activelayer/video/position/values";
        if ( button_target == "/activeclip/video/position/direction") rewind_target = "/activeclip/video/position/values";

        // Random while pressed through resolume
        // if ( m == 0 ) {
        s.setAddress( rewind_target );
        s.addFloatArg( 0 );
        sender.sendMessage(s);
        cout << "send rewind \n";

    }

    // front right top
    if ( a == "/oscjoy.0.button.6") {
        // Reverse while pressed
        ofxOscMessage s;
        if ( m == 0 ) {
            s.setAddress( button_target );
            s.addIntArg(1);
            sender.sendMessage(s);
            cout << "send direction 1\n";
        }

        if ( m == 1) {
            s.setAddress( button_target );
            s.addIntArg(0);
            sender.sendMessage(s);
            cout << "send direction 0\n";
        }
    }

    // front right bottom
    if ( a == "/oscjoy.0.button.7") {
        // Pause while pressed
        ofxOscMessage s;
        s.addFloatArg( m );
        if ( m == 0 ) {
            s.setAddress( button_target );
            s.addIntArg(1);
            sender.sendMessage(s);
            cout << "send pause \n";
        }

        if ( m == 1) {
            s.setAddress( button_target );
            s.addIntArg(2);
            sender.sendMessage(s);
            cout << "send resume \n";
        }
    }

    // under left
    if ( a == "/oscjoy.0.button.8") {
        // solo while pressed layer ( could also work with acive layer )
        ofxOscMessage s;
        s.setAddress("/layer" + ofToString(layer_nr) + "/solo");
        s.addFloatArg(m);
        sender.sendMessage(s);
        cout << "/layer" << ofToString(layer_nr) << "/solo\n";
    }


    // under right
    if ( a == "/oscjoy.0.button.9") {
        // clear layer
        if (m == 1) {
            ofxOscMessage s;
            s.setAddress("/layer" + ofToString(layer_nr) + "/clear");
            s.addFloatArg(1);
            sender.sendMessage(s);
            cout << "/layer" << ofToString(layer_nr) << "/clear\n";
        }
    }

    // on stick, left
    if ( a == "/oscjoy.0.button.10") {
    }

    // on stick right
    if ( a == "/oscjoy.0.button.11") {
        ofxOscMessage s;
        s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/connect");
        s.addFloatArg(1);
        sender.sendMessage(s);
        cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/connect\n";
    }


    // relay message over sender
    // s.setAddress("/test");
    // s.addIntArg(1);
    // s.addFloatArg( 3.5f );
    // s.addStringArg("hello");
    // s.addFloatArg(ofGetElapsedTimef());
    // cout << "relay sender at " << ofToString( s.getAddress() ) << "\n";
}

//--------------------------------------------------------------
void testApp::draw(){

	string buf;
	buf = "listening for osc messages on port " + ofToString( LISTEN_PORT );
	buf += "\nrelaying osc messages to loclhost port " + ofToString( SENDER_PORT );
	ofDrawBitmapString(buf, 10, 20);

	// draw mouse state
	buf = "mouse: " + ofToString(mouseX, 4) +  " " + ofToString(mouseY, 4);
	ofDrawBitmapString(buf, 430, 20);
	ofDrawBitmapString(mouseButtonState, 580, 20);

	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		ofDrawBitmapString( msg_strings[i], 10, 60 + 15 * i);
	}



}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
