#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
# include <windows.h>
# include <winsock.h>
# include <signal.h>
#else
# include <sys/types.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# if defined(__BEOS__)
#  include <signal.h>
# elif defined(__CYGWIN__)
#  include <signal.h>
#  include <sys/signal.h>
#  include <sys/times.h>
# elif defined(__QNX__)
#  include <signal.h>
#  include <sys/times.h>
# elif defined(SUN) || defined(__sparc__) || defined(sun386)
#  include <signal.h>
#  include <sys/times.h>
#  include <limits.h>
# else
#  include <sys/signal.h>  // changed signal.h to sys/signal.h to work with OS/X
#  include <sys/times.h>
# endif
#endif

#include "SDL_joystick.h"
#include "SDL_events.h"

static const char * _version = "1.10";
static const char * _prefix  = "oscjoy";

#define MAX_PENDING_UPDATES  128
#define MAX_CONTROL_NAME_LEN 128

static unsigned long _pendingUpdateCount = 0;
static char _pendingUpdateKeys[MAX_PENDING_UPDATES][MAX_CONTROL_NAME_LEN];
static float _pendingUpdateValues[MAX_PENDING_UPDATES];
static bool _debug = false;

#ifdef WIN32
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

static const int MAX_NUM_ALIASES = 256;
static unsigned long _aliasFrom[MAX_NUM_ALIASES];
static unsigned long _aliasTo[MAX_NUM_ALIASES];
static const char * _aliasName[MAX_NUM_ALIASES];
static int _numAliases = 0;
static uint64 _updateDelay = 10000;  // default to updating every 10ms
static bool _zerocenter = false;

static unsigned long CreateAliasCombo(int joyIdx, int subIdx, int axisIdx)
{
   unsigned long jc = (joyIdx  < 0) ? 0xFF : joyIdx;
   unsigned long sc = (subIdx  < 0) ? 0xFF : subIdx;
   unsigned long ac = (axisIdx < 0) ? 0xFF : axisIdx;
   return (jc<<16)|(sc<<8)|(ac<<0);
}

static void DecodeAliasCombo(unsigned long combo, int & joyIdx, int & subIdx, int & axisIdx)
{
   unsigned long jc = (combo >> 16) & 0xFF;
   unsigned long sc = (combo >>  8) & 0xFF;
   unsigned long ac = (combo >>  0) & 0xFF;
   joyIdx  = (jc == 0xFF) ? -1 : (int)jc;
   subIdx  = (sc == 0xFF) ? -1 : (int)sc;
   axisIdx = (ac == 0xFF) ? -1 : (int)ac;
}

static int ParseAxis(const char * s)
{
   switch(*s)
   {
      case 'x':  case 'X': return 0;
      case 'y':  case 'Y': return 1;
      case 'z':  case 'Z': return 2;
      default:   return atoi(s);
   }
}

static const char * AxisToString(int whichAxis, bool forDisplay)
{
   static char buf[32];
        if (whichAxis < 0) return "?";
   else if (whichAxis < 3) sprintf(buf, "%c", whichAxis+(forDisplay?'X':'x'));
   else                    sprintf(buf, "%s%i", forDisplay?"#":"", whichAxis);
   return buf;
}

static void ParseControlAliasString(bool isTo, const char * ctlType, const char * s, int & retJoy, int & retSub, int & retAxis)
{
   const char * firstDot = strchr(s, '.');
   if (firstDot)
   {
      const char * secondDot = firstDot ? strchr(firstDot+1, '.') : NULL;
      if (secondDot)
      {
         retJoy  = atoi(s);
         retSub  = atoi(firstDot+1);
         retAxis = ParseAxis(secondDot+1);
      }
      else
      {
         retJoy  = atoi(s);
         retSub  = atoi(firstDot+1);
         retAxis = -1;
      }
   }
   else {retSub = atoi(s); retJoy = 0; retAxis = -1;}

   if (isTo) printf("will be sent as ");
        else printf("-> Alias added: ");

   char c = ctlType[0]; c = ((c>='a')&&(c<='z'))?(c-('a'-'A')):c;  // capitalize the first letter of ctlType
   if (retAxis >= 0) printf("(Joystick #%i, %c%s #%i, Axis %s) ", retJoy, c, ctlType+1, retSub, AxisToString(retAxis, true));
                else printf("(Joystick #%i, %c%s #%i) ", retJoy, c, ctlType+1, retSub);

   if (isTo) printf("\n");
}

