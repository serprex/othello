#ifdef GLX
#include <GL/glx.h>
#include <sys/unistd.h>
#define EV(y) ev.x##y
#else
#include <SDL.h>
#include <SDL_opengl.h>
#define ButtonPress SDL_MOUSEBUTTONDOWN
#define ButtonRelease SDL_MOUSEBUTTONUP
#define MotionNotify SDL_MOUSEMOTION
#define EV(y) ev.y
#endif
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
const unsigned char wht[]={255,255,255},blk[]={0,0,0},gry[]={160,160,160},sky[]={192,192,240};
#define case(x) break;case x:;
struct game{
	union{char p[8];uint64_t p8;};
	union{char e[8];uint64_t e8;};
	uint8_t t;
}G;
typedef struct game game;
int getz(uint64_t w,int z){
	return !!(w&1LL<<z);
}
void flipz(uint64_t*w,int z){
	*w^=1LL<<z;
}
void flipxy(uint64_t*w,int x,int y){
	flipz(w,x+y*8);
}
int getpxy(game*g,int x,int y){
	return getz(g->p8,x+y*8);
}
int getexy(game*g,int x,int y){
	return getz(g->e8,x+y*8);
}
int gettxy(game*g,int x,int y){
	return !getexy(g,x,y)?0:getpxy(g,x,y)?1:-1;
}
int sane(unsigned x,unsigned y){
	return x<8&&y<8;
}
int trymove_(game*g,int x,int y,int d){
	int xd,yd,v=0;
	switch(d){
	case(0)xd=1;yd=0;
	case(1)xd=1;yd=1;
	case(2)xd=0;yd=1;
	case(3)xd=-1;yd=1;
	case(4)xd=-1;yd=0;
	case(5)xd=-1;yd=-1;
	case(6)xd=0;yd=-1;
	case(7)xd=1;yd=-1;
	}
	for(int i=1;;i++){
		int xx=x+xd*i,yy=y+yd*i;
		if(!sane(xx,yy))return 0;
		if(!getexy(g,xx,yy))return 0;
		if(getpxy(g,xx,yy)==g->t){
			if(i==1)return 0;
			for(int j=1;j<i;j++)
				flipxy(&g->p8,x+xd*j,y+yd*j);
			return 1;
		}
	}
}
void trymove(game*g,int x,int y){
	if(getexy(g,x,y))return;
	int v=0;
	for(int d=0;d<8;d++)
		v|=trymove_(g,x,y,d);
	if(v){
		flipxy(&g->e8,x,y);
		if(g->t)g->p8|=1LL<<x+y*8;
		g->t=!g->t;
	}
}
int main(int argc,char**argv){
	#ifndef GLX
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface*dpy=SDL_SetVideoMode(128,128,0,SDL_OPENGL);
	#else
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_DOUBLEBUFFER,GLX_RGBA,None});
	Window Wdo=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,128,128,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=ButtonPressMask}});
	XMapWindow(dpy,Wdo);
	glXMakeCurrent(dpy,Wdo,glXCreateContext(dpy,vi,0,GL_TRUE));
	#endif
	glOrtho(0,128,128,0,1,-1);
	G.e8=103481868288;
	G.p8=68853694464;
	for(;;){
		#ifdef GLX
		glXSwapBuffers(dpy,Wdo);
		XEvent ev;
		while(XPending(dpy)){
			XNextEvent(dpy,&ev);
		#else
		SDL_GL_SwapBuffers();
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			if(ev.type==SDL_QUIT)return 0;
		#endif
			if(ev.type==ButtonPress){
				int q=EV(button.x)>>4|EV(button.y)&240,x=q&15,y=q>>4;
				trymove(&G,x,y);
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		for(int z=0;z<64;z++){
			int x=z&7,y=z>>3;
			glColor3ubv(!getz(G.e8,z)?gry:getz(G.p8,z)?wht:blk);
			glRecti(x*16,y*16,x*16+16,y*16+16);
		}
		glColor3ubv(sky);
		glBegin(GL_LINES);
		for(int x=16;x<128;x+=16){
			glVertex2i(x,0);
			glVertex2i(x,256);
			glVertex2i(0,x);
			glVertex2i(256,x);
		}
		glEnd();
		#ifdef GLX
		usleep(30000);
		#else
		SDL_Delay(30);
		#endif
	}
}