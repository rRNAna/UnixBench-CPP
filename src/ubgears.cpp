/**
 * @file        ubgears.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     2.0.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of ubgears.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <sched.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

using namespace std;

// If M_PI is not defined in the system, define it
#ifndef M_PI
constexpr double M_PI = 3.14159265;
#endif

// Macro conversion: Convert NULL pointer to empty string
#define NULLSTR(x) (((x) != nullptr) ? (x) : (""))
// If verbose output is enabled, the output log
#define Log(x) { if (verbose) std::cout << x; }
// Message Printing Macro
#define Msg(x) { std::cout << x; }

// Global variables
static const char *ProgramName = nullptr;
static bool verbose = false;       // verbose mode (default false)
static int runTime = -1;           // Run time (microseconds), -1 means unlimited
static struct timeval clockStart;  // Record start time

// Variables for OpenGL rendering
static GLfloat view_rotx = 20.0f, view_roty = 30.0f, view_rotz = 0.0f;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0f;
static GLint speed = 60;
static GLboolean printInfo = GL_FALSE;

//==========================================================
// Time processing function
//==========================================================

// Record start time
static void start_time(void) {
    gettimeofday(&clockStart, nullptr);
}

// Returns the number of microseconds that have passed since start_time() was called.
static long current_time(void) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    long secs = tv.tv_sec - clockStart.tv_sec;
    long micros = tv.tv_usec - clockStart.tv_usec;
    if (micros < 0) {
        --secs;
        micros += 1000000;
    }
    return secs * 1000000 + micros;
}

//==========================================================
// usage: print help information and exit
//==========================================================
static void usage(void) {
    std::cerr << "usage: " << ProgramName << " [options]\n";
    std::cerr << "-display\tSet X11 display for output.\n";
    std::cerr << "-info\t\tPrint additional GLX information.\n";
    std::cerr << "-time t\t\tRun for t seconds and report performance.\n";
    std::cerr << "-h\t\tPrint this help page.\n";
    std::cerr << "-v\t\tVerbose output.\n\n";
    exit(EXIT_FAILURE);
}

//==========================================================
// OpenGL gears drawing functions
//==========================================================

/*
 * Draw a gear wheel.
 * Inputs:
 *  inner_radius - radius of the central hole
 *  outer_radius - radius at center of teeth
 *  width        - width of gear
 *  teeth        - number of teeth
 *  tooth_depth  - depth of tooth
 */
static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
                 GLint teeth, GLfloat tooth_depth) {
    GLint i;
    GLfloat r0, r1, r2, maxr2, minr2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0f;
    maxr2 = r2 = outer_radius + tooth_depth / 2.0f;
    minr2 = r2;
    da = 2.0f * M_PI / teeth / 4.0f;

    glShadeModel(GL_FLAT);
    glNormal3f(0.0f, 0.0f, 1.0f);

    //
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0f * M_PI / teeth;
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5f);
        if (i < teeth) {
            glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
            glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5f);
        }
    }
    glEnd();

    //
    glBegin(GL_QUADS);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0f * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5f);
        r2 = minr2;
    }
    r2 = maxr2;
    glEnd();

    glNormal3f(0.0f, 0.0f, -1.0f);

    //
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0f * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
        if (i < teeth) {
            glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
            glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
        }
    }
    glEnd();

    //
    glBegin(GL_QUADS);
    da = 2.0f * M_PI / teeth / 4.0f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0f * M_PI / teeth;
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
        r2 = minr2;
    }
    r2 = maxr2;
    glEnd();

    //
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0f * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5f);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5f);
        u = r2 * cos(angle + da) - r1 * cos(angle);
        v = r2 * sin(angle + da) - r1 * sin(angle);
        len = sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5f);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5f);
        glNormal3f(cos(angle + 1.5 * da), sin(angle + 1.5 * da), 0.0f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5f);
        u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
        v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
        glNormal3f(v, -u, 0.0f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5f);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5f);
        glNormal3f(cos(angle + 3.5 * da), sin(angle + 3.5 * da), 0.0f);
        r2 = minr2;
    }
    r2 = maxr2;
    glVertex3f(r1 * cos(0.0f), r1 * sin(0.0f), width * 0.5f);
    glVertex3f(r1 * cos(0.0f), r1 * sin(0.0f), -width * 0.5f);
    glEnd();

    glShadeModel(GL_SMOOTH);

    //
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
       angle = i * 2.0f * M_PI / teeth;
       glNormal3f(-cos(angle), -sin(angle), 0.0f);
       glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5f);
       glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5f);
    }
    glEnd();
}