static void ParseAliasArgs(int argc, char ** argv)
{
   static const char * _types[] = {"axis", "hat", "ball", "button"};
   for (unsigned int j=0; j<sizeof(_types)/sizeof(_types[0]); j++)
   {
      const char * ctlType = _types[j];
      int tlen = strlen(ctlType);
      for (int i=0; i<argc; i++)
      {
         char * a = argv[i];
         if ((strncmp(a, ctlType, tlen) == 0)&&(a[tlen] == ':'))
         {
            char * aliasFrom = a+tlen+1;
            char * aliasTo   = strchr(aliasFrom, '=');
            if (aliasTo)
            {
               if (_numAliases == MAX_NUM_ALIASES)
               {
                  printf("Maximum number of aliases (%i) reached, ignoring additonal aliases.\n", _numAliases);
                  return;
               }

               *aliasTo = '\0'; aliasTo++;
               int fromWhichJoy, fromWhichSub, fromWhichAxis, toWhichJoy, toWhichSub, toWhichAxis;
               ParseControlAliasString(false, ctlType, aliasFrom, fromWhichJoy, fromWhichSub, fromWhichAxis);
               ParseControlAliasString(true,  ctlType, aliasTo,   toWhichJoy,   toWhichSub,   toWhichAxis);
               _aliasFrom[_numAliases] = CreateAliasCombo(fromWhichJoy, fromWhichSub, fromWhichAxis);
               _aliasTo[_numAliases]   = CreateAliasCombo(toWhichJoy,   toWhichSub,   toWhichAxis);
               _aliasName[_numAliases] = ctlType;
               _numAliases++;
            }
            else printf("Warning, unable to process alias arg [%s]:  should take the form %s:fromIndices=toIndices\n", a, ctlType);
         }
      }
   }
}

static void AddOSCFloat(const char * ctlName, int whichJoy, int whichSub, int whichAxis, float value)
{
   // New for v1.10:  See if this combination of parameters should be aliased to something else,
   // and if so, convert the parameters to their new values
   {
      unsigned long combo = CreateAliasCombo(whichJoy, whichSub, whichAxis);
      for (int i=0; i<_numAliases; i++)
      {
         if ((_aliasFrom[i] == combo)&&(strcmp(ctlName, _aliasName[i]) == 0))
         {
            DecodeAliasCombo(_aliasTo[i], whichJoy, whichSub, whichAxis);
            break;
         }
      }
   }

   char buf[MAX_CONTROL_NAME_LEN]; 
   if (whichAxis >= 0) sprintf(buf, "/%s%s%i.%s.%i.%s", _prefix, _prefix[0]?".":"", whichJoy, ctlName, whichSub, AxisToString(whichAxis, false));
                  else sprintf(buf, "/%s%s%i.%s.%i", _prefix, _prefix[0]?".":"", whichJoy, ctlName, whichSub);
   for (unsigned long i=0; i<_pendingUpdateCount; i++)
   {
      const char * next = _pendingUpdateKeys[i];
      if (strcmp(next, buf) == 0) 
      {
         _pendingUpdateValues[i] = value;
         return;
      }
   }
   if (_pendingUpdateCount < MAX_PENDING_UPDATES)
   {
      strcpy(_pendingUpdateKeys[_pendingUpdateCount], buf);
      _pendingUpdateValues[_pendingUpdateCount++] = value;
   }
}

