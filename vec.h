#pragma once

#include <stdio.h>
#include <math.h>

struct vec2f {
    float x, y;

    vec2f& operator *= (float b){ x *= b  ; y *= b  ; return *this; }
    vec2f& operator += (vec2f b){ x += b.x; y += b.y; return *this; }
    vec2f& operator -= (vec2f b){ x -= b.x; y -= b.y; return *this; }
};

vec2f operator + (vec2f a, vec2f b){ return vec2f{a.x + b.x, a.y + b.y}; }
vec2f operator - (vec2f a, vec2f b){ return vec2f{a.x - b.x, a.y - b.y}; }
vec2f operator + (         vec2f b){ return vec2f{    + b.x,     + b.y}; }
vec2f operator - (         vec2f b){ return vec2f{    - b.x,     - b.y}; }
vec2f operator * (float a, vec2f b){ return vec2f{a   * b.x, a   * b.y}; }
vec2f operator * (vec2f a, float b){ return vec2f{a.x * b  , a.y * b  }; }
float dot        (vec2f a, vec2f b){ return float(a.x * b.x+ a.y * b.y); }

vec2f polar(float radians){
    return vec2f{cosf(radians), sinf(radians)};
}

float length(vec2f a){
    return sqrtf(dot(a, a));
}

vec2f normalize(vec2f a){
    return 1.0f/sqrtf(dot(a, a)) * a;
}

vec2f normalize(vec2f a, float eps){
    return 1.0f/(eps + sqrtf(dot(a, a))) * a;
}

vec2f v2f(float x, float y){
    return vec2f{x, y};
}

struct vec3f {
    float x, y, z;

    vec3f& operator *= (float b){ x *= b  ; y *= b  ; z *= b  ; return *this; }
    vec3f& operator += (vec3f b){ x += b.x; y += b.y; z += b.z; return *this; }
    vec3f& operator -= (vec3f b){ x -= b.x; y -= b.y; z -= b.z; return *this; }
};

vec3f operator + (vec3f a, vec3f b){ return vec3f{a.x + b.x, a.y + b.y, a.z + b.z}; }
vec3f operator - (vec3f a, vec3f b){ return vec3f{a.x - b.x, a.y - b.y, a.z - b.z}; }
vec3f operator + (         vec3f b){ return b; }
vec3f operator - (         vec3f b){ return vec3f{ - b.x, - b.y, -b.z}; }
vec3f operator * (float a, vec3f b){ return vec3f{a   * b.x, a   * b.y, a * b.z}; }
vec3f operator * (vec3f a, float b){ return b * a; }
vec3f operator * (vec3f a, vec3f b){ return vec3f{a.y*b.z - a.z*b.y,
						  a.z*b.x - a.x*b.z,
						  a.x*b.y - a.y*b.y};}

float dot(vec3f a, vec3f b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
float lenght(vec3f a){ return sqrtf(dot(a, a));}
vec3f normalize(vec3f a){return 1.0f/sqrtf(dot(a, a)) * a;}

template <typename T, typename U>
T lerp(const T &a, const T &b, const U &u){
    return (U(1) - u)*a + u*b;
}

template <typename T>
const T& clamp(const T &x, const T &a, const T &b){
    return
        x < a ? a :
        x > b ? b :
        x;
}

template <typename T, typename U>
T smoothstep(const T &a, const T &b, const U &u){
    T t = clamp((u - a)/(b - a), U(0), U(1));
    return t*t*(U(3) - U(2)*t);
}

struct droite3f {
	float a, a0, b, b0, c, c0;
};

struct droite3fp {
	vec3f a, b;
	operator droite3f(){
		return droite3f{a.x,
			b.x - a.x,
			a.y,
                        b.y - a.y,
			a.z,
                        b.z - a.z};
	}
};

struct plan3f {
	float a, b, c, d;
	float operator () (vec3f v) const{
		return a*v.x + b*v.y + c*v.z + d;
	}
};

struct plan3fp {
	vec3f a, b, c;
	operator plan3f(){
		vec3f o = (b-a)*(c-a);
		return plan3f{o.x, o.y, o.z, -dot(a, o)};
	}
};

vec3f intersection(const plan3f& p, const droite3f& d){
	float t = -( (p.a*d.a + p.b*d.b + p.c*d.c + p.d)
		    / (p.a*d.a0 + p.b*d.b0 + p.c*d.c0));
	return vec3f{ d.a + d.a0*t,
		      d.b + d.b0*t,
		      d.c + d.c0*t};
}

vec2f change_repere_vers_2d(const plan3fp& p, const vec3f& v){
	//float a=lenght(p.b-p.a), b=lenght(p.c-p.a);
	return vec2f{ dot(p.b-p.a, v-p.a) / lenght(p.b-p.a),
		      dot(p.c-p.a, v-p.a) / lenght(p.c-p.a)};
}

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

	coord projection(vec3f point){
		if( dot( point - (pos+dir), dir ) <= 0) return coord{-1,-1};
		vec2f resf = change_repere_vers_2d(pp, intersection(p, (droite3f)droite3fp{pos, point}));
		return coord{(int)resf.x, (int)resf.y};
	}

	void turn(vec2f v){
		float t = lenght(dir);
		dir += up*(v.y*sensi);
		dir += side*(v.x*sensi);
		dir = normalize(dir)*t;
	}

	void resize(int _h, int _w){
		h = (float)_h;
                w = (float)_w;
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

		//vec3f test = normalize(dir*side) - up;
		//printf(" %f %f %f \n", test.x, test.y, test.z);
	}

	Camera(vec3f _pos, vec3f _dir, int _h, int _w){
		pos = _pos;
		dir = _dir;
		h = (float)_h;
		w = (float)_w;
		actualise();
	}
};






















