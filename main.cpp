#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <SDL2/SDL.h>
#include "vec.h"


#define FPS 30

template <typename T>
struct Grid {
    T *values;
    int nx, ny;

    Grid(int nx, int ny): nx(nx), ny(ny){
        values = new T[nx*ny];
    }

    Grid(const Grid&) = delete;
    Grid& operator = (const Grid&) = delete;

    ~Grid(){
        delete[] values;
    }

    void swap(Grid &other){
        std::swap(values, other.values);
        std::swap(nx, other.nx);
        std::swap(ny, other.ny);
    }

    const T* data() const {
        return values;
    }

    int idx(int x, int y) const {
        //x = clamp(x, 0, nx - 1);
        //y = clamp(y, 0, ny - 1);

        // wrap around
        x = (x + nx) % nx;
        y = (y + ny) % ny;

        return x + y*nx;
    }

    T& operator () (int x, int y){
        return values[idx(x, y)];
    }

    const T& operator () (int x, int y) const {
        return values[idx(x, y)];
    }
};


class Rendering
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	
	int wWidth=800;
	int wHeight=600;

	vec2f mouse={0,0};
	bool b1=false;

	vec3f d={0,0,0};
	const float v=50.0f;

	Camera camera=Camera(vec3f{0,0,1000}, vec3f{1000,0,0}, wHeight, wWidth);
	std::vector<vec3f> points;
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
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

		points = std::vector<vec3f>(0);
	}

	void load(char* filename)
	{
		std::ifstream file(filename);
		char T;
		while(file >> T)
		{
			switch(T)
			{
			case 'P':
				float a,b,c;
				file >> a >> b >> c;
				points.push_back(vec3f{a, b, c});
			}
		}
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
		case SDL_MOUSEMOTION:
			if(b1){
				camera.turn(vec2f{mouse.x - event.motion.x,
						  mouse.y - event.motion.y});
			}
			mouse = vec2f{(float)event.motion.x, (float)event.motion.y};
			break;
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

	void print(void)
	{
		camera.pos += d;
		camera.actualise();
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
		for(auto p=points.begin(); p != points.end(); ++p){
                	coord c = camera.projection(*p);
                	SDL_RenderDrawPoint(renderer, c.x, c.y);
        	}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderPresent(renderer);
		SDL_RenderClear(renderer);
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
	SDL_Quit();
	return 0;
}
