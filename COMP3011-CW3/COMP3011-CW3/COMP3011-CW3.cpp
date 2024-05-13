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
#define SH_MAP_WIDTH 4096
#define SH_MAP_HEIGHT 4096
#define PI 3.14159265358979323846

Camera camera = Camera();

// Define the vertices for sun
int numLatitudeLines = 100;
int numLongitudeLines = 100;

// Define the last mouse position, starting at the center of the screen
double lastX = 0.f;
double lastY = 0.f;

// Define light intensity, used often
float lightIntensity;

// Define control points for the Bezier curve
glm::vec3 controlPoints[4] = {
    glm::vec3(0.0f, 0.0f, 10.0f),
    glm::vec3(-10.0f, 0.0f, -10.0f),
    glm::vec3(10.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 0.0f, 10.0f)
};

// Define the position of the light source (the sun)
glm::vec3 lightPos = glm::vec3(8.f, 20.f, 60.f);

int durationOfSunset = 30; // Duration of the sunset in seconds

// Resizing the window
void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

// Rest of the keyboard inputs are in camera.cpp
void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// Deal with mouse movement (just call camera function)
void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
    camera.processMouseMovement(window, xPos, yPos);
}

// Generate vertices for the sphere then flatten them into a float array
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

// Setup the VAO and VBO for the sphere
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

// Generic function to setup objects
vector<Object> setupObject(vector<Object> objs) {
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
        if (std::string(objs[i].mtl.emit_fil_name) != "none") {
            objs[i].emitTexture = CreateTexture(objs[i].mtl.emit_fil_name); // Load emit map
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


// Setup for the floor
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

// Draw the sphere
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

// Generic function to draw objects
void drawObject(vector<Object> objs, glm::mat4 model, int shaderProgram, int texScale) {
    for (int i = 0; i < objs.size(); i++)
    {
		glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, objs[i].texture);
        
        if (objs[i].specularTexture != -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, objs[i].specularTexture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture_specular"), 1);
        }
        if (objs[i].emitTexture != -1) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, objs[i].emitTexture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture_emit"), 2);
			objs[i].emitIntensity = 15.0f;
            glUniform1f(glGetUniformLocation(shaderProgram, "emitIntensity"), objs[i].emitIntensity);
            //glm::vec3 pointLightPos = objs[i].position;
			glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3f(glGetUniformLocation(shaderProgram, "pointLightPos"), -3.f, 2.f, 1.f);
			glUniform3f(glGetUniformLocation(shaderProgram, "pointLightColor"), pointLightColor.x, pointLightColor.y, pointLightColor.z);
        }
        glBindVertexArray(objs[i].VAO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shaderProgram, "texScale"), texScale);
        glDrawArrays(GL_TRIANGLES, 0, (objs[i].tris.size() * 3));
    }
}