extern "C" {

SDL_EventFilter SDL_EventOK = NULL;

Uint8 SDL_EventState (Uint8 /*type*/, int /*state*/)
{
   return SDL_ENABLE;
}

Uint8 SDL_ProcessEvents[SDL_NUMEVENTS];

int SDL_PushEvent(SDL_Event *event)
{
   switch (event->type) 
   {
      case SDL_JOYAXISMOTION:
      {
         float normVal = ((float)event->jaxis.value)/32768.0f;
         if (_zerocenter == false) normVal = (normVal+1.0f)/2.0f;
         AddOSCFloat("axis", event->jaxis.which, event->jaxis.axis, -1, normVal);  // range=[0,1]
      }
      break;

      case SDL_JOYHATMOTION:
      {
         float hatX=0.0f, hatY=0.0f;
         if (event->jhat.value & SDL_HAT_UP)    hatY=-1.0f;
         if (event->jhat.value & SDL_HAT_RIGHT) hatX=-1.0f;
         if (event->jhat.value & SDL_HAT_DOWN)  hatY=1.0f;
         if (event->jhat.value & SDL_HAT_LEFT)  hatX=1.0f;
         if (_zerocenter == false)
         {
            hatX = (hatX+1.0f)/2.0f;
            hatY = (hatY+1.0f)/2.0f;
         }
         AddOSCFloat("hat", event->jhat.which, event->jhat.hat, 0, hatX);
         AddOSCFloat("hat", event->jhat.which, event->jhat.hat, 1, hatY);
      }
      break;

      case SDL_JOYBALLMOTION:
         AddOSCFloat("ball", event->jball.which, event->jball.ball, 0, ((float)event->jball.xrel)/32767.0f);
         AddOSCFloat("ball", event->jball.which, event->jball.ball, 1, ((float)event->jball.yrel)/32767.0f);
      break;

      case SDL_JOYBUTTONDOWN:
         AddOSCFloat("button", event->jbutton.which, event->jbutton.button, -1, 1.0f);
      break;

      case SDL_JOYBUTTONUP:
         AddOSCFloat("button", event->jbutton.which, event->jbutton.button, -1, 0.0f);
      break;
   }
   return 0;
}

};

#ifndef WIN32

/** Given a uint64, writes the equivalent timeval struct into the second argument.
 *  @param val A uint64 time value in microseconds
 *  @param retStruct On return, contains the equivalent timeval struct to (val)
 */
static inline void Convert64ToTimeVal(uint64 val, struct timeval & retStruct)
{
   retStruct.tv_sec  = (long)(val / 1000000);
   retStruct.tv_usec = (long)(val % 1000000);
}

#endif

static void Snooze64(uint64 micros)
{
#if __BEOS__
   (void) snooze(micros);
#elif __ATHEOS__
   (void) snooze(micros);
#elif WIN32
   Sleep((DWORD)((micros/1000)+(((micros%1000)!=0)?1:0)));
#else
   /** We can use select(), if nothing else */
   struct timeval waitTime;
   Convert64ToTimeVal(micros, waitTime);
   (void) select(0, NULL, NULL, NULL, &waitTime);
#endif
}

static inline void swapChars(char & a, char & b) {char t=a; a=b; b=t;}

