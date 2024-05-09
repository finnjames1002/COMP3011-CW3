#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <vector>

using namespace std;

#include "error.h"
#include "obj.h"
#include "shader.h"
#include "texture.h"
#include "test.h"
#include "shadow.h"
#include "camera.h"

#define WIDTH 1200
#define HEIGHT 800
#define SH_MAP_WIDTH 2048
#define SH_MAP_HEIGHT 2048
#define PI 3.14159265358979323846

Camera camera = Camera();

float radius = 15.0f; // Camera's distance from the center of rotation

// Define the vertices for sun
int numLatitudeLines = 100;
int numLongitudeLines = 100;

// Define the position of the light source (the sun)
//glm::vec3 lightPos = glm::vec3(0.f, 10.f, -80.f);
glm::vec3 lightPos = glm::vec3(0.f, 10.f, -50.f);

void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}



void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

struct Vertex {
    float x, y, z;
};

float* generateSphereVert(int numLatitudeLines, int numLongitudeLines) {
    std::vector<Vertex> vertices;

    for (int i = 0; i < numLatitudeLines; ++i) {
        for (int j = 0; j <= numLongitudeLines; ++j) {
            for (int k = 0; k <= 1; ++k) {
                float theta = static_cast<float>((i + k) * PI / numLatitudeLines);
                float phi = static_cast<float>(j * 2 * PI / numLongitudeLines);

                Vertex vertex;
                vertex.x = std::sin(theta) * std::cos(phi);
                vertex.y = std::sin(theta) * std::sin(phi);
                vertex.z = std::cos(theta);

                vertices.push_back(vertex);
            }
        }
    }

    float* flattenedVertices = new float[(numLatitudeLines + 1) * (numLongitudeLines + 1) * 2 * 6];

    for (int i = 0; i < numLatitudeLines * (numLongitudeLines + 1) * 2; ++i) {
        flattenedVertices[i * 6] = vertices[i].x;
        flattenedVertices[i * 6 + 1] = vertices[i].y;
        flattenedVertices[i * 6 + 2] = vertices[i].z;
        flattenedVertices[i * 6 + 3] = 1.0f; // Red
        flattenedVertices[i * 6 + 4] = 0.87f; // Green
        flattenedVertices[i * 6 + 5] = 0.13f; // Blue
    }


    return flattenedVertices;
}

void setupSphere(unsigned int* sphereVAO, unsigned int* sphereVBO, int numVertices, float* vert) {
    glBindVertexArray(0); // Unbind VAO

    glGenVertexArrays(1, sphereVAO);
    glGenBuffers(1, sphereVBO);

    // Bind and set vertex buffer(s) and attribute pointer(s) for the sphere
    glBindVertexArray(*sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, *sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

vector<Object> setupTree(vector<Object> objs) {
    //Setup for objecs 
    for (int i = 0; i < objs.size(); i++)
    {
        // Enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        objs[i].texture = CreateTexture(objs[i].mtl.fil_name);

        glGenVertexArrays(1, &objs[i].VAO);
        glGenBuffers(1, &objs[i].VBO);

        glBindVertexArray(objs[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, objs[i].VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (objs[i].tris.size() * 18), objs[i].tris.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float))); // Add this line
        glEnableVertexAttribArray(2);
    }
	return objs;
}

vector<Object> setupFloor(vector<Object> objs2) {
    for (int i = 0; i < objs2.size(); i++)
    {
        objs2[i].texture = CreateTexture(objs2[i].mtl.fil_name);

        glGenVertexArrays(1, &objs2[i].VAO);
        glGenBuffers(1, &objs2[i].VBO);

        glBindVertexArray(objs2[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, objs2[i].VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (objs2[i].tris.size() * 18), objs2[i].tris.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    return objs2;
}

void drawSphere(int sphereProgram, int sphereVAO, int numVertices, glm::mat4 view, glm::mat4 projection, glm::mat4 modelSun) {
    // Draw the sphere first
    glUseProgram(sphereProgram); // Use the sphere shader program
    glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // Use the same model matrix as the textured objects
    glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelSun));
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
}

void drawObject(vector<Object> objs, glm::mat4 model, int shaderProgram, int texScale) {
    for (int i = 0; i < objs.size(); i++)
    {
		glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, objs[i].texture);
        glBindVertexArray(objs[i].VAO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), static_cast<GLfloat>(texScale));
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(objs[i].tris.size() * 3));
    }
}

