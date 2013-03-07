oscjoy v1.10 12/8/2007
by Jeremy Friesner (jaf@lcsaudio.com)
Copyright 2007 Meyer Sound Laboratories inc.

This directory contains oscjoy a command line utility that watches 
all the joysticks that are hooked up to your computer and sends out 
OpenSoundControl message over UDP whenever a joysticks' state changes.

oscjoy is designed to let you use your regular computer joysticks 
to control anything that can be controlled via OpenSoundControl.


How do I install oscjoy?
------------------------

Just copy the oscjoy executable from the appropriate subdirectory
(macosx_bin, suse10_bin, or win32_bin) to wherever you want to keep it.
That's all!


What if I want to recompile oscjoy myself?
------------------------------------------

That's easy:  Under Linux or MacOS/X just type "make".
Under Windows, you can use the included oscjoy.vcproj under
Microsoft Visual C++, or if you have the free Borland C++ compiler 
(BCC55) installed, you can compile by typing "make -f Makefile.borland".

The executable will appear as "oscjoy" (or "oscjoy.exe", under Windows)


How is oscjoy licensed?
-----------------------

oscjoy is licensed under the GNU Lesser Public License.  
See the file COPYING for details.

Note that the code in sdl_joystick folder was borrowed from the SDL 
library and modified to fit this program's needs.  

The code in the sdl_joystick folder is under the GNU LGPL license.


How do I use oscjoy? 
--------------------

Follow these steps:

1) Open a shell window

2) Run oscjoy like this:

    ./oscjoy 192.168.1.101:14005 debug

   (replace the IP address and port number with appropriate values, 
   depending on where you want oscjoy to send the UDP packets to)
   
3) Wiggle your joystick(s) and note the output that oscjoy prints to stdout.
   The debug output will tell you the method names of the OpenSoundControl 
   control points that oscjoy is sending, and the values it is setting them to.
 
4) Set up your OpenSoundControl-receiving program of choice to handle the 
   packets appropriately.

5) Re-run oscjoy without the "debug" flag if you don't want all that output.


What other command line options does oscjoy support?
----------------------------------------------------

Usage:  ./oscjoy target.ip.address[:targetport] [prefix=oscjoy] 
                 [sourceport=8000] [updaterate=100Hz] [debug] [daemon]
                 [zerocenter] [axis:ASPEC=ASPEC] [hat:ASPEC=ASPEC] 
                 [ball:ASPEC=ASPEC] [button:ASPEC=ASPEC]

"prefix" lets you specify a prefix for the OpenSoundControl method
names that are generated.  For example, setting the prefix to "Fred"
and moving the joystick would send the OSC method "/Fred.0.axis.0".
The default prefix is "oscjoy".

"sourceport" lets you specify the local port that the UDP socket
is bound to.  oscjoy never receives any UDP packets, it only sends
them, so binding to a well-defined local port isn't strictly 
necessary.  However, some OpenSoundControl programs (such as LCS's
LX-300 system) use the sender's port number when deciding how
to handle the incoming packets, so it is still sometimes useful
to set it.  You can specify sourceport=0 if you don't want the
UDP socket to be bound to any local port.  The default behavior
is to bind to local UDP port 8000.

"updaterate" lets you specify how many times per second the
joysticks should be polled.  This value can range between 1Hz
and 1000Hz.  The default update rate is 100Hz.

"debug", if specified, will cause oscjoy to output diagnostic
information to stdout whenever a joystick movement is detected.
This is handy for quickly determining what OSC method names
and values oscjoy is sending.

"daemon", if specified, will cause oscjoy to run as a background
process.  Note that this option is not supported under Windows.

"zerocenter", if specified, will cause joystick range values
to be sent in the range [-1.0f,1.0f], instead of the default
range of [0.0f,1.0f].  See below for details.

"axis", "hat", "ball", and "button" arguments allow you to make
aliases -- that is, to tell OSCJoy to remap control names so
that when a specified control is moved on the joystic, OSCJoy
will pretend that a different control was moved instead.  This
can be useful if you want your joystick appear differently to
the OSC-receiving program.  The possible formats for ASPEC are
as follows:

   joy.ctl.axis  (e.g. "0.1.y")
   joy.ctl       (e.g. "1.2")
   ctl           (e.g. "5")

If the value for "joy" is not specified, joystick #0 will be
assumed.  If no axis value is specified, then no sub-axis value
will be assumed (sub-axes are only used for hats and trackballs
anyway).  So for example if you wanted to swap your joystick's
X and Y axes, you could do this:

   ./oscjoy ip_address axis:1=0 axis:0=1

Or if you wanted the first button on joystick #0 to appear to
be the 10th button on joystick #1, you could do this:

   ./oscjoy ip_address button:0=1.9

and so on.


What is the format of the data that oscjoy sends?
-------------------------------------------------

oscjoy sends UDP packets, with the data in the UDP packets
corresponding to the OpenSoundControl protocol specification
which is documented here:

   http://www.cnmat.berkeley.edu/OpenSoundControl/OSC-spec.html

Each component of the joystick (axis, button, trackball, or hat)
is represented by a unique OSC method-name, which is composed
of the user-specified prefix string, joystick ID, control type
name, control type index, and optionally a sub-index.  All indices
are zero-based.  For example, the second button of the first
joystick is represented like this:

   /oscjoy.0.button.1

The values associated with the method are normalized to the range 
[0.0f, 1.0f].  So a centered joystick would be sent as

Send 0/2: [/oscjoy.0.axis.0]=0.500000
Send 1/2: [/oscjoy.0.axis.1]=0.500000

whereas a joystick moved to the upper left would be sent as

Send 0/2: [/oscjoy.0.axis.0]=0.000000
Send 1/2: [/oscjoy.0.axis.1]=0.000000

and a joystick move to the lower right would be sent as

Send 0/2: [/oscjoy.0.axis.0]=1.000000
Send 1/2: [/oscjoy.0.axis.1]=1.000000

If you prefer to have sent joystick values range from 
[-1.0f,+1.0f] instead of the default [0.0f,+1.0f] range, 
you can specify the "zerocenter" keyword as a command line
argument.

All values sent by oscjoy are sent as floating point values.
For buttons, 1.0f is sent to indicate that a button has been
pressed, and 0.0f is sent to indicate that a button has
been released.

oscjoy will group multiple commands inside an OSC bundle if
possible, to cut down on the number of UDP packets that
must be sent.

History
-------

v1.10  12/27/2007 - Added the "axis", "button", "hat", and "ball"
                    control-aliasing arguments, per Martin
                    Ahnelov's request.

v1.00  3/26/2006  - Initial Release

