#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>

using namespace std;

#include "struct.h"
#include "error.h"
#include "parser.h"
#include "shader.h"
#include "texture.h"
#include "obj.h"
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

double lastX = 0.f;
double lastY = 0.f;

// Define the position of the light source (the sun)
glm::vec3 lightPos = glm::vec3(0.f, 20.f, 60.f);

int durationOfSunset = 10; // Duration of the sunset in seconds

void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {

    if (camera.cameraType == CAMERA_FREE) {
        camera.processMouseMovement(window, xPos, yPos);
    }
}



float* generateSphereVert(int numLatitudeLines, int numLongitudeLines) {
    std::vector<Vertex> vertices;

    for (int i = 0; i < numLatitudeLines; ++i) {
        for (int j = 0; j <= numLongitudeLines; ++j) {
            for (int k = 0; k <= 1; ++k) {
                float theta = (i + k) * (2 * PI) / numLatitudeLines;
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
        
        if (std::string(objs[i].mtl.specular_fil_name) != "none") {
            objs[i].specularTexture = CreateTexture(objs[i].mtl.specular_fil_name); // Load spec map
        }
        

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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, objs[i].texture);
        /*
        if (objs[i].specularTexture != -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, objs[i].specularTexture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture_specular"), 1);
        }*/
        glBindVertexArray(objs[i].VAO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), texScale);
        glDrawArrays(GL_TRIANGLES, 0, (objs[i].tris.size() * 3));
    }
}

void drawFloor(vector<Object> objs, glm::mat4 model, int shaderProgram, int texScale) {
    for (int i = 0; i < objs.size(); i++)
    {
        glUseProgram(shaderProgram);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, objs[i].texture);
        glBindVertexArray(objs[i].VAO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), texScale);
        glDrawArrays(GL_TRIANGLES, 0, (objs[i].tris.size() * 3));
    }
}

