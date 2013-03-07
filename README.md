ABOUT
=====

ResolumeJoy tries to elegantly use a joystick, or rather a gamepad, as an 
interface device for Resolume VJ software.

At the moment, the project consists in oscjoy1.10 and the oscRemapper.

- oscJoy: https://public.msli.com/lcs/oscjoy/index.html
- Resolume VJ Software: http://resolume.com/
- OpenFrameworks: http://www.openframeworks.cc/
- OSC: http://opensoundcontrol.org/

We're using a Thrustmaster Firestorm (dual analog 3) for testing. You can use
any gemepad or joystick, but the Firestorm has 12 buttons and mimics the
PS3/PS4 layout. It's a bit bigger (perfect for adult hands) and pretty solid.
Also it is very cheap, you should be able to get a hold of one for under $20.


Release notes
-------------
 - 0.0.1 It actually works on PC
 - 0.0.2 Two sensible implementations are actually working


Roadmap
-------
 - Get the whole thing to run on Mac
 - Upgrade to codeblocks 0074 and code::blocks 12
 - Implement 4 usefull interface modes
 - Refactor out all the coding quirks
 - Merge oscJoy with the reMapper
 - Detach the mappings from the program using XML or JSON or ...
 - make everything easy installable
 - enjoy


Installation
============

PC Users
--------

To use the software with resolume, all you need are the executables for 
oscRemapper and oscJoy. The easy way is to download the repository and click
'startup.bat' which should fire up everything you need. You should see 2
terminal screens. One running oscjoy in debug mode, and another running the 
ReMapper.

If that didn't work, make sure you have the folllowing files available and 
active on your computer.

- oscRemapping.exe
- oscRemapping_DEBUG.exe
- oscjoy.exe

open a command line ( run >> command ) and browse to the directory you have
these files in. First start up osc joy

lib\oscjoy 127.0.0.1:6666 debug

This tells oscJoy to work on the localhost 127.0.0.1 on port 6666. I've set it 
to work in debug mode. If everything went well, you sould see a dump of signals
when you press a button on your gamepad or move it around.

If that went well, start up the remapper.

bin\oscRemapper_DEBUG.exe

Another box should open, and a blue screen; receiving and remapping the signals
of oscJoy

Now finally start up resolume.

In Resolume, check your OSC settings under preferences >> OSC
Resolume should be listening to OSC on Port 7000 (default)

Note, the remapper can also pickup OSC signals sent by Resolume, if you have
Resolume set to sent OSC on port 6667.

tl;dr:
oscjoy sends at port 6666 at which reMapper listens, it sends out osc to 
resolume on the default port 7000


Mac Users
---------

There is no Macintosh version available yet.
VJ Void should make haste with that :p


How does it work?
=================

Basically oscJOY sends out specific signals to OSC addresses. These can be
received and mapped in other programs, but only if they allow re-addressing of
their various functions and elements.

Resolume is very specific in the naming convention of the different objects in
it's interface. For instance, to change the speed of the active clip one has to
send a float to a specific OSC address ( '/activeclip/video/position' ), which 
makes a lot of sense.

oscJOY sends specific signals, like '/joy1/axis.0/float', these have to be
mapped to Resolumes specific addresses. 

The mapper receives these oscJoy messages, collects them, and then relays them
to resolume. But it also repacks them, so it can for instance combine oscJoy
messages to better interact with specific settings in resolume.
The right pad on the joystick for instance sends out x and y values, the Remapper
uses these to select clip and layer values. So the left right pad can be used
to browse through your deck by parsing and repacking values.

All 'mappings' are build this way, by receiving and repacking values for 
Resolume.


Cloning and Editing
===================

NOTE: this program was written in Codeblocks 10.05 and open frameworks 0073
which are older releases, please check the tutorial for setup and make sure 
you download the older versions for this to work. A new release in a newer
version is underway.


PC Users
--------

Download Open Frameworks 0073:
http://www.openframeworks.cc/download/

Download Code Blocks 10.05
http://www.codeblocks.org/downloads/5

Follow this tutorial (note that the linkes downloads are different then the ones
I've put here, please make sure you have the correct files (ie. the ones in this
readme file)
http://www.openframeworks.cc/setup/codeblocks/

IMPORTANT: As the oscRemapper was build upon the openFrameworks Project 
'oscReceiveExample', the oscRemapper the project should be cloned in the same
folder in your openFrameworks dir. Most likely in: 
  
  openFrameworks/Examples/addon/ 

If the workspace etc. resides in that directory, all linkage should work fine.

tl;dr:
Git clone into openFrameworks/Examples/addon/, a reMapper dir will be created
and your the project will compile fine


Mac Users
---------

No tips available


RELEASE
=======

Open source release under a General Public License (GPL) (februari 2013)
originally written by XangadiX for Sense-Studios.com 2013

Copyright (c) 2013, XangadiX for Veejays.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