static void draw(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
       glRotatef(view_rotx, 1.0f, 0.0f, 0.0f);
       glRotatef(view_roty, 0.0f, 1.0f, 0.0f);
       glRotatef(view_rotz, 0.0f, 0.0f, 1.0f);

       glPushMatrix();
          glTranslatef(-3.0f, -2.0f, 0.0f);
          glRotatef(angle, 0.0f, 0.0f, 1.0f);
          glCallList(gear1);
       glPopMatrix();

       glPushMatrix();
          glTranslatef(3.1f, -2.0f, 0.0f);
          glRotatef(-2.0f * angle - 9.0f, 0.0f, 0.0f, 1.0f);
          glCallList(gear2);
       glPopMatrix();

       glPushMatrix();
          glTranslatef(-3.1f, 4.2f, 0.0f);
          glRotatef(-2.0f * angle - 25.0f, 0.0f, 0.0f, 1.0f);
          glCallList(gear3);
       glPopMatrix();
    glPopMatrix();
}

static void reshape(int width, int height) {
    GLfloat h = static_cast<GLfloat>(height) / static_cast<GLfloat>(width);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (h >= 1.0f)
       glFrustum(-1.0f, 1.0f, -h, h, 5.0f, 60.0f);
    else
       glFrustum(-1.0f/h, 1.0f/h, -1.0f, 1.0f, 5.0f, 60.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -40.0f);
}

