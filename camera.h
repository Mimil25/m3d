#pragma once

#include <math.h>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "vec.h"

#define MAX_DISTANCE 100000.0f

uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
	r = clamp(r, 0u, 255u);
	g = clamp(g, 0u, 255u);
	b = clamp(b, 0u, 255u);
	a = clamp(a, 0u, 255u);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

inline uint64_t melangeCouleur(const uint64_t a, const int64_t b){
	const uint64_t masque=0x80808080;
	return ((a<<1)&masque) + ((b<<1)&masque);
}

struct Point{
	vec3f p;
	vec2f p2;
	bool bord;
	uint32_t c;
};

struct Triangle{
	Point *a, *b, *c;
	uint32_t color;
};

Triangle triangle(Point* a, Point* b, Point* c, const uint32_t color){
	Triangle t;

	t.a = a;
	t.b = b;
	t.c = c;

	t.color = color;

	return t;
}

struct Shapes{
	std::vector<Point> points=std::vector<Point>();
	std::vector<Triangle> triangles=std::vector<Triangle>();
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
				case 'T':{
						 int a, b, c;
						 file >> a >> b >> c;
						 Triangle t = triangle(&points[a], &points[b], &points[c], color);
						 triangles.push_back(t);
						 break;}
			}
		}
	}
};

struct coord{int x, y;};
inline coord Coord(const vec2f& v){return coord{(int)v.x,(int)v.y};}

class Camera {
	private :
		plan3f p;
		plan3fp pp;
		vec3f up, side;

		Grid<float> matriceD=Grid<float>(0,0);

	public:
		vec3f pos, dir;
		float w, h;
		int W, H;
		float sensi=1.0f;

	private:

		inline vec2f projection0(const vec3f& point) const{
			const float d=dot( point - pos, dir );
			if( d <= 0){
				return change_repere_vers_2d(pp, intersection(p, (droite3f)droite3fp{pos, point}))*d;
			}
			return change_repere_vers_2d(pp, intersection(p, (droite3f)droite3fp{pos, point}));
		}

		inline vec2f projection1(const vec3f& point) const{
			vec2f v = projection0(point);
			if(v.x < 0)       v = vec2f{0.0f, (v.y-h/2.0f)*( (w/2.0f) / (w/2.0f-v.x)) + h/2.0f};
			else if(v.x >= w) v = vec2f{w-1.0f,(v.y-h/2.0f)*((w/2.0f) / (v.x-w/2.0f)) + h/2.0f};
			if(v.y < 0)       v = vec2f{(v.x-w/2.0f)*( (h/2.0f) / (h/2.0f-v.y)) + w/2.0f, 0.0f};
			else if(v.y >= h) v = vec2f{(v.x-w/2.0f)*((h/2.0f) / (v.y-h/2.0f)) + w/2.0f,h-1.0f};
			return v;
		}

		inline vec2f projection1(const vec3f& point, bool& b) const{
			vec2f v = projection0(point);
			if(v.x < 0)      {v = vec2f{0.0f, (v.y-h/2.0f)*( (w/2.0f) / (w/2.0f-v.x)) + h/2.0f};b=true;}
			else if(v.x >= w){v = vec2f{w-1.0f,(v.y-h/2.0f)*((w/2.0f) / (v.x-w/2.0f)) + h/2.0f};b=true;}
			if(v.y < 0)      {v = vec2f{(v.x-w/2.0f)*( (h/2.0f) / (h/2.0f-v.y)) + w/2.0f, 0.0f};b=true;}
			else if(v.y >= h){v = vec2f{(v.x-w/2.0f)*((h/2.0f) / (v.y-h/2.0f)) + w/2.0f,h-1.0f};b=true;}
			return v;
		}

		inline vec2f projection2(const vec3f& point, bool* b) const{
			const vec2f v = projection0(point);
			*b = (v.x < 0 or v.x >= w or v.y < 0 or v.y >= h);
			return v;
		}

	public:

		inline coord projection(const vec3f& point) const{
			return Coord(projection1(point));
		}

	private:

		void renderPoint(uint32_t* pixels, const Point& p) const{
			if(p.bord)return;
			const coord c = Coord(p.p2);
			pixels[W*c.y + c.x] = p.c;
		}

		inline void makeLineH(uint32_t* pixels, const int y, int x1, int x2, const uint32_t color){
			if(y < 0 or x2 < 0 or x1 >= W or y >= H)return;
			if(x1 < 0) x1=0;
			if(x2 >= W)x2=W-1;
			const uint32_t* end=pixels+W*y+x2;
			for(uint32_t* p=pixels+W*y+x1; p < end; ++p){
				*p = color;
			}
		}

		inline void setPixel(uint32_t* pixels, const int x, const int y, const uint32_t color){
			if(x < 0 or x >= W or y < 0 or y >= H)return;
			pixels[W*y + x] = color;
		}

