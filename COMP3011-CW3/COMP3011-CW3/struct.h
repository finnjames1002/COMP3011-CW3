#include <vector>

class vec3
{
public:
	float x, y, z;

	vec3() {}
	vec3(float l, float m, float r)
	{
		x = l;
		y = m;
		z = r;
	}
	~vec3() {}
};

class vec2
{
public:
	float x, y;

	vec2() {}
	vec2(float l, float r)
	{
		x = l;
		y = r;
	}
	~vec2() {}
};



struct vertex
{
public:
	vec3 vc;
	vec3 tc;

	vertex() {}
	vertex(vec3 vc_in, vec3 tc_in)
	{
		vc = vc_in;
		tc = tc_in;
	}
	~vertex() {}
};

struct Vertex {
	float x, y, z;
};

struct triangle
{
public:
	vertex verts[3];

	triangle() {}
	triangle(vertex v0, vertex v1, vertex v2)
	{
		verts[0] = v0;
		verts[1] = v1;
		verts[2] = v2;
	}
	~triangle() {}
};

class Material
{
public:
	char mtl_name[256];
	char fil_name[256];
	char specular_fil_name[256];
	char emit_fil_name[256];

	Material() {
		strcpy(mtl_name, "none");
		strcpy(fil_name, "none");
		strcpy(specular_fil_name, "none");
		strcpy(emit_fil_name, "none");
	}
	Material(char* n, char* f, char* sf, char* ef)
	{
		strcpy(mtl_name, n);
		strcpy(fil_name, f);
		strcpy(specular_fil_name, sf);
		strcpy(emit_fil_name, ef);
	}
	~Material()
	{
	}
};

class Object
{
public:
	unsigned int VAO;
	unsigned int VBO;
	vector<triangle> tris;
	Material mtl;
	GLuint texture;
	GLuint specularTexture;
	GLuint emitTexture;


	Object() {}
	Object(Material m)
	{
		strcpy(mtl.fil_name, m.fil_name);
		strcpy(mtl.mtl_name, m.mtl_name);
		strcpy(mtl.specular_fil_name, m.specular_fil_name);
		strcpy(mtl.emit_fil_name, m.emit_fil_name);
	}
	~Object()
	{
	}
};