// Generate depth map for shadow
void generateDepthMap(unsigned int shadowShaderProgram, ShadowStruct shadow, glm::mat4 projectedLightSpaceMatrix, 
    vector<Object> objs, vector<Object> objs2, vector<Object> objs3, vector<Object> objs4,
    glm::mat4 sheepModel,glm::mat4 treeModel, glm::mat4 floorModel, glm::mat4 lampModel)
{
    glViewport(0, 0, SH_MAP_WIDTH, SH_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "projectedLightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(projectedLightSpaceMatrix));
    drawObject(objs2, floorModel, shadowShaderProgram, 10);
	drawObject(objs, treeModel, shadowShaderProgram, 1);
	drawObject(objs3, sheepModel, shadowShaderProgram, 1);
	drawObject(objs4, lampModel, shadowShaderProgram, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Calculate the light space matrix
glm::mat4 calculateLSM(glm::vec3 treePosition, glm::vec3 &lightDir) {
	if (lightIntensity < 0.2f) {
        // Setup light space matrix
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.f, 20.0f);
        glm::vec3 plPos = glm::vec3(-3.f, 2.f, 1.f);
        glm::vec3 plDir = glm::normalize(glm::vec3(-3.f, 0.f, 1.f) - plPos);
        glm::mat4 lightView = glm::lookAt(plPos, plDir - plPos, glm::vec3(0.f, 1.f, 0.f));
        return lightProjection * lightView;
	}
    else {
        // Setup light space matrix
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.f, 140.0f);
        lightDir = glm::normalize(treePosition - lightPos);
        glm::mat4 lightView = glm::lookAt(lightPos, lightDir - lightPos, glm::vec3(0.f, 1.f, 0.f));
        return lightProjection * lightView;
    }
}

// Calculate the sky color and light intensity
glm::vec3 calculateSkyColour(unsigned int shaderProgram) {
    // Calculate the sky color
    glm::vec3 dayColor = glm::vec3(0.51f, 0.79f, 1.f); // Sky color during the day
    glm::vec3 sunsetColor = glm::vec3(0.99f, 0.37f, 0.33f); // Sky color during the sunset
    glm::vec3 duskColor = glm::vec3(0.2f, 0.2f, 0.2f); // Sky color during the dusk
    glm::vec3 skyColor;
    // Calculate the light intensity
    float duskStartThreshold = 25.f; // y-coordinate where dusk starts
    float duskEndThreshold = 0.0f; // y-coordinate where it's fully dark
    lightIntensity = 1.f;

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

// Calculate the position of the sun
glm::mat4 calculateSunPos(glm::mat4 modelSun) {
	// Calculate the current time for moving the sun
    float t = fmod(glfwGetTime(), durationOfSunset) / durationOfSunset;

    // Calculate the new position of the sun
    float radiusY = 30.0f; // Radius of the ellipse in the y-axis
    float radiusZ = 120.0f; // Radius of the ellipse in the z-axis
    float h = 0.0f; // y-coordinate of the center
    float k = 0.0f; // z-coordinate of the center
	float y = h + radiusY * cos(2 * PI * t); // Calculate the y-coordinate of the sun
	float z = k + radiusZ * sin(2 * PI * t); // Calculate the z-coordinate of the sun
	lightPos = glm::vec3(lightPos.x, y, z); // Update the position of the light source (x remains the same)

    modelSun = glm::mat4(1.f); // Reset the model
	modelSun = glm::translate(modelSun, lightPos); // Translate the sun to the new position
    modelSun = glm::scale(modelSun, glm::vec3(2.f, 2.f, 2.f)); // Scale the sun to make it larger
	return modelSun;
}

// Update the position of the sheep based on Bezier curve
void updateSheepPosition(glm::mat4& modelSheep) {
    // Calculate the current time t
    float currentTime = glfwGetTime();
    float animationDuration = 20.0f; // Change this value to adjust the speed of the sheep
    float t = fmod(currentTime, animationDuration) / animationDuration;

    glm::vec3 sheepPosition = static_cast<float>(pow((1 - t), 3)) * controlPoints[0]
        + 3.f * static_cast<float>(pow((1 - t), 2)) * t * controlPoints[1]
        + 3.f * (1 - t) * static_cast<float>(pow(t, 2)) * controlPoints[2]
        + static_cast<float>(pow(t, 3)) * controlPoints[3];

    // Store the current position
    glm::vec3 currentPosition = sheepPosition;

    // Calculate the next position
    float delta = 0.01f;
	float tNext = fmod(currentTime + delta, animationDuration) / animationDuration; // Next time position for rotation
    glm::vec3 nextPosition = static_cast<float>(pow((1 - tNext), 3)) * controlPoints[0]
        + 3.f * static_cast<float>(pow((1 - tNext), 2)) * tNext * controlPoints[1]
        + 3.f * (1 - tNext) * static_cast<float>(pow(tNext, 2)) * controlPoints[2]
        + static_cast<float>(pow(tNext, 3)) * controlPoints[3];

    // Calculate the direction and rotation
	glm::vec3 direction = glm::normalize(nextPosition - currentPosition); // Calculate the direction its going in
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
	up = glm::cross(direction, right); // Calculate the up vector

	// Calculate the rotation matrix (overcomplicated because I couldnt make it look smooth any other way)
    glm::mat4 rotation = glm::mat4(1.f);
    rotation[0] = glm::vec4(right, 0.f);
    rotation[1] = glm::vec4(up, 0.f);
    rotation[2] = glm::vec4(direction, 0.f);

    // Update the sheep model
	modelSheep = glm::mat4(1.f); // Reset the model
	modelSheep = glm::translate(modelSheep, currentPosition); // Move to the current position
    modelSheep *= rotation; // Apply the rotation (couldnt use normal method)
	modelSheep = glm::scale(modelSheep, glm::vec3(.02f, .02f, .02f)); // Scale the sheep (its huge)

    // Store nextpos as current pos
    sheepPosition = nextPosition;
}

// Update the shaders
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
	// Pre-setup for the sphere
    float* vert = generateSphereVert(numLatitudeLines, numLongitudeLines);
    int numVertices = numLatitudeLines * numLongitudeLines * 6;

	// Setup the GL window
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

	// Compile the shaders
    unsigned int shaderProgram = CompileShader("textured.vert", "textured.frag");
	unsigned int sphereProgram = CompileShader("triangle.vert", "triangle.frag");
	unsigned int shadowProgram = CompileShader("shadow.vert", "shadow.frag");

	// Setup the objects
    Obj1 obj;
    Obj1* objp = &obj;
    vector<Object> treeObjs = objp->ParseTree();
	treeObjs = setupObject(treeObjs);
	vector<Object> floorObjs = objp->ParseFloor();
	floorObjs = setupFloor(floorObjs);
	vector<Object> sheepObjs = objp->ParseSheep();
	sheepObjs = setupObject(sheepObjs);
	vector<Object> lampObjs = objp->ParseLamp();
	lampObjs = setupObject(lampObjs);
    
    // Define the VAO and VBO for the sphere
    unsigned int sphereVAO, sphereVBO;
	setupSphere(&sphereVAO, &sphereVBO, numVertices, vert);

    // Unbind the VAO
    glBindVertexArray(0);

	// Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Enable depth testing and alpha blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Ensure a program is active
    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        camera.processKeyboard(window); // Process inputs

		// Setup the model matrices
        glm::mat4 modelTree = glm::mat4(1.f);
        objp->ModelTree(&modelTree);
		glm::mat4 modelFloor = glm::mat4(1.f);
		objp->ModelFloor(&modelFloor);
		glm::mat4 modelSheep = glm::mat4(1.f);
		objp->ModelSheep(&modelSheep);
		glm::mat4 modelLamp = glm::mat4(1.f);
		objp->ModelLamp(&modelLamp);

		updateSheepPosition(modelSheep); // Update the position of the sheep

        // Extract the translation part of the model matrix to get the position of the tree
        glm::vec3 treePosition = glm::vec3(modelTree[3]);

		camera.updateCam(glm::vec3(treePosition.x, treePosition.y + 1, treePosition.z), glm::vec3(0.f, 1.f, 0.f)); // Update the camera (point slightly upwards)

		// Setup view, projection, sun matrices
        glm::mat4 view = glm::mat4(1.f);
        view = glm::lookAt(camera.getPosition(), camera.getTarget(), camera.getUp());
        glm::mat4 projection = glm::mat4(1.f);
        projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 10000.f);
		glm::mat4 modelSun = glm::mat4(1.f);
		modelSun = calculateSunPos(modelSun); // Calculate the position of the sun

		// Calculate the light space matrix
		glm::vec3 lightDir;
		glm::mat4 projectedLightSpaceMatrix = calculateLSM(treePosition, lightDir);

		// Calculate the sky color
		glm::vec3 skyColor = calculateSkyColour(shaderProgram);
        
		updateShaders(shaderProgram, sphereProgram, view, projection, projectedLightSpaceMatrix, lightDir); // Update the shaders

		// Shadow map
        generateDepthMap(shadowProgram, shadow, projectedLightSpaceMatrix, treeObjs, floorObjs, sheepObjs, lampObjs, modelSheep, modelTree, modelFloor, modelLamp);
		//saveShadowMapToBitmap(shadow.Texture, SH_MAP_WIDTH, SH_MAP_HEIGHT);

		// Reset the viewport and clear the screen
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(skyColor.x, skyColor.y, skyColor.z, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Draw the sphere
		drawSphere(sphereProgram, sphereVAO, numVertices, view, projection, modelSun);

        glUseProgram(shaderProgram); // Switch back to original shader
		// Bind then pass the shadow map to the shaders
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, shadow.Texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 3);

		// Draw the objects
        drawObject(floorObjs, modelFloor, shaderProgram, 20);
		drawObject(sheepObjs, modelSheep, shaderProgram, 1);
		drawObject(lampObjs, modelLamp, shaderProgram, 1);
        drawObject(treeObjs, modelTree, shaderProgram, 1);
		
        
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
        processKeyboard(window);
    }
    delete[] vert;
    glfwTerminate();
    return 0;
}