void WatchJoysticks(int sock, SDL_Joystick **joysticks, int numJoysticks)
{
   char * buf = new char[MAX_PENDING_UPDATES*MAX_CONTROL_NAME_LEN*2];
   const bool isBigEndian = (htonl(1)==1);

   /* Print info about the joystick we are watching */
   for (int i=0; i<numJoysticks; i++)
   {
      const char * name = SDL_JoystickName(SDL_JoystickIndex(joysticks[i]));
      printf("-> Watching joystick %d: (%s)\n", SDL_JoystickIndex(joysticks[i]), name ? name : "Unknown Joystick");
      printf("-> Joystick has %d axes, %d hats, %d balls, and %d buttons\n", SDL_JoystickNumAxes(joysticks[i]), SDL_JoystickNumHats(joysticks[i]), SDL_JoystickNumBalls(joysticks[i]),SDL_JoystickNumButtons(joysticks[i]));
   }

   /* Loop, getting joystick events! */
   while(1)
   {
      SDL_JoystickUpdate();
      if (_pendingUpdateCount > 0)
      {
         char * p = buf;
         if (_pendingUpdateCount > 1) {memcpy(p, "#bundle\0\0\0\0\0\0\0\0\1", 16); p += 16;}

         for (unsigned long i=0; i<_pendingUpdateCount; i++) 
         {
            if (_debug) printf("Send %lu/%lu: [%s]=%f\n", i, _pendingUpdateCount, _pendingUpdateKeys[i], _pendingUpdateValues[i]);
            unsigned long * itemSizePtr = (unsigned long *)((_pendingUpdateCount > 1) ? p : NULL);  // we will populate this later
            if (itemSizePtr) p += 4;

            // Send the control name
            int keylen = strlen(_pendingUpdateKeys[i])+1; 
            memcpy(p, _pendingUpdateKeys[i], keylen); p += keylen;
            while((p-buf)%4) *p++ = '\0';   // OSC requires that we pad strings to a multiple of 4 bytes!

            // Send the type string
            memcpy(p, ",f\0\0", 4); p += 4;  // argument-type-string (arguments are always in float format)

            // Send the value
            memcpy(p, &_pendingUpdateValues[i], 4);
            if (isBigEndian == false)
            {
               // my equivalent of htonf()
               swapChars(p[0], p[3]);
               swapChars(p[1], p[2]);
            }
            p += 4;

            if (itemSizePtr) *itemSizePtr = htonl(p-(((char *)itemSizePtr)+4));
         }

         unsigned long packetLen = p-buf;
#ifdef DEBUG_OSC_FORMAT
         if (_debug) 
         {
            for (unsigned long x=0; x<packetLen; x++) {if (x%8==0) printf("\n"); printf("%02x(%c) ", buf[x], buf[x]);} 
            printf("\n");
         }
#endif
         if (send(sock, buf, packetLen, 0) != (int)packetLen) perror("Error sending UDP data");

         _pendingUpdateCount = 0;
      }
      Snooze64(_updateDelay);
   }
   delete [] buf;
}

static unsigned long GetHostByName(const char * name)
{
   unsigned long ret = inet_addr(name);  // first see if we can parse it as a numeric address
   if ((ret == 0)||(ret == ((unsigned long)-1)))
   {
      struct hostent * he = gethostbyname(name);
      ret = ntohl(he ? *((unsigned long*)he->h_addr) : 0);
   }
   else ret = ntohl(ret);

   return ret;
}

static void Inet_NtoA(unsigned long addr, char * ipbuf)
{  
   sprintf(ipbuf, "%li.%li.%li.%li", (addr>>24)&0xFF, (addr>>16)&0xFF, (addr>>8)&0xFF, (addr>>0)&0xFF);
}

// defaultVal is the value to return if the keyword isn't found
// implicitVal is the value to return if the keyword is found but no value was specified
static const char * ParseArg(int argc, char ** argv, const char * key, const char * defaultVal, const char * implicitVal)
{
   for (int i=2; i<argc; i++)  // argv[0] and argv[1] have special meanings so we don't parse them
   {
      const char * equals = strchr(argv[i], '=');
      if ((equals ? strncmp(argv[i], key, equals-argv[i]) : strcmp(argv[i], key)) == 0) return equals ? (equals+1) : implicitVal;
   }
   return defaultVal;
}

/* Source code stolen from UNIX Network Programming, Volume 1
 * Comments from the Unix FAQ
 */
