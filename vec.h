#pragma once

#include <stdio.h>
#include <math.h>

struct vec2f {
    float x, y;

    inline vec2f& operator *= (float b){ x *= b  ; y *= b  ; return *this; }
    inline vec2f& operator += (vec2f& b){ x += b.x; y += b.y; return *this; }
    inline vec2f& operator -= (vec2f& b){ x -= b.x; y -= b.y; return *this; }
};
inline vec2f operator + (const vec2f& a, const vec2f& b){ return vec2f{a.x + b.x, a.y + b.y}; }
inline vec2f operator - (const vec2f& a, const vec2f& b){ return vec2f{a.x - b.x, a.y - b.y}; }
inline vec2f operator + (         vec2f b){ return vec2f{    + b.x,     + b.y}; }
inline vec2f operator - ( const vec2f& b){ return vec2f{    - b.x,     - b.y}; }
inline vec2f operator * (float a, const vec2f& b){ return vec2f{a   * b.x, a   * b.y}; }
inline vec2f operator * (const vec2f& a, float b){ return vec2f{a.x * b  , a.y * b  }; }
inline float dot        (const vec2f& a, const vec2f& b){ return float(a.x * b.x+ a.y * b.y); }

vec2f polar(float radians){
    return vec2f{cosf(radians), sinf(radians)};
}

inline float length(const vec2f& a){
    return sqrtf(dot(a, a));
}

inline vec2f normalize(const vec2f& a){
    return 1.0f/sqrtf(dot(a, a)) * a;
}

inline vec2f normalize(vec2f& a, float eps){
    return 1.0f/(eps + sqrtf(dot(a, a))) * a;
}

inline vec2f v2f(float x, float y){
    return vec2f{x, y};
}

struct vec3f {
    float x, y, z;

    inline vec3f& operator *= (float b){ x *= b  ; y *= b  ; z *= b  ; return *this; }
    inline vec3f& operator += (const vec3f& b){ x += b.x; y += b.y; z += b.z; return *this; }
    inline vec3f& operator -= (const vec3f& b){ x -= b.x; y -= b.y; z -= b.z; return *this; }
};

inline vec3f operator + (const vec3f& a, const vec3f& b){ return vec3f{a.x + b.x, a.y + b.y, a.z + b.z}; }
inline vec3f operator - (const vec3f& a, const vec3f& b){ return vec3f{a.x - b.x, a.y - b.y, a.z - b.z}; }
inline vec3f operator + (         const vec3f& b){ return b; }
inline vec3f operator - (         const vec3f& b){ return vec3f{ - b.x, - b.y, -b.z}; }
inline vec3f operator * (const float a, const vec3f& b){ return vec3f{a   * b.x, a   * b.y, a * b.z}; }
inline vec3f operator * (const vec3f& a, const float b){ return b * a; }
inline vec3f operator * (const vec3f& a, const vec3f& b){ return vec3f{a.y*b.z - a.z*b.y,
								       a.z*b.x - a.x*b.z,
				   				       a.x*b.y - a.y*b.y};}

inline float dot(const vec3f& a, const vec3f& b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float length(const vec3f& a){ return sqrtf(dot(a, a));}
inline vec3f normalize(const vec3f& a){return 1.0f/sqrtf(dot(a, a)) * a;}

template <typename T>
struct Grid {
    T *values;
    int nx, ny;

    Grid(int nx, int ny): nx(nx), ny(ny){
        values = new T[nx*ny];
    }

    //Grid(const Grid&) = delete;
    //Grid& operator = (const Grid&) = delete;

    ~Grid(){
        delete[] values;
    }

    void realoc(int _nx, int _ny){
	    nx = _nx;
	    ny = _ny;
	    delete[] values;
	    values = new T[nx*ny];
    }

    inline const T* data() const {
        return values;
    }

    inline int idx(const int x, const int y) const {
        //x = clamp(x, 0, nx - 1);
        //y = clamp(y, 0, ny - 1);

        // wrap around
        //x = (x + nx) % nx;
        //y = (y + ny) % ny;

        return x + y*nx;
    }

    inline T& operator () (const int x, const int y){
        return values[idx(x, y)];
    }

    inline const T& operator () (const int x, const int y) const {
        return values[idx(x, y)];
    }
};

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

struct droite2f{ // a*x + b*y = 0
	float a, b;
};

struct droite2fp{
	vec2f a, b;
//	inline operator droite2f() const{}
};

struct droite3f {
	float a, a0, b, b0, c, c0;
};

struct droite3fp {
	vec3f a, b;
	inline operator droite3f() const{
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
	inline float operator () (vec3f v) const{
		return a*v.x + b*v.y + c*v.z + d;
	}
};

struct plan3fp {
	vec3f a, b, c;
	inline operator plan3f() const{
		vec3f o = (b-a)*(c-a);
		return plan3f{o.x, o.y, o.z, -dot(a, o)};
	}
};

inline vec3f intersection(const plan3f& p, const droite3f& d){
	float t = -( (p.a*d.a + p.b*d.b + p.c*d.c + p.d)
		    / (p.a*d.a0 + p.b*d.b0 + p.c*d.c0));
	return vec3f{ d.a + d.a0*t,
		      d.b + d.b0*t,
		      d.c + d.c0*t};
}

inline vec2f change_repere_vers_2d(const plan3fp& p, const vec3f& v){
	return vec2f{ dot(p.b-p.a, v-p.a) / length(p.b-p.a),
		      dot(p.c-p.a, v-p.a) / length(p.c-p.a)};
}

