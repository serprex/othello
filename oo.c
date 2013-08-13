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
#define MAXMOVES 32
#include <stdint.h>
#include <string.h>
const unsigned char wht[]={255,255,255},blk[]={0,0,0},gry[]={160,160,160},blu[]={180,180,128};
typedef struct game{
	uint64_t p,e;
	uint8_t t;
}game;
game G;
uint8_t Gm[MAXMOVES];
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
	return getz(g->p,x+y*8);
}
int getexy(game*g,int x,int y){
	return getz(g->e,x+y*8);
}
int sane(unsigned x,unsigned y){
	return x<8&&y<8;
}
int trymove_(game*g,int x,int y,int xd,int yd){
	int xx=x,yy=y;
	for(;;){
		xx+=xd;
		yy+=yd;
		if(!sane(xx,yy)||!getexy(g,xx,yy))return 0;
		if(getpxy(g,xx,yy)==g->t){
			if(x+xd==xx&&y+yd==yy)return 0;
			for(;;){
				xx-=xd;
				yy-=yd;
				if(xx==x&&yy==y)return 1;
				flipxy(&g->p,xx,yy);
			}
		}
	}
}
int trymove(game*g,int x,int y){
	if(getexy(g,x,y))return 0;
	int v=0;
	for(int xd=-1;xd<2;xd++)
		for(int yd=-1;yd<2;yd+=1+!xd)
			v|=trymove_(g,x,y,xd,yd);
	if(v){
		flipxy(&g->e,x,y);
		if(g->t)g->p|=1LL<<x+y*8;
		g->t=!g->t;
	}
	return v;
}
int canmove_(game*g,int x,int y,int xd,int yd){
	int xx=x,yy=y;
	for(;;){
		xx+=xd;
		yy+=yd;
		if(!sane(xx,yy)||!getexy(g,xx,yy))return 0;
		if(getpxy(g,xx,yy)==g->t)return x+xd!=xx||y+yd!=yy;
	}
}
int canmove(game*g,int x,int y){
	if(getexy(g,x,y))return 0;
	int v=0;
	for(int xd=-1;xd<2;xd++)
		for(int yd=-1;yd<2;yd+=1+!xd)
			v|=canmove_(g,x,y,xd,yd);
	return v;
}
void movelist(game*g,uint8_t*m){
	for(int x=0;x<8;x++)
		for(int y=0;y<8;y++)
			if(canmove(g,x,y))*m++=x|y<<3;
	*m=255;
}
int inlistz(uint8_t*m,int z){
	for(int i=0;m[i]!=255;i++)
		if(m[i]==z)return i;
	return -1;
}
int inlist(uint8_t*m,int x,int y){
	return inlistz(m,x+y*8);
}
int main(int argc,char**argv){
	#ifndef GLX
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window*dpy=SDL_CreateWindow(0,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,256,256,SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(dpy);
	#else
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_DOUBLEBUFFER,GLX_RGBA,None});
	Window Wdo=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,256,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=ButtonPressMask}});
	XMapWindow(dpy,Wdo);
	glXMakeCurrent(dpy,Wdo,glXCreateContext(dpy,vi,0,GL_TRUE));
	#endif
	glOrtho(0,256,256,0,1,0);
	G.e=103481868288;
	G.p=68853694464;
	movelist(&G,Gm);
	for(;;){
		#ifdef GLX
		glXSwapBuffers(dpy,Wdo);
		XEvent ev;
		while(XPending(dpy)){
			XNextEvent(dpy,&ev);
		#else
		SDL_GL_SwapWindow(dpy);
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			if(ev.type==SDL_QUIT)return 0;
		#endif
			if(ev.type==ButtonPress){
				trymove(&G,EV(button.x)>>5,EV(button.y)>>5);
				movelist(&G,Gm);
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		for(int z=0;z<64;z++){
			int x=z&7,y=z>>3;
			glColor3ubv(!getz(G.e,z)?(inlistz(Gm,z)!=-1?blu:gry):getz(G.p,z)?wht:blk);
			glRecti(x*32,y*32,x*32+32,y*32+32);
		}
		glColor3ubv(G.t?wht:blk);
		glBegin(GL_LINES);
		for(int x=32;x<256;x+=32){
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