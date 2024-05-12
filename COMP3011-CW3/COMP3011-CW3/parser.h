#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;

int mtl_parse(char* filename, vector<Material> *mtls)
{
	FILE* file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "Error opening MTL file: %s\n", strerror(errno));
		return 0;
	}

	char line[256];
	Material current_material;

	std::string filename_str(filename);
	size_t lastSlashPos = filename_str.find_last_of("/");
	// Create a string of the directory's path
	std::string directoryPath = (lastSlashPos != std::string::npos) ? filename_str.substr(0, lastSlashPos + 1) : "";

	while (fgets(line, sizeof(line), file)) {
		char token[32];
		sscanf(line, "%s", token);

		if (strcmp(token, "newmtl") == 0) {
			// If current_material is not empty, add it to mtls
			mtls->push_back(current_material);
			
			// Create a new Material object
			current_material = Material();
			sscanf(line + 7, "%s", current_material.mtl_name);
		}
		else if (strcmp(token, "map_Kd") == 0) {
			char texture_filename[256];
			sscanf(line + 7, "%s", texture_filename);
			// Prepend the path
			std::string fullTextureFilename = directoryPath + texture_filename;
			// Copy the name to file name
			strncpy(current_material.fil_name, fullTextureFilename.c_str(), sizeof(current_material.fil_name) - 1);
			// Manually add null terminator. There is probably an easier way of doing this :o
			current_material.fil_name[sizeof(current_material.fil_name) - 1] = '\0';
		}
		else if (strcmp(token, "map_Ks") == 0) {
			char specular_texture_filename[256];
			sscanf(line + 7, "%s", specular_texture_filename);
			// Prepend the path
			std::string fullTextureFilename = directoryPath + specular_texture_filename;
			// Copy the name to specular texture file name
			strncpy(current_material.specular_fil_name, fullTextureFilename.c_str(), sizeof(current_material.specular_fil_name) - 1);
			// Manually add null terminator. There is probably an easier way of doing this :o
			current_material.specular_fil_name[sizeof(current_material.specular_fil_name) - 1] = '\0';
		}
	}

	// Add the last material after the loop ends
	if (current_material.mtl_name[0] != '\0') {
		mtls->push_back(current_material);
	}

	fclose(file);
	return 1;
}

int obj_parse(const char * filename, vector<Object> *objs)
{
	std::vector<Material> materials;
	std::vector<vec3> vecs;
	std::vector<vec2> uvs;

	FILE* file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "Error opening OBJ file: %s\n", strerror(errno));
		return 0;
	}

	std::string filename_str(filename);
	size_t lastSlashPos = filename_str.find_last_of("/");
	// Create string of directory path
	std::string directoryPath = (lastSlashPos != std::string::npos) ? filename_str.substr(0, lastSlashPos + 1) : "";

	char line[256];
	Object obj;
	bool mtlParsed = false;
	// While lines to find
	while (fgets(line, sizeof(line), file)) {
		char token[32];
		sscanf(line, "%s", token);
		if (strcmp(token, "v") == 0) {
			float x, y, z;
			sscanf(line + 2, "%f %f %f", &x, &y, &z);
			vecs.push_back(vec3(x, y, z));
		}
		else if (strcmp(token, "vt") == 0) {
			float u, v;
			sscanf(line + 3, "%f %f", &u, &v);
			// Have to flip textures for t1
			v = -v;
			uvs.push_back(vec2(u, v));
		}
		else if (!mtlParsed && strcmp(token, "mtllib") == 0) {
			char mtl_filename[256];
			sscanf(line + 7, "%s", mtl_filename);
			// Prepend the path
			std::string fullMtlFilename = directoryPath + mtl_filename;
			mtl_parse(&fullMtlFilename[0], &materials);
			// Was running this twice without the bool check. I dont know why
			mtlParsed = true;
		}
		else if (strcmp(token, "usemtl") == 0) {
			char material_name[256];
			sscanf(line + 7, "%s", material_name);
			// For mat in materials
			for (const Material& mat : materials) {
				// If str same
				if (strcmp(mat.mtl_name, material_name) == 0) {
					obj.mtl = mat;
					objs->push_back(obj);
					break;
				}
			}
			// Create a new object
			obj = Object();
		}
		else if (strcmp(token, "f") == 0) {
			int v1=0,v2=0,v3=0,t1=0,t2=0,t3=0,n1=0,n2=0,n3=0;
			sscanf(line + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
			// Make new vertices
			vertex vert0(vecs[v1 - 1], vec3(uvs[t1 - 1].x, uvs[t1 - 1].y, 0));
			vertex vert1(vecs[v2 - 1], vec3(uvs[t2 - 1].x, uvs[t2 - 1].y, 0));
			vertex vert2(vecs[v3 - 1], vec3(uvs[t3 - 1].x, uvs[t3 - 1].y, 0));
			// Store in the objs tris
			objs->back().tris.push_back(triangle(vert0,vert1,vert2));
		}
	}

	fclose(file);
	return 1;
}
