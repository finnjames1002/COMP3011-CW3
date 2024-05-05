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


void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}



void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// Define the vertices for an untextured triangle
float vert[] = {
	// positions          // colors
	-0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
	0.0f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f
};


int main(int argc, char** argv)
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(width, height, "Assessment 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, SizeCallback);

    gl3wInit();

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebguMessageCallback, 0);

    unsigned int shaderProgram = CompileShader("textured.vert", "textured.frag");
	unsigned int triangleProgram = CompileShader("triangle.vert", "triangle.frag");

    //SECTION A - EDIT THIS CODE TO TEST
    Test0 T0;
    Test1 T1;
    Test2 T2;
    Test* tester = &T2;
    vector<Object> objs = tester->ParseObj();
    //SECTION A - EDIT THIS CODE TO TEST

    //SECTION B - SETTING UP TEXTURE AND VAO FOR EACH OBJECT. DO NOT EDIT THIS CODE 
    for (int i = 0; i < objs.size(); i++)
    {
        // render the triangle stored in vertices
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
    }
    //SECTION B - SETTING UP TEXTURE AND VAO FOR EACH OBJECT. DO NOT EDIT THIS CODE 

    // Define the VAO and VBO for the triangle
    unsigned int triangleVAO, triangleVBO;
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    // Bind and set vertex buffer(s) and attribute pointer(s)
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

        glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glm::mat4 view = glm::mat4(1.f);
        view = glm::translate(view, -glm::vec3(0.f, 0.f, 2.f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::mat4(1.f);
        projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 10000.f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the triangle first
        glUseProgram(triangleProgram); // Use the triangle shader program
        glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(triangleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(triangleVAO);
        glEnable(GL_DEPTH_TEST); // Enable depth testing
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisable(GL_DEPTH_TEST); // Disable depth testing
        glBindVertexArray(0);

        glUseProgram(shaderProgram); // Switch back to the original 

        glUseProgram(shaderProgram); // Switch back to the original shader program

        //SECTION C - TEXTURE BINDING, MODEL MATRIX AND DRAW CALL FOR EACH OBJECT. DO NOT EDIT THIS CODE 
        for (int i = 0; i < objs.size(); i++)
        {
            glBindTexture(GL_TEXTURE_2D, objs[i].texture);

            glBindVertexArray(objs[i].VAO);

            glm::mat4 model = glm::mat4(1.f);
            tester->Model(&model);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, (objs[i].tris.size() * 3));
        }
        //SECTION C - TEXTURE BINDING, MODEL MATRIX AND DRAW CALL FOR EACH OBJECT. DO NOT EDIT THIS CODE 

        glfwSwapBuffers(window);

        glfwPollEvents();
        processKeyboard(window);
    }

    glfwTerminate();

    return 0;
}

