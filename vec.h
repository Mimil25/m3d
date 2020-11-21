#pragma once

#include <stdio.h>
#include <math.h>

struct vec2f {
    float x, y;

    vec2f& operator *= (float b){ x *= b  ; y *= b  ; return *this; }
    vec2f& operator += (vec2f& b){ x += b.x; y += b.y; return *this; }
    vec2f& operator -= (vec2f& b){ x -= b.x; y -= b.y; return *this; }
};

vec2f operator + (const vec2f& a, const vec2f& b){ return vec2f{a.x + b.x, a.y + b.y}; }
vec2f operator - (const vec2f& a, const vec2f& b){ return vec2f{a.x - b.x, a.y - b.y}; }
vec2f operator + (         vec2f b){ return vec2f{    + b.x,     + b.y}; }
vec2f operator - ( const vec2f& b){ return vec2f{    - b.x,     - b.y}; }
vec2f operator * (float a, const vec2f& b){ return vec2f{a   * b.x, a   * b.y}; }
vec2f operator * (const vec2f& a, float b){ return vec2f{a.x * b  , a.y * b  }; }
float dot        (const vec2f& a, const vec2f& b){ return float(a.x * b.x+ a.y * b.y); }

vec2f polar(float radians){
    return vec2f{cosf(radians), sinf(radians)};
}

float length(vec2f& a){
    return sqrtf(dot(a, a));
}

vec2f normalize(const vec2f& a){
    return 1.0f/sqrtf(dot(a, a)) * a;
}

vec2f normalize(vec2f& a, float eps){
    return 1.0f/(eps + sqrtf(dot(a, a))) * a;
}

vec2f v2f(float x, float y){
    return vec2f{x, y};
}

struct vec3f {
    float x, y, z;

    vec3f& operator *= (float b){ x *= b  ; y *= b  ; z *= b  ; return *this; }
    vec3f& operator += (const vec3f& b){ x += b.x; y += b.y; z += b.z; return *this; }
    vec3f& operator -= (const vec3f& b){ x -= b.x; y -= b.y; z -= b.z; return *this; }
};

vec3f operator + (const vec3f& a, const vec3f& b){ return vec3f{a.x + b.x, a.y + b.y, a.z + b.z}; }
vec3f operator - (const vec3f& a, const vec3f& b){ return vec3f{a.x - b.x, a.y - b.y, a.z - b.z}; }
vec3f operator + (         const vec3f& b){ return b; }
vec3f operator - (         const vec3f& b){ return vec3f{ - b.x, - b.y, -b.z}; }
vec3f operator * (const float a, const vec3f& b){ return vec3f{a   * b.x, a   * b.y, a * b.z}; }
vec3f operator * (const vec3f& a, const float b){ return b * a; }
vec3f operator * (const vec3f& a, const vec3f& b){ return vec3f{a.y*b.z - a.z*b.y,
						  a.z*b.x - a.x*b.z,
						  a.x*b.y - a.y*b.y};}

float dot(const vec3f& a, const vec3f& b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
float lenght(const vec3f& a){ return sqrtf(dot(a, a));}
vec3f normalize(const vec3f& a){return 1.0f/sqrtf(dot(a, a)) * a;}

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
	return vec2f{ dot(p.b-p.a, v-p.a) / lenght(p.b-p.a),
		      dot(p.c-p.a, v-p.a) / lenght(p.c-p.a)};
}