static void init(void) {
    static GLfloat pos[4] = { 5.0f, 5.0f, 10.0f, 0.0f };
    static GLfloat red[4]   = { 0.8f, 0.1f, 0.0f, 1.0f };
    static GLfloat green[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
    static GLfloat blue[4]  = { 0.2f, 0.2f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    gear1 = glGenLists(1);
    glNewList(gear1, GL_COMPILE);
       glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
       gear(1.0f, 4.0f, 1.0f, 20, 0.7f);
    glEndList();

    gear2 = glGenLists(1);
    glNewList(gear2, GL_COMPILE);
       glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
       gear(0.5f, 2.0f, 2.0f, 10, 0.7f);
    glEndList();

    gear3 = glGenLists(1);
    glNewList(gear3, GL_COMPILE);
       glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
       gear(1.3f, 2.0f, 0.5f, 10, 0.7f);
    glEndList();

    glEnable(GL_NORMALIZE);
}

// Creates an RGB double-buffered window and returns the window and GLX context handles.
static void make_window(Display *dpy, Screen *scr,
                        const char *name,
                        int x, int y, int width, int height,
                        Window *winRet, GLXContext *ctxRet) {
    int attrib[] = { GLX_RGBA,
                     GLX_RED_SIZE, 1,
                     GLX_GREEN_SIZE, 1,
                     GLX_BLUE_SIZE, 1,
                     GLX_DOUBLEBUFFER,
                     GLX_DEPTH_SIZE, 1,
                     None };
    int scrnum = XScreenNumberOfScreen(scr);
    Window root = XRootWindow(dpy, scrnum);
    XSetWindowAttributes attr;
    unsigned long mask;
    Window win;
    GLXContext ctx;
    XVisualInfo *visinfo = glXChooseVisual(dpy, scrnum, attrib);
    if (!visinfo) {
       std::cerr << ProgramName << ": Error: couldn't get an RGB, Double-buffered visual.\n";
       exit(EXIT_FAILURE);
    }

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow(dpy, root, x, y, width, height,
                        0, visinfo->depth, InputOutput,
                        visinfo->visual, mask, &attr);

    {
       XSizeHints sizehints;
       sizehints.x = x;
       sizehints.y = y;
       sizehints.width  = width;
       sizehints.height = height;
       sizehints.flags = USSize | USPosition;
       XSetNormalHints(dpy, win, &sizehints);
       XSetStandardProperties(dpy, win, name, name,
                              None, (char **)nullptr, 0, &sizehints);
    }

    ctx = glXCreateContext(dpy, visinfo, nullptr, True);
    if (!ctx) {
       std::cerr << ProgramName << ": Error: glXCreateContext failed.\n";
       exit(EXIT_FAILURE);
    }
    XFree(visinfo);

    XMapWindow(dpy, win);
    glXMakeCurrent(dpy, win, ctx);

    GLint max[2] = { 0, 0 };
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, max);
    if (printInfo)
       std::cout << "GL_MAX_VIEWPORT_DIMS=" << max[0] << "/" << max[1] << "\n";
    if (width > max[0] || height > max[1]) {
       std::cerr << ProgramName << ": Error: Requested window size (" << width << "/" << height
                 << ") larger than maximum supported by GL engine (" << max[0] << "/" << max[1] << ").\n";
       exit(EXIT_FAILURE);
    }

    *winRet = win;
    *ctxRet = ctx;
}

static void event_loop(Display *dpy, Window win) {
    while (true) {
       while (XPending(dpy) > 0) {
          XEvent event;
          XNextEvent(dpy, &event);
          switch (event.type) {
             case Expose:
                Log(("Event: Expose\n"));
                // 重绘将在下方进行
                break;
             case ConfigureNotify:
                Log(("Event: ConfigureNotify\n"));
                reshape(event.xconfigure.width, event.xconfigure.height);
                break;
          }
       }

       // Timing and rendering control variables
       static long startTime = 0;
       static long lastFrame = 0;
       static long lastFps = 0;
       static int frames = 0;
       static long runFrames = 0;
       long t = current_time();
       long useconds = t - lastFrame;
       if (useconds == 0)
          useconds = 10000;  // 默认设定 10000 微秒

       angle += (static_cast<double>(speed) * useconds) / 1000000.0;
       if (angle > 360.0)
          angle -= 360.0;
       draw();
       glXSwapBuffers(dpy, win);

       lastFrame = t;
       frames++;

       if (t - lastFps >= 5000000L) {
          GLfloat seconds = (t - lastFps) / 1000000.0f;
          GLfloat fps = frames / seconds;
          std::cout << frames << " frames in " << seconds << " seconds = " << fps << " FPS\n";
          lastFps = t;
          frames = 0;
          if (runTime > 0 && startTime == 0) {
             std::cout << "Start timing!\n";
             startTime = t;
          }
       }
       if (startTime > 0)
          ++runFrames;
       if (runTime > 0 && startTime > 0 && t - startTime > runTime) {
          double timeSec = (t - startTime) / 1000000.0;
          std::cerr << "COUNT|" << runFrames << "|1|fps\n";
          std::cerr << "TIME|" << timeSec << "\n";
          exit(0);
       }

       // Give up the CPU
       sched_yield();
    }
}

int main(int argc, char *argv[]) {
    bool use_threadsafe_api = false;
    Display *dpy;
    Window win;
    Screen *screen;
    GLXContext ctx;
    const char *dpyName = nullptr;
    int i;
    XRectangle winrect;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
       const char *arg = argv[i];
       int len = std::strlen(arg);

       if (std::strcmp(argv[i], "-display") == 0) {
          if (++i >= argc)
             usage();
          dpyName = argv[i];
       }
       else if (std::strcmp(argv[i], "-info") == 0) {
          printInfo = GL_TRUE;
       }
       else if (std::strcmp(argv[i], "-time") == 0) {
          if (++i >= argc)
             usage();
          runTime = std::atoi(argv[i]) * 1000000;
       }
       else if (!std::strncmp("-v", arg, len)) {
          verbose = true;
          printInfo = GL_TRUE;
       }
       else if (!std::strncmp("-debug_use_threadsafe_api", arg, len)) {
          use_threadsafe_api = true;
       }
       else if (std::strcmp(argv[i], "-h") == 0) {
          usage();
       }
       else {
          std::cerr << ProgramName << ": Unsupported option '" << argv[i] << "'.\n";
          usage();
       }
    }

    if (use_threadsafe_api) {
       if (!XInitThreads()) {
          std::cerr << ProgramName << ": XInitThreads() failure.\n";
          exit(EXIT_FAILURE);
       }
    }

    dpy = XOpenDisplay(dpyName);
    if (!dpy) {
       std::cerr << ProgramName << ": Error: couldn't open display '" << dpyName << "'\n";
       return EXIT_FAILURE;
    }

    screen = XDefaultScreenOfDisplay(dpy);

    winrect.x = 0;
    winrect.y = 0;
    winrect.width = 300;
    winrect.height = 300;

    Log(("Window x=%d, y=%d, width=%d, height=%d\n",
         (int)winrect.x, (int)winrect.y, (int)winrect.width, (int)winrect.height));

    make_window(dpy, screen, "ubgears", winrect.x, winrect.y, winrect.width, winrect.height, &win, &ctx);
    reshape(winrect.width, winrect.height);

    if (printInfo) {
       std::cout << "GL_RENDERER   = " << (const char *)glGetString(GL_RENDERER) << "\n";
       std::cout << "GL_VERSION    = " << (const char *)glGetString(GL_VERSION) << "\n";
       std::cout << "GL_VENDOR     = " << (const char *)glGetString(GL_VENDOR) << "\n";
       std::cout << "GL_EXTENSIONS = " << (const char *)glGetString(GL_EXTENSIONS) << "\n";
    }

    init();
    start_time();
    event_loop(dpy, win);

    glXDestroyContext(dpy, ctx);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return EXIT_SUCCESS;
}