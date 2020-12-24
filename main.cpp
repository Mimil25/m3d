#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <SDL2/SDL.h>
#include "vec.h"
#include "camera.h"


#define FPS 20

//#define CVEW
//#define TRIANGLES
#define POINTS


class Rendering
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	const uint32_t pixelFormat=SDL_PIXELFORMAT_RGBA8888;

	int wWidth=800;
	int wHeight=600;

	vec2f mouse={0,0};
	bool b1=false;

	vec3f d={0,0,0};
	const float v=50.0f;

	Camera camera=Camera(vec3f{0,10000,10000}, vec3f{1000,0,0}, wHeight, wWidth);
	Shapes shapes;
	public:

	Rendering()
	{
		if(SDL_Init(SDL_INIT_VIDEO)!=0)
		{
			exit(EXIT_FAILURE);
		}
		window = SDL_CreateWindow("simulation",
				SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				wWidth, wHeight,
				SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, wWidth, wHeight);
		
		camera.setdirto(vec3f{0,0,0});
	}

	void load(char* filename)
	{
		shapes.load(filename);
	}

	void handleEvent(const SDL_Event& event)
	{
		switch(event.type){
			case SDL_MOUSEBUTTONDOWN:
				b1 = true;
				break;
			case SDL_MOUSEBUTTONUP:
				b1 = false;
				break;
#ifndef CVEW
			case SDL_MOUSEMOTION:
				if(b1){
					camera.turn(vec2f{mouse.x - event.motion.x,
							mouse.y - event.motion.y});
				}
				mouse = vec2f{(float)event.motion.x, (float)event.motion.y};
				break;
#endif
			case SDL_KEYDOWN:
				goto handleKey;
			case SDL_KEYUP:
				goto handleKey;
			case SDL_MOUSEWHEEL:
				break;
			case SDL_WINDOWEVENT:
				switch(event.window.event){
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						wWidth = event.window.data1;
						wHeight = event.window.data2;
						camera.resize(wHeight, wWidth);
						SDL_DestroyTexture(texture);
						texture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, wWidth, wHeight);
						break;
				}
				break;


		}

		return;

handleKey:
		vec2f tmp;
		if( event.type == SDL_KEYDOWN )
		{
			switch( event.key.keysym.sym )
			{
				case SDLK_SPACE: d.z = v ; break;
				case SDLK_LCTRL: d.z = -v; break;
				case SDLK_z :
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x = tmp.x; d.y = tmp.y; break;
				case SDLK_s : 
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*(-v);
						 d.x = tmp.x; d.y = tmp.y; break;
				case SDLK_q :
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x = tmp.y; d.y = -tmp.x; break;
				case SDLK_d : 
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x = -tmp.y; d.y = tmp.x; break;
				default: break;
			}
		}
		else if( event.type == SDL_KEYUP )
		{
			switch( event.key.keysym.sym )
			{
				case SDLK_SPACE: d.z -= v ; break;
				case SDLK_LCTRL: d.z -= -v; break;
				case SDLK_z :
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x -= tmp.x; d.y -= tmp.y; break;
				case SDLK_s : 
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*(-v);
						 d.x -= tmp.x; d.y -= tmp.y; break;
				case SDLK_q :
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x -= tmp.y; d.y -= -tmp.x; break;
				case SDLK_d : 
						 tmp=normalize(vec2f{camera.dir.x, camera.dir.y})*v;
						 d.x -= -tmp.y; d.y -= tmp.x; break;
						 /*case SDLK_z : d.x = 0; d.y = 0; break;
						   case SDLK_s : d.x = 0; d.y = 0; break;
						   case SDLK_q : d.x = 0; d.y = 0; break;
						   case SDLK_d : d.x = 0; d.y = 0; break;*/
				default: break;
			}
		}
	}

	static uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
		r = clamp(r, 0u, 255u);
		g = clamp(g, 0u, 255u);
		b = clamp(b, 0u, 255u);
		a = clamp(a, 0u, 255u);
		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	void print(void)
	{
		camera.pos += d;
#ifdef CVEW
		camera.setdirto(vec3f{0,0,0});
#endif
		camera.actualise();


		int pitch;
		uint8_t *pixels;
		SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

		camera.render((uint32_t*)pixels, shapes, 0x27);

		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_RenderClear(renderer);
	}

	void destroy(void)
	{
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();
	}
};

int main(int argc, char* argv[])
{
	Rendering renderer;

	if(argc >= 2){
		renderer.load(argv[1]);
	}

	SDL_Event event;
	bool running=true;
	int t, time=SDL_GetTicks();
	srand(0);
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT) running=false;
			else renderer.handleEvent(event);
		}

		renderer.print();

		t = time - SDL_GetTicks() + 1000/FPS;
		if(t > 0) SDL_Delay(t);
		else SDL_Log("On a sauté la pause");
		time = SDL_GetTicks();
	}
	renderer.destroy();
	return 0;
}