		inline void ligne(uint32_t* pixels, int x1, int y1, int x2, int y2, const uint32_t color)
		{
			int d, dx, dy, aincr, bincr, xincr, yincr, x, y;

			if (abs(x2 - x1) < abs(y2 - y1)) {
				/* parcours par l'axe vertical */

				if (y1 > y2) {
					std::swap(x1, x2);
					std::swap(y1, y2);
				}

				xincr = x2 > x1 ? 1 : -1;
				dy = y2 - y1;
				dx = abs(x2 - x1);
				d = 2 * dx - dy;
				aincr = 2 * (dx - dy);
				bincr = 2 * dx;
				x = x1;
				y = y1;

				setPixel(pixels, x, y, color);

				for (y = y1+1; y <= y2; ++y) {
					if (d >= 0) {
						x += xincr;
						d += aincr;
					} else
						d += bincr;

					setPixel(pixels, x, y, color);
				}

			} else {
				/* parcours par l'axe horizontal */

				if (x1 > x2) {
					std::swap(x1, x2);
					std::swap(y1, y2);
				}

				yincr = y2 > y1 ? 1 : -1;
				dx = x2 - x1;
				dy = abs(y2 - y1);
				d = 2 * dy - dx;
				aincr = 2 * (dy - dx);
				bincr = 2 * dy;
				x = x1;
				y = y1;

				setPixel(pixels, x, y, color);

				for (x = x1+1; x <= x2; ++x) {
					if (d >= 0) {
						y += yincr;
						d += aincr;
					} else
						d += bincr;

					setPixel(pixels, x, y, color);
				}
			}
		}

		void renderTriangle(uint32_t* pixels, const Triangle& t){
			ligne(pixels, t.a->p2.x, t.a->p2.y, t.b->p2.x, t.b->p2.y, t.color);
			ligne(pixels, t.a->p2.x, t.a->p2.y, t.c->p2.x, t.c->p2.y, t.color);
			ligne(pixels, t.b->p2.x, t.b->p2.y, t.c->p2.x, t.c->p2.y, t.color);
		}

		void renderTriangleF(uint32_t* pixels, const Triangle& t){
			vec2f a,b,c;
			a = t.a->p2;
			b = t.b->p2;
			c = t.c->p2;
			if(a.y > b.y) std::swap(a, b);
			if(a.y > c.y) std::swap(a, c);
			if(b.y > c.y) std::swap(b, c);

			const float ab=(b.x-a.x)/(b.y-a.y),
			      ac=(c.x-a.x)/(c.y-a.y),
			      bc=(a.x-c.x)/(c.y-b.y);

			int y;
			float x1=a.x,x2=a.x;

			if(ab < ac){

				for(y = (int)a.y; y < (int)b.y; ++y){
					makeLineH(pixels, y, (int)x1, (int)x2, t.color);
					x1 += ab;
					x2 += ac;
				}
				for(;y < (int)c.y; ++y){
					makeLineH(pixels, y, (int)x1, (int)x2, t.color);
					x1 += bc;
					x2 += ac;
				}

			} else {

				for(y = (int)a.y; y < (int)b.y; ++y){
					makeLineH(pixels, y, (int)x2, (int)x1, t.color);
					x1 += ab;
					x2 += ac;
				}
				for(;y < (int)c.y; ++y){
					makeLineH(pixels, y, (int)x2, (int)x1, t.color);
					x1 += bc;
					x2 += ac;
				}
			}
		}

	public:

		void render(uint32_t* pixels, Shapes& shapes, const uint8_t flags){
			if(flags & 1){
				const uint64_t* end = (uint64_t*)pixels + W/2 *H;
				for(uint64_t* p=(uint64_t*)pixels; p<end; *(p++)=0);
			}
			if(flags & 2){
				const float* end = matriceD.values + W*H;
				for(float* p=matriceD.values; p < end; *(p++)=MAX_DISTANCE);
			}
			if(flags & 4){
				const auto end = shapes.points.end();
				for(auto p=shapes.points.begin(); p < end; ++p)
					p->p2 = projection2(p->p, &(p->bord));
			}
			if(flags & 8){
				const auto end = shapes.points.end();
				for(auto p=shapes.points.begin(); p < end; ++p)
					renderPoint(pixels, *p);
			}
			if(flags & 16){
				const auto end = shapes.triangles.end();
				for(auto p=shapes.triangles.begin(); p < end; ++p)
					renderTriangleF(pixels, *p);
			}
			if(flags & 32){
				const auto end = shapes.triangles.end();
				for(auto p=shapes.triangles.begin(); p < end; ++p)
					renderTriangle(pixels, *p);
			}
			if(flags & 64){}
			if(flags & 128){}
		}

		void turn(const vec2f& v){
			const float t = length(dir);
			dir += up*(v.y*sensi);
			dir += side*(v.x*sensi);
			dir = normalize(dir)*t;
		}

		void resize(int _h, int _w){
			H = _h;
			W = _w;
			h = (float)_h;
			w = (float)_w;
			matriceD.realoc(W, H);
		}

		void setdirto(const vec3f& d){
			dir = normalize(d-pos)*length(dir);
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
			H = _h;
			W = _w;
			h = (float)_h;
			w = (float)_w;
			matriceD.realoc(W, H);
			actualise();
		}
};
