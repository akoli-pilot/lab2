//
//modified by: Phillip Lakes (Koli)
//date: 01/22/2025
//
//original author: Gordon Griesel
//date:            2025
//purpose:         OpenGL sample program
//
//This program needs some refactoring.
//We will do this in class together.
//
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <algorithm>

//some structures

class Global {
public:
	float w;
	float dir;
	float dir_y;
	float pos[2];
	int xres, yres;
	struct timeval bounce_freq;
    float bounce_itval;
	Global();
} g;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);


int main()
{
	init_opengl();
	int done = 0;
	//main game loop
	while (!done) {
		//look for external events such as keyboard, mouse.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	return 0;
}

Global::Global()
{
	xres = 400;
	yres = 200;
	w = 20.0f;
	dir = 30.0f;
	dir_y = 15.0f;
	pos[0] = {0.0f+w};
	pos[1] = {g.yres/2.0f};
	gettimeofday(&bounce_freq, NULL);
    bounce_itval = 0.0f;
}

X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	}
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab-1");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//Window has been resized.
	g.xres = width;
	g.yres = height;
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed.
			//int y = g.yres - e->xbutton.y;
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_a:
				//the 'a' key was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
			case XK_Up:
                g.dir *= 1.2f;
                g.dir_y *= 1.2f;
                break;
            case XK_Down:
                g.dir *= 0.8f;
                g.dir_y *= 0.8f;
                break;
		}
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics()
{
	// Horizontal
	g.pos[0] += g.dir;

	// L+R
	if (g.pos[0] >= (g.xres-g.w)) {
		g.pos[0] = (g.xres-g.w);
		g.dir = -g.dir;

		struct timeval current_time;
        gettimeofday(&current_time, NULL);
        g.bounce_itval = (current_time.tv_sec - g.bounce_freq.tv_sec) +
                            (current_time.tv_usec - g.bounce_freq.tv_usec) / 1000000.0f;
        g.bounce_freq = current_time;
	}
	if (g.pos[0] <= g.w) {
		g.pos[0] = g.w;
		g.dir = -g.dir;

		struct timeval current_time;
        gettimeofday(&current_time, NULL);
        g.bounce_itval = (current_time.tv_sec - g.bounce_freq.tv_sec) +
                            (current_time.tv_usec - g.bounce_freq.tv_usec) / 1000000.0f;
        g.bounce_freq = current_time;
	}

	// Vertical
	g.pos[1] += g.dir_y;

	// T+B
	if (g.pos[1] + g.w >= g.yres) {
        g.pos[1] = g.yres - g.w;
        g.dir_y = -g.dir_y;
    }
    if (g.pos[1] - g.w <= 0) {
        g.pos[1] = g.w;
        g.dir_y = -g.dir_y;
    }
}

void render()
{
	//clear the window
	glClear(GL_COLOR_BUFFER_BIT);
	if (2 * g.w >= g.xres)
        return;

	int red = 220;
	int blue = 220;
	if (g.bounce_itval > 0.0f) {
        float rate = 1.0f / g.bounce_itval;
        float min_rate = 0.1f;
        float max_rate = 10.0f;
        rate = (rate - min_rate) / (max_rate - min_rate);
        rate = std::max(0.0f, std::min(1.0f, rate));
        red = static_cast<int>(rate * 255);
        blue = static_cast<int>((1.0f - rate) * 255);
    }

	//draw the box
	glPushMatrix();
	glTranslatef(g.pos[0], g.pos[1], 0.0f);
	glColor3ub(red, 120, blue);
	glBegin(GL_QUADS);
		glVertex2f(-g.w, -g.w);
		glVertex2f(-g.w,  g.w);
		glVertex2f( g.w,  g.w);
		glVertex2f( g.w, -g.w);
	glEnd();
	glPopMatrix();
}