#ifndef WIN32
static int SpawnDaemonProcess(bool & returningAsParent, const char * optNewDir, const char * optOutputTo, bool createIfNecessary)
{
   // Here are the steps to become a daemon:
   // 1. fork() so the parent can exit, this returns control to the command line or shell invoking
   //    your program. This step is required so that the new process is guaranteed not to be a process
   //    group leader. The next step, setsid(), fails if you're a process group leader.
   pid_t pid = fork();
   if (pid < 0) return -1;
   if (pid > 0) 
   {
      returningAsParent = true;
      return 0;
   }
   else returningAsParent = false; 

   // 2. setsid() to become a process group and session group leader. Since a controlling terminal is
   //    associated with a session, and this new session has not yet acquired a controlling terminal
   //    our process now has no controlling terminal, which is a Good Thing for daemons.
   setsid();

   // 3. fork() again so the parent, (the session group leader), can exit. This means that we, as a
   //    non-session group leader, can never regain a controlling terminal.
   signal(SIGHUP, SIG_IGN);
   pid = fork();
   if (pid < 0) return -1;
   if (pid > 0) exit(0);

   // 4. chdir("/") can ensure that our process doesn't keep any directory in use. Failure to do this
   //    could make it so that an administrator couldn't unmount a filesystem, because it was our
   //    current directory. [Equivalently, we could change to any directory containing files important
   //    to the daemon's operation.]
   if (optNewDir) chdir(optNewDir);

   // 5. umask(0) so that we have complete control over the permissions of anything we write.
   //    We don't know what umask we may have inherited. [This step is optional]
   umask(0);

   // 6. close() fds 0, 1, and 2. This releases the standard in, out, and error we inherited from our parent
   //    process. We have no way of knowing where these fds might have been redirected to. Note that many
   //    daemons use sysconf() to determine the limit _SC_OPEN_MAX. _SC_OPEN_MAX tells you the maximun open
   //    files/process. Then in a loop, the daemon can close all possible file descriptors. You have to
   //    decide if you need to do this or not. If you think that there might be file-descriptors open you should
   //    close them, since there's a limit on number of concurrent file descriptors.
   // 7. Establish new open descriptors for stdin, stdout and stderr. Even if you don't plan to use them,
   //    it is still a good idea to have them open. The precise handling of these is a matter of taste;
   //    if you have a logfile, for example, you might wish to open it as stdout or stderr, and open `/dev/null'
   //    as stdin; alternatively, you could open `/dev/console' as stderr and/or stdout, and `/dev/null' as stdin,
   //    or any other combination that makes sense for your particular daemon.
   int nullfd = open("/dev/null", O_RDWR);
   if (nullfd >= 0) dup2(nullfd, STDIN_FILENO);

   int outfd = -1;
   if (optOutputTo) 
   {
      outfd = open(optOutputTo, O_WRONLY | (createIfNecessary ? O_CREAT : 0));
      if (outfd < 0) printf("BecomeDaemonProcess():  Couldn't open %s to redirect stdout, stderr\n", optOutputTo);
   }
   if (outfd >= 0) dup2(outfd, STDOUT_FILENO);
   if (outfd >= 0) dup2(outfd, STDERR_FILENO);

   return 0;
}

static int BecomeDaemonProcess(const char * optNewDir = NULL, const char * optOutputTo = "/dev/null", bool createIfNecessary = true)
{
   bool isParent;
   int ret = SpawnDaemonProcess(isParent, optNewDir, optOutputTo, createIfNecessary);
   if ((ret == 0)&&(isParent)) exit(0);
   return ret;
}
#endif

