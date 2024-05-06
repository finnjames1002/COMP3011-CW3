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

const int width = 1200;
const int height = 800;
float radius = 15.0f; // Camera's distance from the center of rotation


void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}



void processKeyboard(GLFWwindow* window)
{
    float zoomSpeed = 0.005f; // The speed of zooming
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // Zoom in
    {
        radius -= zoomSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // Zoom out
    {
        radius += zoomSpeed;
    }
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


// Define the indices for the floor
GLuint floorIndices[] = {
    0, 1, 2,
    2, 3, 0
};


const float PI = 3.14159265358979323846;

struct Vertex {
    float x, y, z;
};

float* generateSphereVert(int numLatitudeLines, int numLongitudeLines) {
    std::vector<Vertex> vertices;

    for (int i = 0; i < numLatitudeLines; ++i) {
        for (int j = 0; j <= numLongitudeLines; ++j) {
            for (int k = 0; k <= 1; ++k) {
                float theta = (i + k) * PI / numLatitudeLines;
                float phi = j * 2 * PI / numLongitudeLines;

                Vertex vertex;
                vertex.x = std::sin(theta) * std::cos(phi);
                vertex.y = std::sin(theta) * std::sin(phi);
                vertex.z = std::cos(theta);

                vertices.push_back(vertex);
            }
        }
    }

    float* flattenedVertices = new float[vertices.size() * 6]; // 6 because we now have 3 coordinates and 3 color values

    for (int i = 0; i < vertices.size(); ++i) {
        flattenedVertices[i * 6] = vertices[i].x;
        flattenedVertices[i * 6 + 1] = vertices[i].y;
        flattenedVertices[i * 6 + 2] = vertices[i].z;
        flattenedVertices[i * 6 + 3] = 1.0f; // Red
        flattenedVertices[i * 6 + 4] = 0.87f; // Green
        flattenedVertices[i * 6 + 5] = 0.13f; // Blue
    }

    return flattenedVertices;
}


// Define the vertices for an untextured triangle
int numLatitudeLines = 100;
int numLongitudeLines = 100;


int main(int argc, char** argv)
{
    float* vert = generateSphereVert(numLatitudeLines, numLongitudeLines);
    int numVertices = numLatitudeLines * numLongitudeLines * 6;
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8); // Enable 8x MSAA
    GLFWwindow* window = glfwCreateWindow(width, height, "Assessment 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, SizeCallback);


    gl3wInit();
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugMessageCallback, 0);

    unsigned int shaderProgram = CompileShader("textured.vert", "textured.frag");
	unsigned int sphereProgram = CompileShader("triangle.vert", "triangle.frag");

    //SECTION A - EDIT THIS CODE TO TEST
    //Test0 T0;
    Test1 T1;
    Test2 T2;
    Test* tester = &T2;
    vector<Object> objs = tester->ParseObj();

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
        glEnableVertexAttribArray(2); // Add this line
    }

	vector<Object> objs2 = T1.ParseObj();

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

    glBindVertexArray(0); // Unbind VAO

    // Define the VAO and VBO for the sphere
    unsigned int sphereVAO, sphereVBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);

    // Bind and set vertex buffer(s) and attribute pointer(s) for the sphere
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Define the position of the light source (the sun)
    glm::vec3 lightPos = glm::vec3(0.f, 10.f, -80.f); // Replace with the actual position of your sun

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
        processKeyboard(window);

        glClearColor(0.51f, 0.79f, 1.f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glm::mat4 model = glm::mat4(1.f);
        tester->Model(&model);

		glm::mat4 modelFloor = glm::mat4(1.f);
		tester->ModelFloor(&modelFloor);

        // Extract the translation part of the model matrix to get the position of the tree
        glm::vec3 treePosition = glm::vec3(model[3]);

        float cameraSpeed = (float)glfwGetTime() / 2; // Speed of camera rotation
        
		float yOffset = 1.0f; // Height of the camera above the tree

        // Calculate the new position of the camera based on the rotation angle and the position of the tree
        glm::vec3 cameraPos = glm::vec3(
            treePosition.x + radius * cos(cameraSpeed),
            treePosition.y + yOffset,
            treePosition.z + radius * sin(cameraSpeed)
        );



        glm::vec3 cameraTarget = treePosition; // Make the camera look at the tree
        glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

        glm::mat4 view = glm::mat4(1.f);
        view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::mat4(1.f);
        projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 10000.f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 modelSun = glm::mat4(1.f);
        tester->ModelSun(&modelSun);

        // Pass the light position to the shaders
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(sphereProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        // Pass the view position (camera position) to the shaders
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(sphereProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);


        // Draw the sphere first
        glUseProgram(sphereProgram); // Use the sphere shader program
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // Use the same model matrix as the textured objects
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelSun));
        glBindVertexArray(sphereVAO);
        glEnable(GL_DEPTH_TEST); // Enable depth testing
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);

        glBindVertexArray(0);

        glUseProgram(shaderProgram); // Switch back to the original 


        //SECTION C - TEXTURE BINDING, MODEL MATRIX AND DRAW CALL FOR EACH OBJECT. DO NOT EDIT THIS CODE 
        for (int i = 0; i < objs2.size(); i++)
        {
            int texScale = 50;
            glBindTexture(GL_TEXTURE_2D, objs2[i].texture);

            glBindVertexArray(objs2[i].VAO);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelFloor));

            glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), texScale);
            glDrawArrays(GL_TRIANGLES, 0, (objs2[i].tris.size() * 3));
        }

        //SECTION C - TEXTURE BINDING, MODEL MATRIX AND DRAW CALL FOR EACH OBJECT. DO NOT EDIT THIS CODE 
        for (int i = 0; i < objs.size(); i++)
        {
			int texScale = 1;
            glBindTexture(GL_TEXTURE_2D, objs[i].texture);

            glBindVertexArray(objs[i].VAO);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), texScale);
            glDrawArrays(GL_TRIANGLES, 0, (objs[i].tris.size() * 3));
        }
        

        glfwSwapBuffers(window);

        glfwPollEvents();
        processKeyboard(window);
    }

    delete[] vert;

    glfwTerminate();

    return 0;
}