void generateDepthMap(unsigned int shadowShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix, vector<Object> objs, vector<Object> objs2)
{
    glViewport(0, 0, SH_MAP_WIDTH, SH_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
    drawObject(objs2, glm::mat4(1.f), shadowShaderProgram, 50);
	drawObject(objs, glm::mat4(1.f), shadowShaderProgram, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char** argv)
{
    float* vert = generateSphereVert(numLatitudeLines, numLongitudeLines);
    int numVertices = numLatitudeLines * numLongitudeLines * 6;
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8); // Enable 8x MSAA
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Assessment 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, SizeCallback);

    gl3wInit();
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugMessageCallback, 0);
    
    // Shadow setup
	ShadowStruct shadow = setup_shadowmap(SH_MAP_WIDTH, SH_MAP_HEIGHT);

    unsigned int shaderProgram = CompileShader("textured.vert", "textured.frag");
	unsigned int sphereProgram = CompileShader("triangle.vert", "triangle.frag");
	unsigned int shadowProgram = CompileShader("shadow.vert", "shadow.frag");

    //SECTION A - EDIT THIS CODE TO TEST
    //Test0 T0;
    Test1 T1;
    Test2 T2;
    Test* tester = &T2;
    vector<Object> objs = tester->ParseObj();

    
	objs = setupTree(objs);

	vector<Object> objs2 = T1.ParseObj();

	objs2 = setupFloor(objs2);
    

    // Define the VAO and VBO for the sphere
    unsigned int sphereVAO, sphereVBO;
	setupSphere(&sphereVAO, &sphereVBO, numVertices, vert);
    
    // Unbind the VAO
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);


    while (!glfwWindowShouldClose(window))
    {
        camera.processKeyboard(window);
		camera.setRotationSpeed((float)glfwGetTime() / 2);

        glClearColor(0.51f, 0.79f, 1.f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glm::mat4 model = glm::mat4(1.f);
        tester->Model(&model);

		glm::mat4 modelFloor = glm::mat4(1.f);
		tester->ModelFloor(&modelFloor);

        // Extract the translation part of the model matrix to get the position of the tree
        glm::vec3 treePosition = glm::vec3(model[3]);

		camera.updateCam(treePosition, glm::vec3(0.f, 1.f, 0.f));

		// Setup view, projection, sun matrices
        glm::mat4 view = glm::mat4(1.f);
        view = glm::lookAt(camera.getPosition(), camera.getTarget(), camera.getUp());
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection = glm::mat4(1.f);
        projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 10000.f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 modelSun = glm::mat4(1.f);
        tester->ModelSun(&modelSun);

        // Pass the light and view position to the shaders
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(sphereProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        glUniform3f(glGetUniformLocation(sphereProgram, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);

        // Setup light space matrix
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 200.0f);
        glm::vec3 lightDir = glm::normalize(lightPos - glm::vec3(0.f, -1.8f, -3.f));
        glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, camera.getUp());
        glm::mat4 projectedLightSpaceMatrix = lightProjection * lightView;
        generateDepthMap(shadowProgram, shadow, projectedLightSpaceMatrix, objs, objs2);
		//saveShadowMapToBitmap(shadow.Texture, SH_MAP_WIDTH, SH_MAP_HEIGHT);

        glViewport(0, 0, WIDTH, HEIGHT);

		// Draw the sphere
		drawSphere(sphereProgram, sphereVAO, numVertices, view, projection, modelSun);

		glBindVertexArray(0); // Unbind VAO

        glUseProgram(shaderProgram); // Switch back to the original 

		// Draw the objects
        drawObject(objs2, modelFloor, shaderProgram, 50);
		drawObject(objs, model, shaderProgram, 1);
		        
        glfwSwapBuffers(window);

        glfwPollEvents();
        processKeyboard(window);
    }

    delete[] vert;

    glfwTerminate();

    return 0;
}

