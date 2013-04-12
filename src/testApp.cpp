#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// listen on the given port
	cout << "listening for osc messages on port " << LISTEN_PORT << "\n";
	receiver.setup(LISTEN_PORT);

    cout << "relay sender at " << HOST ":" << SENDER_PORT << "\n";
    sender.setup(HOST, SENDER_PORT);

	current_msg_string = 0;
	ofBackground(30, 30, 130);

    // init values
    signal_amplitude = 1; // used to modulate the floats to resolume values NOT USED
    signal_dc = 0; // adds or substracts from signal NOT USED

    deck_nr = 1;
    track_nr = 1; // for running up and down the track
    clip_nr = 1;
    layer_nr = 1;
    button_target = "/activeclip/video/position/values";
    mix_mode = "2clips"; // 4clips, linkage
    combosEnabled = "false";
    combos = "";

    joy_axis_x = 0.5;
    joy_axis_y = 0.5;
    addr = "";

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

		for(int i = 0; i < m.getNumArgs(); i++){
			string msg_string;
			msg_string = m.getAddress();
            msg_string += ": ";

            if(m.getArgType(i) == OFXOSC_TYPE_FLOAT) {
                // All joystick arguments are floats
                joystickRemap( m.getArgAsFloat(i), ofToString( m.getAddress() ) );
            }

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
    // ## available

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
    // button.10 (float) (1/0)
    // button.11 (float) (1/0)

    // ## (some) resolume mappings and notes
    // set resolume to send signals to 6666 on your ip to view these
    // /composition/video/cross/values
    // /activelayer/video/position/values
    // /activeclip/video/position/values

    // composition/direction --> for whole comps
    // no values :(

    // /track/
    // /track#/connect
    // /layer1/autopilot [1,2,3]
    // /layer1/clip
    // s.setAddress("/composition/video/scale/values");
    // "/composition/link" + num + "/values"; -> rotary dashboard (1-6?)

    // /layer1/clip2
    // /layer1/clip2/position/values

    // ### How to mix 4 clips insteat of 2
    // left hat & axis; select and switch in pairs of 2 (4 clips)
    // right axis, set active clip for toggling
    // buttons, move track/connect, reset?
    // bottom buttons, clear set 1 set 2 ( per 2 videos ) (?)

    // ### although it seems a good idea to have the layers move
    // with selection, I've now chosen to 'lock' the controlled
    // layers into the bottom most layers.
    // here the basic code to make the layer selectable through layer_nr
    // use s.setAddress("/layer" + ofToString(layer_nr) + "/video/opacity/values"); (x) otherwise
    // and s.setAddress("/layer" + ofToString(layer_nr + 1) + "/video/opacity/values"); (y) otherwise


    // ######################
    // ### STICKS AND HAT ###
    // ######################

    if ( a == "/oscjoy.0.hat.0.x") {
       // declare a signal
        // write as ofIsStringInString(a, 'axix')
        ofxOscMessage s;
        s.addFloatArg( m );
        s.setAddress("/layer1/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer1/video/opacity/values: " << ofToString( m ) << "\n";
    }

    if ( a == "/oscjoy.0.hat.0.y") {
       // declare a signal
        // write as ofIsStringInString(a, 'axix')
        ofxOscMessage s;
        s.addFloatArg(  abs( m - 1 ) );
        s.setAddress("/layer2/video/opacity/values");
        sender.sendMessage(s);
        cout << "/layer2/video/opacity/values: " << ofToString(  abs( m - 1 ) ) << "\n";
    }

    // #### end hat ####


    if ( a == "/oscjoy.0.axis.0") { // x, or sideways

        if ( mix_mode == "linkage") { // connect to dashboard
            ofxOscMessage s1;
            ofxOscMessage s2;
            s1.addFloatArg( m );
            s2.addFloatArg( m );
            s1.setAddress("/layer1/link1/values");
            s2.setAddress("/layer2/link1/values");
            sender.sendMessage(s1);
            sender.sendMessage(s2);
            cout << "mode:" << mix_mode << " /layer1/link1/values, /layer1/link1/values " << ofToString( m ) << "\n";
        }

        if ( mix_mode == "4clips") {
            joy_axis_x = m;
            doFourClips();
        }

        if ( mix_mode == "2clips") { // default
            ofxOscMessage s;
            s.addFloatArg( m ) ;
            s.setAddress("/layer1/video/opacity/values");
            sender.sendMessage(s);
            cout << "mode:" << mix_mode << " /layer1/video/opacity/values: " << ofToString( m ) << "\n";
        }
    }

    if ( a == "/oscjoy.0.axis.1") { //, y or upward

        if ( mix_mode == "linkage") { // connect to dashboard
            ofxOscMessage s1;
            ofxOscMessage s2;
            s1.addFloatArg( m );
            s2.addFloatArg( m );
            s1.setAddress("/layer1/link2/values");
            s2.setAddress("/layer2/link2/values");
            sender.sendMessage(s1);
            sender.sendMessage(s2);
            cout << "mode:" << mix_mode << " /layer1/link2/values, /layer2/link2/values " << ofToString( m ) << "\n";
        }

        if ( mix_mode == "4clips") {
            joy_axis_y = m;
            doFourClips();
        }

        if ( mix_mode == "2clips") { // default
            ofxOscMessage s;
            s.addFloatArg( abs( m - 1 ) );
            s.setAddress("/layer2/video/opacity/values");
            sender.sendMessage(s);
            cout << "mode:" << mix_mode << " /layer2/video/opacity/values: " << ofToString( abs( m - 1 ) ) << "\n";
        }
    }

    // #### end axis left ####

    if ( a == "/oscjoy.0.axis.2") {
        if ( m < 0.05 ) {
            layer_nr++;

            // first select the layer
            ofxOscMessage s2;
            s2.setAddress("/layer" + ofToString(layer_nr) + "/select");
            s2.addFloatArg(1);
            sender.sendMessage(s2);

            // then select the layer and clip
            // resolume auto previews the active clip, which is annoying, there is no select
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
            // resolume auto previews the active clip, which is annoying, there is no select
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

    // #### end axis right #####



    if ( a == "/oscjoy.0.button.0") {
        // switch tracks --> TEST
        // if ( m == 1 ) track_nr++;
        //ofxOscMessage s;
        //s.setAddress("/track" + ofToString(track_nr) + "/connect");
        //s.addFloatArg(1);
        //sender.sendMessage(s);
        //cout << "/track" << ofToString(track_nr) << "/connect\n";

        // set mix mode
        // 2clips, 4clips, linkage
        cout << "switch mode";

        if ( m == 1 ) {
            if (mix_mode == "2clips") {
                mix_mode = "4clips";
            }else if(mix_mode == "4clips") {
                mix_mode = "linkage";
            }else{
                mix_mode = "2clips";
            }

            cout << "mix mode set to:" << mix_mode << "\n";
        }
    }

    if ( a == "/oscjoy.0.button.1") {
        if ( m == 1 ) {
            if ( button_target == "/activeclip/video/position/values" ) {
                button_target = "/activelayer/video/position/values";

            }else if ( button_target == "/activelayer/video/position/values" ) {
                button_target = "/composition/direction";

            }else{
                button_target = "/activeclip/video/position/values";
            }

            cout << "switch to " << button_target << "\n";
        }
    }

    //if (ofIsStringInString(n, h))

    if ( a == "/oscjoy.0.button.2") {
        // switch tracks --> TEST
        //if ( m == 1 ) track_nr--;
        //if (track_nr < 1) track_nr = 1;
        //ofxOscMessage s;
        //s.setAddress("/track" + ofToString(track_nr) + "/connect");
        //s.addFloatArg(1);
        //sender.sendMessage(s);
        //cout << "/track" << ofToString(track_nr) << "/connect\n";
        if ( m == 1 ) {
            combosEnabled = "true";
        }else{
            combosEnabled = "false";
        }
    }

    if ( a == "/oscjoy.0.button.3") {
        // switch tracks --> TEST
        //if ( m == 1 ) track_nr++;
        //ofxOscMessage s;
        //s.setAddress("/track" + ofToString(track_nr) + "/connect");
        //s.addFloatArg(1);
        //sender.sendMessage(s);
        //cout << "/track" << ofToString(track_nr) << "/connect\n";
    }


    // #####################
    // ### FRONT BUTTONS ###
    // #####################

    // front left top
    if ( a == "/oscjoy.0.button.4") {
        // Random position
        ofxOscMessage s;

        if ( button_target != "/composition/direction") {
            // Random while pressed through c++
            // this doesn't work with compositions as they have no
            // video/position/values

            if ( m == 1 ) {
                s.setAddress( button_target );
                s.addFloatArg( ofRandom(1) );
                sender.sendMessage(s);
                cout << button_target << "\n";
            }else{
                s.setAddress( button_target );
                s.addIntArg(2);
                sender.sendMessage(s);
                cout << "send resume " << button_target << "\n";
            }


        }else{

            // Random while pressed through resolume
            if ( m == 0 ) {
                s.setAddress( button_target );
                s.addIntArg(1);
                sender.sendMessage(s);
                cout << "send resume \n";
            }

            if ( m == 1) {
                s.setAddress( button_target );
                s.addIntArg(3);
                sender.sendMessage(s);
                cout << "send random direction \n";
            }
        }
    }

    // front left bottom
    if ( a == "/oscjoy.0.button.5") {

        // Rewind
        ofxOscMessage s;

        if ( button_target != "/composition/direction") {
            // Random while pressed through c++
            // this doesn't work with compositions as they have no
            // video/position/values
            if ( m == 1 ) {
                s.setAddress( button_target );
                s.addFloatArg( 0 );
                sender.sendMessage(s);
                cout << "Send position 0 on " << button_target << "\n";
            }

        }else{
           cout << "No rewind with " << button_target << "\n";
        }
    }

    // front right top
    if ( a == "/oscjoy.0.button.6") {
        // Reverse while pressed
        string direction_target = button_target;
        if ( button_target == "/activeclip/video/position/values") direction_target = "/activeclip/video/position/direction";
        if ( button_target == "/activelayer/video/position/values") direction_target = "/activelayer/video/position/direction";

        ofxOscMessage s;
        if ( m == 0 ) {
            s.setAddress( direction_target );
            s.addIntArg(1);
            sender.sendMessage(s);
            cout << "send direction 1 " << direction_target << "\n";
        }

        if ( m == 1) {
            s.setAddress( direction_target );
            s.addIntArg(0);
            sender.sendMessage(s);
            cout << "send direction 0 " << direction_target << "\n";
        }
    }

    // front right bottom
    if ( a == "/oscjoy.0.button.7") {
        // Pause while pressed
        // Reverse while pressed
        string pause_target = button_target;
        if ( button_target == "/activeclip/video/position/values") pause_target = "/activeclip/video/position/direction";
        if ( button_target == "/activelayer/video/position/values") pause_target = "/activelayer/video/position/direction";

        ofxOscMessage s;
        if ( m == 0 ) {
            s.setAddress( pause_target );
            s.addIntArg(1);
            sender.sendMessage(s);
            cout << "send direction 1 " << pause_target << "\n";
        }

        if ( m == 1) {
            s.setAddress( pause_target );
            s.addIntArg(2);
            sender.sendMessage(s);
            cout << "send direction 2 " << pause_target << "\n";
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
        // nothing
    }

    // on stick right
    if ( a == "/oscjoy.0.button.11") {
        // set previewed clip as active
        ofxOscMessage s;
        s.setAddress("/layer" + ofToString(layer_nr) + "/clip" + ofToString(clip_nr) + "/connect");
        s.addFloatArg(1);
        sender.sendMessage(s);
        cout << "/layer" << ofToString(layer_nr) << "/clip" << ofToString(clip_nr) << "/connect\n";
    }
}

void testApp::doFourClips() {
    /*
    l1 [l1c1][l1c2]
    l2 [l2c1][l2c2]

    // ## switch pattern
    0   l1:.5, l2:.5 -- l1: c1, l2: c1 -- jx: 0.5,  jy: 0
    1   l1: 1, l2:.0 -- l1: c1, l2:    -- jx: 1,    jy: 0
    2   l1:.5, l2:.5 -- l1: c1, l2: c2 -- jx: 1,    jy: 0.5
    3   l1: 0, l2: 1 -- l1:   , l2: c2 -- jx: 1,    jy: 1
    4   11:.5, l2:.5 -- l1: c2, l2: c2 -- jx: 0.5,  jy: 1
    5   11: 1, l2: 0 -- l1: c2, l2:    -- jx: 0,    jy: 1
    6   11:.5, l2:.5 -- l1: c2, l2: c1 -- jx: 0,    jy: 0.5
    7   l1: 0, l2: 1 -- l1:   , l2: c1 -- jx: 0,    jy: 0

    7   0   1
      \ | /
    6 -   - 2
      / | \
    5   4   3

    // ## fade layers pattern

        5 1 5 0 5 1 5 0 --> layer 1

        5 1 1 1 5 0 0 0 --> fader x
        0 0 5 1 1 1 5 0 --> fader y

        5 0 5 1 5 0 5 1 --> layer 2

        5 1 1 1 5 0 0 0 --> fader x
        0 0 5 1 1 1 5 0 --> fader y
    */

    float marge_high = 0.9;
    float marge_low = 0.1;

    // select clip
    string hasSwitched = "false";
    string combo22 = "";
    string combo12 = "";
    string combo21 = "";
    string combo11 = "";

    if ( joy_axis_x > marge_high && joy_axis_y < marge_low && addr != "/layer2/clip"+ofToString(clip_nr+1)+"/connect" ) {
        addr = "/layer2/clip"+ofToString(clip_nr+1)+"/connect";
        hasSwitched = "true";
        combo22 = "TR";
    }

    if ( joy_axis_x > marge_high && joy_axis_y > marge_high && addr != "/layer1/clip"+ofToString(clip_nr+1)+"/connect" ) {
        addr = "/layer1/clip"+ofToString(clip_nr+1)+"/connect";
        hasSwitched = "true";
        combo12 = "DR";
    }

    if ( joy_axis_x < marge_low && joy_axis_y > marge_high && addr != "/layer2/clip"+ofToString(clip_nr)+"/connect" ) {
        addr = "/layer2/clip"+ofToString(clip_nr)+"/connect";
        hasSwitched = "true";
        combo21 = "TL";
    }

    if ( joy_axis_x < marge_low && joy_axis_y < marge_low && addr != "/layer1/clip"+ofToString(clip_nr)+"/connect" ) {
        addr = "/layer1/clip"+ofToString(clip_nr)+"/connect";
        hasSwitched = "true";
        combo11 = "DL";
    }

    if ( hasSwitched == "true") {
        ofxOscMessage s;
        s.setAddress( addr );
        s.addFloatArg(1);
        sender.sendMessage(s);
        cout << " mode: " << mix_mode << " addr: " << addr << "\n";
        if ( combosEnabled == "true" ) {
            combos = combos + combo22 + combo12 + combo21 + combo11;
        }else{
            combos = "disabled";
        }
    }

    if ( ofIsStringInString( combos, "TRDRTLDL" ) ) {
        cout << "COMBO: Right";
        combos = "";
        clip_nr++;
    }

    if ( ofIsStringInString( combos, "DLTLDRTR" ) ) {
        cout << "COMBO: Left";
        combos = "";
        if (clip_nr > 1) clip_nr--;
    }

    float layer2_opacity = abs( ( joy_axis_x + joy_axis_y ) - 1 );
    float layer1_opacity = abs( layer2_opacity - 1 );

    ofxOscMessage s1;
    ofxOscMessage s2;
    s1.setAddress( "/layer1/video/opacity/values" );
    s2.setAddress( "/layer2/video/opacity/values" );
    s1.addFloatArg(layer1_opacity);
    s2.addFloatArg(layer2_opacity);
    sender.sendMessage(s1);
    sender.sendMessage(s2);

    ofxOscMessage s3;
    string set_active = "";
    if (layer1_opacity > 0.7) set_active = "/layer1/select";
    if (layer2_opacity > 0.7) set_active = "/layer2/select";
    s3.setAddress( set_active );
    s3.addFloatArg(1);
    sender.sendMessage(s3);

    cout << "layer 1: " << layer1_opacity << "layer 2: " << layer2_opacity << "\n";
}


//--------------------------------------------------------------
void testApp::draw(){

	string buf;
	buf = "listening for osc messages on port " + ofToString( LISTEN_PORT );
	buf += "\nrelaying osc messages to localhost, port " + ofToString( SENDER_PORT );
	buf += "\n";
	buf += "\n deck: " + ofToString( deck_nr ) + ", layer: " + ofToString( layer_nr ) + ", clip: " + ofToString( clip_nr ) + ", track: " + ofToString( track_nr );
	buf += "\n mode: " + mix_mode + ", target: " + button_target;
	buf += "\n combos: " + combos;
	ofDrawBitmapString(buf, 10, 20);

	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		ofDrawBitmapString( msg_strings[i], 10, 112 + 14 * i);
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
