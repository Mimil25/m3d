#pragma once

#include <math.h>
#include <fstream>
#include <iostream>

#include "vec.h"

uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
	r = clamp(r, 0u, 255u);
	g = clamp(g, 0u, 255u);
	b = clamp(b, 0u, 255u);
	a = clamp(a, 0u, 255u);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

struct Point{
	vec3f p;
	uint32_t c;
};

struct Shapes{
	std::vector<Point> points=std::vector<Point>();
	void load(char* filename){
		std::ifstream file(filename);
		char T;
		uint32_t color=rgba32(255,255,255,255);
		while(file >> T){
			switch(T){
				case 'C':{
					uint32_t r,g,b,a;
					file >> r >> g >> b >> a;
					color = rgba32(r,g,b,a);
					break;}
				case 'P':{
					float a,b,c;
					file >> a >> b >> c;
					Point p;
					p.p = vec3f{a, b, c};
					p.c = color;
					points.push_back(p);
					break;}
			}
		}
	}
};

struct coord{int x, y;};

class Camera {
	private :
		plan3f p;
		plan3fp pp;
		vec3f up, side;

	public:
		vec3f pos, dir;
		float h, w;
		float sensi=1.0f;

		coord projection(const vec3f& point){
			if( dot( point - pos, dir ) <= 0) return coord{-1,-1};
			vec2f resf = change_repere_vers_2d(pp, intersection(p, (droite3f)droite3fp{pos, point}));
			return coord{(int)resf.x, (int)resf.y};
		}

		void turn(const vec2f& v){
			float t = lenght(dir);
			dir += up*(v.y*sensi);
			dir += side*(v.x*sensi);
			dir = normalize(dir)*t;
		}

		void resize(int _h, int _w){
			h = (float)_h;
			w = (float)_w;
		}

		void setdirto(const vec3f& d){
			dir = normalize(d-pos)*lenght(dir);
		}

		void actualise(void){
			up = vec3f{0.0f, 0.0f, -1.0f};
			if( dir.z*dir.z > 0.0001f ){
				up = -dir;
				up.z = -(up.x*up.x + up.y*up.y) / abs(up.z);
				up = normalize(up);
			}
			side = normalize(vec3f{-dir.y, dir.x, 0.0f});
			pp.a = pos + dir - h/2*up - w/2*side;
			pp.b = pp.a + w*side;
			pp.c = pp.a + h*up;

			p = plan3f{dir.x, dir.y, dir.z, -dot(pp.a, dir)};
		}

		Camera(vec3f _pos, vec3f _dir, int _h, int _w){
			pos = _pos;
			dir = _dir;
			h = (float)_h;
			w = (float)_w;
			actualise();
		}
};