void generateDepthMap(unsigned int shadowShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix, vector<Object> objs, vector<Object> objs2, glm::mat4 treeModel, glm::mat4 floorModel)
{
    glViewport(0, 0, SH_MAP_WIDTH, SH_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
    drawFloor(objs2, floorModel, shadowShaderProgram, 10);
	drawObject(objs, treeModel, shadowShaderProgram, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 calculateLSM(glm::vec3 treePosition, glm::vec3 &lightDir) {
    // Setup light space matrix
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.f, 100.0f);
    lightDir = glm::normalize(treePosition - lightPos);
    glm::mat4 lightView = glm::lookAt(lightPos, lightDir - lightPos, glm::vec3(0.f, 1.f, 0.f));
	return lightProjection * lightView;
}

glm::vec3 calculateSkyColour(unsigned int shaderProgram) {
    // Calculate the sky color
    glm::vec3 dayColor = glm::vec3(0.51f, 0.79f, 1.f); // Sky color during the day
    glm::vec3 sunsetColor = glm::vec3(0.99f, 0.37f, 0.33f); // Sky color during the sunset
    glm::vec3 duskColor = glm::vec3(0.2f, 0.2f, 0.2f); // Sky color during the dusk
    glm::vec3 skyColor;
    // Calculate the light intensity
    float duskStartThreshold = 25.f; // y-coordinate where dusk starts
    float duskEndThreshold = 0.0f; // y-coordinate where it's fully dark
    float lightIntensity = 1.f;

    if (lightPos.y <= duskEndThreshold) {
        lightIntensity = 0.1f; // Fully dark when the sun is below the end threshold
        skyColor = duskColor; // Sunset color when the sun is below the end threshold
    }
    else if (lightPos.y <= duskStartThreshold) {
        // Smoothly transition from full intensity to dark between the start and end thresholds
        lightIntensity = 0.2 + ((lightPos.y - duskEndThreshold) / (duskStartThreshold - duskEndThreshold));
        float factor = (lightPos.y - duskEndThreshold) / (duskStartThreshold - duskEndThreshold);
        skyColor = dayColor * factor + sunsetColor * (1 - factor);
    }
    else {
        skyColor = dayColor; // Day color when the sun is above the start threshold
        lightIntensity = 1.0f; // Full intensity when the sun is above the start threshold
    }

    // Clamp the light intensity between 0 and 1
    lightIntensity = glm::clamp(lightIntensity, 0.0f, 1.0f);

    // Pass the light intensity to the shaders
    glUniform1f(glGetUniformLocation(shaderProgram, "lightIntensity"), lightIntensity);

	return skyColor;
}

glm::mat4 calculateSunPos(glm::mat4 modelSun) {
    // Calculate the current time t
    float t = fmod(glfwGetTime(), durationOfSunset) / durationOfSunset;

    // Calculate the new position of the sun
    float radiusY = 30.0f; // Radius of the ellipse in the y-axis
    float radiusZ = 120.0f; // Radius of the ellipse in the z-axis
    float h = 0.0f; // y-coordinate of the ellipse's center
    float k = 0.0f; // z-coordinate of the ellipse's center
    float y = h + radiusY * cos(2 * PI * t);
    float z = k + radiusZ * sin(2 * PI * t);
    lightPos = glm::vec3(lightPos.x, y, z);

    modelSun = glm::mat4(1.f); // Reset the model matrix
    modelSun = glm::translate(modelSun, lightPos);
    modelSun = glm::scale(modelSun, glm::vec3(2.f, 2.f, 2.f)); // Scale the sun to make it larger
	return modelSun;
}

void updateShaders(unsigned int shaderProgram, unsigned int sphereProgram, glm::mat4 view, glm::mat4 projection, glm::mat4 projectedLightSpaceMatrix, glm::vec3 lightDir) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Pass the light and view position to the shaders
    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(sphereProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    glUniform3f(glGetUniformLocation(sphereProgram, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
    glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
}

int main(int argc, char** argv)
{
    float* vert = generateSphereVert(numLatitudeLines, numLongitudeLines);
    int numVertices = numLatitudeLines * numLongitudeLines * 6;
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8); // Enable 8x MSAA
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "COMP-3011 CW3", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, SizeCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gl3wInit();
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(DebugMessageCallback, 0);
    
    // Shadow setup
	ShadowStruct shadow = setup_shadowmap(SH_MAP_WIDTH, SH_MAP_HEIGHT);

	// Setup shaders
    unsigned int shaderProgram = CompileShader("textured.vert", "textured.frag");
	unsigned int sphereProgram = CompileShader("triangle.vert", "triangle.frag");
	unsigned int shadowProgram = CompileShader("shadow.vert", "shadow.frag");

	// Setup the objects
    Obj1 obj;
    Obj1* objp = &obj;
    vector<Object> objs = objp->ParseTree();
	objs = setupTree(objs);
	vector<Object> objs2 = objp->ParseFloor();
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

        glm::mat4 modelTree = glm::mat4(1.f);
        objp->Model(&modelTree);

		glm::mat4 modelFloor = glm::mat4(1.f);
		objp->ModelFloor(&modelFloor);

        // Extract the translation part of the model matrix to get the position of the tree
        glm::vec3 treePosition = glm::vec3(modelTree[3]);

		camera.updateCam(treePosition, glm::vec3(0.f, 1.f, 0.f));

		// Setup view, projection, sun matrices
        glm::mat4 view = glm::mat4(1.f);
        view = glm::lookAt(camera.getPosition(), camera.getTarget(), camera.getUp());
        glm::mat4 projection = glm::mat4(1.f);
        projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 10000.f);
		

		glm::mat4 modelSun = glm::mat4(1.f);
		modelSun = calculateSunPos(modelSun);

		// Calculate the light space matrix
		glm::vec3 lightDir;
		glm::mat4 projectedLightSpaceMatrix = calculateLSM(treePosition, lightDir);

		// Calculate the sky color
		glm::vec3 skyColor = calculateSkyColour(shaderProgram);
        
        updateShaders(shaderProgram, sphereProgram, view, projection, projectedLightSpaceMatrix, lightDir);

		// Shadow map
        generateDepthMap(shadowProgram, shadow, projectedLightSpaceMatrix, objs, objs2, modelTree, modelFloor);
		//saveShadowMapToBitmap(shadow.Texture, SH_MAP_WIDTH, SH_MAP_HEIGHT);

		// Reset the viewport and clear the screen
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(skyColor.x, skyColor.y, skyColor.z, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Draw the sphere
		drawSphere(sphereProgram, sphereVAO, numVertices, view, projection, modelSun);

        glUseProgram(shaderProgram); // Switch back to original shader
        glBindTexture(GL_TEXTURE_2D, shadow.Texture);
        
		// Draw the objects
        drawFloor(objs2, modelFloor, shaderProgram, 10);
		drawObject(objs, modelTree, shaderProgram, 1);
        
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
        processKeyboard(window);
    }
    delete[] vert;
    glfwTerminate();
    return 0;
}