int main(int argc, char *argv[])
{
   int i, numJoysticks, numJoysticksOpened = 0;
   SDL_Joystick * joysticks[256];
   unsigned long targetIP;
   int targetport = 14005;  // default to the port LCS's mixerd listens on
   int sourceport = atoi(ParseArg(argc, argv, "sourceport", "8000", "8000"));  // what the Lemur uses
   char ipbuf[16];
   char * colon;

   if (argc < 2)
   {
      printf("oscjoy v%s.  Compiled on %s\n", _version, __DATE__);
      printf("Usage:  ./oscjoy target.ip.address[:targetport] [prefix=oscjoy] [sourceport=8000] [updaterate=100Hz] [axis:ASPEC=ASPEC] [hat:ASPEC=ASPEC] [ball:ASPEC=ASPEC] [button:ASPEC=ASPEC] [zerocenter] [debug] [daemon]\n");
      printf("(where ASPEC can be of the form joy.ctl.axis, joy.ctl, or just ctl.\n");
      printf(" for example, axis:0=3 would make it so that when the first joystick's first axis moved, oscjoy would\n");
      printf(" send out OSC packets saying that the first joystick's fourth axis had moved.  Up to %i simultaneous\n", MAX_NUM_ALIASES);
      printf(" aliases may be specified at once)\n");
      return 5;
   }

   colon = strchr(argv[1], ':');
   if (colon)
   {
      targetport = atoi(colon+1);
      if (targetport == 0)
      {
         printf("Couldn't parse targetport [%s]\n", colon+1);
         return 10;
      }
      *colon = '\0';
   }
   
   _debug = (atoi(ParseArg(argc, argv, "debug", "0", "1")) != 0);
   if (_debug) printf("-> Debug output enabled.\n");

   ParseAliasArgs(argc, argv);

#ifdef WIN32
   WORD versionWanted = MAKEWORD(1, 1);
   WSADATA wsaData;
   int ret = WSAStartup(versionWanted, &wsaData);
   if (ret != 0)
   {
      printf("Couldn't initialize Winsock!");
      return 0;
   }
#endif

   _prefix = ParseArg(argc, argv, "prefix", "oscjoy", "");
   if (_debug) printf("-> OSC control name prefix set to [%s]\n", _prefix);

   _zerocenter = (atoi(ParseArg(argc, argv, "zerocenter", "0", "1")) != 0);
   if (_zerocenter) printf("-> Zero-centered axes enabled.\n");
 
   long updaterate = atol(ParseArg(argc, argv, "updaterate", "100", "100"));
   if (updaterate > 1000) updaterate = 1000;
   if (updaterate < 1) updaterate = 1;
   _updateDelay = 1000000/updaterate;

   memset(SDL_ProcessEvents,SDL_ENABLE,sizeof(SDL_ProcessEvents));
   (void) SDL_JoystickInit();

   targetIP = GetHostByName(argv[1]);
   if (targetIP == 0)
   {
      printf("Couldn't parse IP address [%s]\n", argv[1]);
      return 10;
   }

   Inet_NtoA(targetIP, ipbuf);
   printf("-> OSC target is %s:%i\n-> Local UDP port is %i\n-> Update rate is %liHz\n", ipbuf, targetport, sourceport, updaterate);

   int sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0)
   {
      printf("Error creating UDP socket!\n");
      return 10;
   }

   // Try to bind our socket to port 8000, since otherwise mixerd won't recognize the port
   if (sourceport > 0)
   {
      struct sockaddr_in saSocket;
      memset(&saSocket, 0, sizeof(saSocket));
      saSocket.sin_family      = AF_INET;
      saSocket.sin_addr.s_addr = htonl(INADDR_ANY);
      saSocket.sin_port        = htons(sourceport);
      if (bind(sock, (struct sockaddr *) &saSocket, sizeof(saSocket)) != 0) printf("Warning, unable to bind socket to port %i\n", sourceport);
   }

   {
      struct sockaddr_in saAddr;
      memset(&saAddr, 0, sizeof(saAddr));
      saAddr.sin_family      = AF_INET;
      saAddr.sin_port        = htons(targetport);
      saAddr.sin_addr.s_addr = htonl(targetIP); 
      if (connect(sock, (struct sockaddr *) &saAddr, sizeof(saAddr)) != 0)
      {
         printf("Error connecting UDP socket!\n");
         return 10;
      }
   }

   /* Print information about the joysticks */
   numJoysticks = SDL_NumJoysticks();
   printf("-> There are %d joysticks attached\n", numJoysticks);
   for (i=0; i<numJoysticks; ++i)
   {
      const char * name = SDL_JoystickName(i);
      joysticks[numJoysticksOpened] = SDL_JoystickOpen(i);
      printf("-> Joystick %d: %s was %s.\n",i, name?name:"Unknown Joystick", joysticks[i] ? "opened" : "not opened");
      if (joysticks[numJoysticksOpened]) numJoysticksOpened++;
   }

   if (numJoysticksOpened > 0)
   {
      if (atoi(ParseArg(argc, argv, "daemon", "0", "1"))==1)
      {
#ifdef WIN32
         printf("Sorry, the daemon option isn't supported under Windows.\n");
#else
         printf("-> Spawning daemon process...\n"); 
         if (BecomeDaemonProcess() != 0) printf("Error, couldn't become a daemon process!\n");
#endif
      }

      WatchJoysticks(sock, joysticks, numJoysticksOpened);
      for (i=0; i<numJoysticksOpened; i++) SDL_JoystickClose(joysticks[i]);
   }
   else {printf("No Joysticks available to watch, exiting\n"); return 5;}

#ifdef WIN32
   closesocket(sock);
   WSACleanup();
#else
   close(sock);
#endif

   return(0);
}
