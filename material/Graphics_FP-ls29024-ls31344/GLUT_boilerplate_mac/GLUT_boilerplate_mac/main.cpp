/*
    GRAPHICS PROGRAMMING
    Final Project

    Author: Lluís Masdeu (ls31344)
    Author: Oriol Ramis (ls29024)
*/

// Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

// OpenGL GLUT & GLM Libraries
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <string>
#include <vector>

// Include of the class in charge of reading '.obj' files
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "imageloader.h"

// Include Shader class
#include "Shader.h"

using namespace glm;

// ------------------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------------------

// Default size of the window in pixels
int g_ViewportWidth = 640;
int g_ViewportHeight = 640;

// Background color
const vec3 g_ColorFons(0.2f, 0.2f, 0.2f);

// Variables where we store the Shaders.
GLuint g_UniverseShaderProgram = 0, g_SunShaderProgram = 0, g_EarthShaderProgram = 0, g_JupiterShaderProgram = 0,
    g_MarsShaderProgram = 0, g_NeptuneShaderProgram = 0, g_VenusShaderProgram = 0;

// VAOs (Vertex Array Object) vector where we store the objects' geometry we'll use
std::vector<GLuint> vaos;

// Variables where we'll store the different textures
GLuint textureMilkyway, textureSun, textureEarth, textureEarthNight, textureEarthNormal, textureEarthSpec,
    textureJupiter, textureMars, textureMarsNormal, textureMercury, textureMercuryNormal, textureNeptune,
    textureVenus;

// Vector which will contain each object's number of triangles
std::vector<GLuint> numTriangles;

// Vectors with the positions of the different planets.
vec3 g_sunPosition = glm::vec3(0.0f, 0.0f, 0.0f), g_earthPosition = glm::vec3(1.0f, 0.0f, -8.9f),
    g_jupiterPosition = glm::vec3(6.0f, 0.0f, -4.5f), g_marsPosition = glm::vec3(-16.3f, 0.0f, -4.5f),
    g_neptunePosition = glm::vec3(6.5f, 0.0f, 7.75f), g_venusPosition = glm::vec3(-6.8f, 0.0f, -7.5f);

// Vectors with the information of the view matrix.
vec3 g_cameraPosition = glm::vec3(0.0f, 0.0f, 18.0f), g_cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f),
    g_upVector = glm::vec3(0.0f, 1.0f, 0.0f);

// Projection matrix for every object.              // ANNOTATION: perspective Schema:
glm::mat4 projection_matrix = glm::perspective(     // glm::perspective(
    60.0f,                                          //    Field of view,
    (float) g_ViewportHeight / g_ViewportWidth,     //    Aspect ratio,
    0.1f,                                           //    Near plane (distance from camera),
    50.0f                                           //    Far plane (distance from camera)
);                                                  // );

// Parameters used for the camera movement.
bool g_wKey = false, g_aKey = false, g_sKey = false, g_dKey = false, g_fKey = false, g_gKey = false;
double camDistance = 0.0f;
float g_angle = 0.0, deltaAngle = 0.0f, rAngle = 0.0f;
int xOrigin = -1;

// Parameters used for the drawing of the planets.
float g_ambient = 0.5f, g_shininess = 30.0f, g_transparency = 1.0f;
vec3 g_lightColor = glm::vec3(1.0f, 1.0f, 1.0f), g_lightColorMars = glm::vec3(0.86f, 0.48f, 0.20f);

// ------------------------------------------------------------------------------------------
// Function in which we'll open and store the different resources we'll use through the
// program.
// ------------------------------------------------------------------------------------------
void createGeometry() {
    // Open the object and declare the varibales which will store its parameters.
    std::string basePath = "/Users/lluismasdeu/Desktop/marysun-scarabagel/Graphics_FP-ls29024-ls31344/GLUT_boilerplate_mac/GLUT_boilerplate_mac/";
    std::string objectsPath = basePath + "objects/";
    std::string texturesPath = basePath + "textures/";

    std::string inputFile = objectsPath + "sphere.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    // Save the different textures.
    Image * milkywayImage = loadBMP((texturesPath + "milkywaymap.bmp").c_str());
    Image * sunImage = loadBMP((texturesPath + "sunmap.bmp").c_str());
    Image * earthImage = loadBMP((texturesPath + "earthmap.bmp").c_str());
    Image * earthNightImage = loadBMP((texturesPath + "earthnightmap.bmp").c_str());
    Image * earthNormal = loadBMP((texturesPath + "earthnormal.bmp").c_str());
    Image * earthSpec = loadBMP((texturesPath + "earthspecular.bmp").c_str());
    Image * jupiterImage = loadBMP((texturesPath + "jupitermap.bmp").c_str());
    Image * marsImage = loadBMP((texturesPath + "marsmap.bmp").c_str());
    Image * marsNormal = loadBMP((texturesPath + "marsnormal.bmp").c_str());
    Image * neptuneImage = loadBMP((texturesPath + "neptunemap.bmp").c_str());
    Image * venusImage = loadBMP((texturesPath + "venusmap.bmp").c_str());

    bool ret;
    ret = tinyobj::LoadObj(shapes, materials, err, inputFile.c_str(), basePath.c_str());

    // Check there is no error.
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    // Display the number of shapes of the read file.
    std::cout << "# of shapes : " << shapes.size() << std::endl;

    // Variable which'll serve to store the buffers
    GLuint buffer;

    // The same procedure for every object of the file.
    for (int i = 0; i < shapes.size(); i++) {
        // Add a position in the dynamic array of VAOs, and reset the buffer variable.
        vaos.push_back(0);
        buffer = 0;

        // Create VAO which'll contain the geometry of the object.
        glGenVertexArraysAPPLE(1, &vaos[i]);
        glBindVertexArrayAPPLE(vaos[i]);

        // Create VBO for the vertices.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * 3 * sizeof(float),
                     &(shapes[i].mesh.positions[0]), GL_STATIC_DRAW);

        // Activate the vertices' attribute in Shader.
        GLuint vertexLoc = glGetAttribLocation(g_EarthShaderProgram, "a_vertexPos");
        glEnableVertexAttribArray(vertexLoc);
        glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Reset the buffer variable.
        buffer = 0;

        // Create VBO for the textures.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size() * 2 * sizeof(float),
                     &(shapes[i].mesh.texcoords[i]), GL_STATIC_DRAW);

        // Activate the UVS' attribute in Shader.
        GLuint uvLoc = glGetAttribLocation(g_EarthShaderProgram, "a_uvs");
        glEnableVertexAttribArray(uvLoc);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Reset the buffer variable.
        buffer = 0;

        // Create VBO for the normals.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() * 3 * sizeof(float),
                     &(shapes[i].mesh.normals[i]), GL_STATIC_DRAW);

        // Activate normals' attribute in Shader.
        GLuint normals = glGetAttribLocation(g_EarthShaderProgram, "a_normal");
        glEnableVertexAttribArray(normals);
        glVertexAttribPointer(normals, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Create VBO for the indexes.
        glGenBuffers(1, &buffer); //create more new buffers
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     shapes[i].mesh.indices.size() * sizeof(unsigned int), &(shapes[i].mesh.indices[0]),
                     GL_STATIC_DRAW);

        // Store the number of triangles of every object.
        numTriangles.push_back(shapes[i].mesh.indices.size() / 3);

        glBindVertexArrayAPPLE(0); //unbind VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers
    }

    // Universe
    glGenTextures(1, &textureMilkyway);
    glBindTexture(GL_TEXTURE_2D, textureMilkyway);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       // ANNOTATION: glTexImage2D Schema
    glTexImage2D(                                                           // glTexImage2D(
        GL_TEXTURE_2D,                                                      //    target,
        0,                                                                  //    level, (= 0, no mipmap)
        GL_RGB,                                                             //    the we opengl stores the data,
        milkywayImage->width,                                               //    width,
        milkywayImage->height,                                              //    height,
        0,                                                                  //    border, (must be 0)
        GL_RGB,                                                             //    format of the original data,
        GL_UNSIGNED_BYTE,                                                   //    type of data,
        milkywayImage->pixels                                               //    pointer to start of data
    );                                                                      // );

    // Sun
    glGenTextures(1, &textureSun);
    glBindTexture(GL_TEXTURE_2D, textureSun);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        sunImage->width,
        sunImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        sunImage->pixels
    );

    // Earth
    glGenTextures(1, &textureEarth);
    glBindTexture(GL_TEXTURE_2D, textureEarth);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        earthImage->width,
        earthImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        earthImage->pixels
    );

    // Earth - Night
    glGenTextures(1, &textureEarthNight);
    glBindTexture(GL_TEXTURE_2D, textureEarthNight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        earthNightImage->width,
        earthNightImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        earthNightImage->pixels
    );

    // Earth - Normals
    glGenTextures(1, &textureEarthNormal);
    glBindTexture(GL_TEXTURE_2D, textureEarthNormal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        earthNormal->width,
        earthNormal->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        earthNormal->pixels
    );

    // Earth - Specular
    glGenTextures(1, &textureEarthSpec);
    glBindTexture(GL_TEXTURE_2D, textureEarthSpec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        earthSpec->width,
        earthSpec->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        earthSpec->pixels
    );

    // Jupiter
    glGenTextures(1, &textureJupiter);
    glBindTexture(GL_TEXTURE_2D, textureJupiter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        jupiterImage->width,
        jupiterImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        jupiterImage->pixels
    );

    // Mars
    glGenTextures(1, &textureMars);
    glBindTexture(GL_TEXTURE_2D, textureMars);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        marsImage->width,
        marsImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        marsImage->pixels
    );

    // Mars - Normals
    glGenTextures(1, &textureMarsNormal);
    glBindTexture(GL_TEXTURE_2D, textureMarsNormal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        marsNormal->width,
        marsNormal->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        marsNormal->pixels
    );

    // Neptune
    glGenTextures(1, &textureNeptune);
    glBindTexture(GL_TEXTURE_2D, textureNeptune);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        neptuneImage->width,
        neptuneImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        neptuneImage->pixels
    );

    // Venus
    glGenTextures(1, &textureVenus);
    glBindTexture(GL_TEXTURE_2D, textureVenus);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        venusImage->width,
        venusImage->height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        venusImage->pixels
    );
}

// ------------------------------------------------------------------------------------------
// Load all resources
// ------------------------------------------------------------------------------------------
void loadResources()
{
    // We load the Shader and create the program for the Universe.
    Shader universeShader("universeShader.vsh", "universeShader.fsh");
    g_UniverseShaderProgram = universeShader.program;

    // We load the Shader and create the program for the Sun.
    Shader sunShader("sunShader.vsh", "sunShader.fsh");
    g_SunShaderProgram = sunShader.program;

    // We load the Shader and create the program for the Earth.
    Shader earthShader("earthShader.vsh", "earthShader.fsh");
    g_EarthShaderProgram = earthShader.program;

    // We load the Shader and create the program for Jupiter.
    Shader jupiterShader("jupiterShader.vsh", "jupiterShader.fsh");
    g_JupiterShaderProgram = jupiterShader.program;

    // We load the Shader and create the program for Mars.
    Shader marsShader("marsShader.vsh", "marsShader.fsh");
    g_MarsShaderProgram = marsShader.program;

    // We load the Shader and create the program for Neptune.
    Shader neptuneShader("neptuneShader.vsh", "neptuneShader.fsh");
    g_NeptuneShaderProgram = neptuneShader.program;

    // We load the Shader and create the program for Venus.
    Shader venusShader("venusShader.vsh", "venusShader.fsh");
    g_VenusShaderProgram = venusShader.program;

    // Create geometry and store in VAO.
    createGeometry();
}

// ------------------------------------------------------------------------------------------
// Function which'll check when selected keyboard keys are pressed left
// ------------------------------------------------------------------------------------------
void onReshape(int w, int h) {
    g_ViewportWidth = w;
    g_ViewportHeight = h;
    
    // Modify the gl viewport so that it covers all the window
    glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);
}

// ------------------------------------------------------------------------------------------
// Function which'll compute distance between camera position and camera target
// ------------------------------------------------------------------------------------------
void computeDistance() {
    camDistance = sqrt(pow(g_cameraTarget.x - g_cameraPosition.x, 2.0f)
                       + pow(g_cameraTarget.y - g_cameraPosition.y, 2.0f)
                       + pow(g_cameraTarget.z - g_cameraPosition.z, 2.0f));
}

// ------------------------------------------------------------------------------------------
// Function which'll move camera around object
// ------------------------------------------------------------------------------------------
void sideCameraMovement() {
    g_cameraPosition.x = camDistance * cos(g_angle) + g_cameraTarget.x;
    g_cameraPosition.z = camDistance * sin(g_angle) + g_cameraTarget.z;
}

// ------------------------------------------------------------------------------------------
// Function which'll check when selected keyboard keys are pressed
// ------------------------------------------------------------------------------------------
void onKeyDown(unsigned char key, int x, int y) {
    computeDistance();

    switch (key) {
        case 'w':
            g_wKey = true;
            break;

        case 'a':
            g_aKey = true;
            break;

        case 's':
            g_sKey = true;
            break;

        case 'd':
            g_dKey = true;
            break;

        case 'f':               // With this key, we'll define a temporary camera position in which
            g_fKey = true;      // the normals texture of the Earth can be seen in a proper way.
            break;

        case 'g':               // With this key, we'll define a temporary camera position in which
            g_gKey = true;      // the night texture of the Earth can be seen in a proper way.
            break;
    }
}

// ------------------------------------------------------------------------------------------
// Function which'll check when selected keyboard keys are pressed left
// ------------------------------------------------------------------------------------------
void onKeyUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            g_wKey = false;
            break;

        case 'a':
            g_aKey = false;
            break;

        case 's':
            g_sKey = false;
            break;

        case 'd':
            g_dKey = false;
            break;

        case 'f':                   // With this key, we'll define a camera position in which
            g_fKey = false;         // the normals texture of the Earth can be seen in a proper way.
            break;

        case 'g':                   // With this key, we'll define a temporary camera position in which
            g_gKey = false;         // the night texture of the Earth can be seen in a proper way.
            break;

        case 27:                    // 27 is the code for ESC key.
            exit(EXIT_SUCCESS);     // Exit the programme.
            break;
    }
}

// ------------------------------------------------------------------------------------------
// Function which'll detect when mouse button is pressed
// ------------------------------------------------------------------------------------------
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {       // Start motion if the left button is pressed.
        if (state == GLUT_UP) {             // When the button is released.
            g_angle += deltaAngle;
            xOrigin = -1;
        } else  {                             // state = GLUT_DOWN
            xOrigin = x;
        }
    }
}

// ------------------------------------------------------------------------------------------
// Function which'll detect mouse movement
// ------------------------------------------------------------------------------------------
void mouseMove(int x, int y) {
    if (xOrigin >= 0) {                             // Only true when the left button is down.
        deltaAngle = (x - xOrigin) * 0.0001f;       // Update deltaAngle with the changes made.
        g_angle += deltaAngle;
        
        glutPostRedisplay();                        // Redisplay with the changes made.
    }
}

// ------------------------------------------------------------------------------------------
// Function which'll draw the universe
// ------------------------------------------------------------------------------------------
void drawMilkyway() {
    // We'll just paint the back of the sphere.
    glCullFace(GL_FRONT);

    // Select the shader we use to paint.
    glUseProgram(g_UniverseShaderProgram);

    // Variables to store that'll pass the information to the Shader.
    GLuint u_model = glGetUniformLocation(g_UniverseShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_UniverseShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_UniverseShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_UniverseShaderProgram, "u_texture");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_cameraPosition);

    // View Matrix                                  // ANNOTATION: lookAt Schema:
    glm::mat4 view_matrix = glm::lookAt(            // glm::lookAt(
        g_cameraPosition,                           //    Camera position,
        g_cameraTarget,                             //    Camera target,
        g_upVector                                  //    Up vector
    );                                              // );

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    // Pass texture to Shader.
    glUniform1i(u_texture, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureMilkyway);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw the Sun
// ------------------------------------------------------------------------------------------
void drawSun() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_SunShaderProgram);

    GLuint u_model = glGetUniformLocation(g_SunShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_SunShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_SunShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_SunShaderProgram, "u_texture");
    GLuint u_transparency = glGetUniformLocation(g_SunShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_sunPosition);
    model_matrix *= scale(mat4(1.0f), vec3(1.15f, 1.15f, 1.15f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, -0.0001f, 0.0f));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniform1i(u_texture, 0);
    glUniform1f(u_transparency, g_transparency);

    // Pass texture to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureSun);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw the Earth
// ------------------------------------------------------------------------------------------
void drawEarth() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_EarthShaderProgram);

    GLuint u_model = glGetUniformLocation(g_EarthShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_EarthShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_EarthShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_EarthShaderProgram, "u_texture");
    GLuint u_texture_normal = glGetUniformLocation(g_EarthShaderProgram, "u_texture_normal");
    GLuint u_texture_spec = glGetUniformLocation(g_EarthShaderProgram, "u_texture_spec");
    GLuint u_texture_night = glGetUniformLocation(g_EarthShaderProgram, "u_texture_night");
    GLuint u_light_dir = glGetUniformLocation(g_EarthShaderProgram, "u_light_dir");
    GLuint u_light_color = glGetUniformLocation(g_EarthShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_EarthShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_EarthShaderProgram, "u_shininess");
    GLuint u_ambient = glGetUniformLocation(g_EarthShaderProgram, "u_ambient");
    GLuint u_normal_matrix = glGetUniformLocation(g_EarthShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_EarthShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_earthPosition);
    model_matrix *= scale(mat4(1.0f), vec3(1.05f, 1.05f, 1.05f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, 0.0005f, 0.0f));

    // Normal Matrix
    mat3 normal_matrix = inverseTranspose(mat3(model_matrix));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Light Direction
    vec3 g_lightDirection = glm::vec3(g_sunPosition.x - g_earthPosition.x, g_sunPosition.y - g_earthPosition.y,
        g_sunPosition.z - g_earthPosition.z);

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform1i(u_texture_normal, 1);
    glUniform1i(u_texture_spec, 2);
    glUniform1i(u_texture_night, 3);
    glUniform3f(u_light_dir, g_lightDirection.x, g_lightDirection.y, g_lightDirection.z);
    glUniform3f(u_light_color, g_lightColor.x, g_lightColor.y, g_lightColor.z);
    glUniform3f(u_cam_pos, g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z);
    glUniform1f(u_shininess, g_shininess);
    glUniform1f(u_ambient, g_ambient);
    glUniform1f(u_transparency, g_transparency);

    // Pass textures to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureEarth);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureEarthNormal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureEarthSpec);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureEarthNight);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw Jupiter
// ------------------------------------------------------------------------------------------
void drawJupiter() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_JupiterShaderProgram);

    GLuint u_model = glGetUniformLocation(g_JupiterShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_JupiterShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_JupiterShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_JupiterShaderProgram, "u_texture");
    GLuint u_light_dir = glGetUniformLocation(g_JupiterShaderProgram, "u_light_dir");
    GLuint u_light_color = glGetUniformLocation(g_JupiterShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_JupiterShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_JupiterShaderProgram, "u_shininess");
    GLuint u_ambient = glGetUniformLocation(g_JupiterShaderProgram, "u_ambient");
    GLuint u_normal_matrix = glGetUniformLocation(g_JupiterShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_JupiterShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_jupiterPosition);
    model_matrix *= scale(mat4(1.0f), vec3(0.95f, 0.95f, 0.95f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, -0.00025f, 0.0f));

    // Normal Matrix
    mat3 normal_matrix = inverseTranspose(mat3(model_matrix));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Light Direction
    vec3 g_lightDirection = glm::vec3(g_sunPosition.x - g_jupiterPosition.x, g_sunPosition.y - g_jupiterPosition.y, g_sunPosition.z - g_jupiterPosition.z);

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform3f(u_light_dir, g_lightDirection.x, g_lightDirection.y, g_lightDirection.z);
    glUniform3f(u_light_color, g_lightColor.x, g_lightColor.y, g_lightColor.z);
    glUniform3f(u_cam_pos, g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z);
    glUniform1f(u_shininess, g_shininess);
    glUniform1f(u_ambient, g_ambient);
    glUniform1f(u_transparency, g_transparency);

    // Pass texture to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureJupiter);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw Mars
// ------------------------------------------------------------------------------------------
void drawMars() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_MarsShaderProgram);

    GLuint u_model = glGetUniformLocation(g_MarsShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_MarsShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_MarsShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_MarsShaderProgram, "u_texture");
    GLuint u_texture_normal = glGetUniformLocation(g_MarsShaderProgram, "u_texture_normal");
    GLuint u_light_dir = glGetUniformLocation(g_MarsShaderProgram, "u_light_dir");
    GLuint u_light_color = glGetUniformLocation(g_MarsShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_MarsShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_MarsShaderProgram, "u_shininess");
    GLuint u_ambient = glGetUniformLocation(g_MarsShaderProgram, "u_ambient");
    GLuint u_normal_matrix = glGetUniformLocation(g_MarsShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_MarsShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_marsPosition);
    model_matrix *= scale(mat4(1.0f), vec3(0.75f, 0.75f, 0.75f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, 0.00014f, 0.0f));

    // Normal Matrix
    mat3 normal_matrix = inverseTranspose(mat3(model_matrix));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Light Direction
    vec3 g_lightDirection = glm::vec3(g_sunPosition.x - g_marsPosition.x, g_sunPosition.y - g_marsPosition.y, g_sunPosition.z - g_marsPosition.z);

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform1i(u_texture_normal, 1);
    glUniform3f(u_light_dir, g_lightDirection.x, g_lightDirection.y, g_lightDirection.z);
    glUniform3f(u_light_color, g_lightColor.x, g_lightColor.y, g_lightColor.z);
    glUniform3f(u_cam_pos, g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z);
    glUniform1f(u_shininess, g_shininess);
    glUniform1f(u_ambient, g_ambient);
    glUniform1f(u_transparency, g_transparency);

    // Pass textures to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureMars);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureMarsNormal);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw Neptune
// ------------------------------------------------------------------------------------------
void drawNeptune() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_NeptuneShaderProgram);

    GLuint u_model = glGetUniformLocation(g_NeptuneShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_NeptuneShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_NeptuneShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_NeptuneShaderProgram, "u_texture");
    GLuint u_light_dir = glGetUniformLocation(g_NeptuneShaderProgram, "u_light_dir");
    GLuint u_light_color = glGetUniformLocation(g_NeptuneShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_NeptuneShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_NeptuneShaderProgram, "u_shininess");
    GLuint u_ambient = glGetUniformLocation(g_NeptuneShaderProgram, "u_ambient");
    GLuint u_normal_matrix = glGetUniformLocation(g_NeptuneShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_NeptuneShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_neptunePosition);
    model_matrix *= scale(mat4(1.0f), vec3(0.75f, 0.75f, 0.75f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, -0.000002f, 0.0f));

    // Normal Matrix
    mat3 normal_matrix = inverseTranspose(mat3(model_matrix));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Light Direction
    vec3 g_lightDirection = glm::vec3(g_sunPosition.x - g_neptunePosition.x, g_sunPosition.y - g_neptunePosition.y,
        g_sunPosition.z - g_neptunePosition.z);

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform3f(u_light_dir, g_lightDirection.x, g_lightDirection.y, g_lightDirection.z);
    glUniform3f(u_light_color, g_lightColor.x, g_lightColor.y, g_lightColor.z);
    glUniform3f(u_cam_pos, g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z);
    glUniform1f(u_shininess, g_shininess);
    glUniform1f(u_ambient, g_ambient);
    glUniform1f(u_transparency, g_transparency);

    // Pass texture to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureNeptune);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll draw Venus
// ------------------------------------------------------------------------------------------
void drawVenus() {
    // We'll just paint the front of the sphere.
    glCullFace(GL_BACK);

    // Select the shader we use to paint.
    glUseProgram(g_VenusShaderProgram);

    GLuint u_model = glGetUniformLocation(g_VenusShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_VenusShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_VenusShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_VenusShaderProgram, "u_texture");
    GLuint u_light_dir = glGetUniformLocation(g_VenusShaderProgram, "u_light_dir");
    GLuint u_light_dir_mars = glGetUniformLocation(g_VenusShaderProgram, "u_light_dir_mars");
    GLuint u_light_color = glGetUniformLocation(g_VenusShaderProgram, "u_light_color");
    GLuint u_light_color_mars = glGetUniformLocation(g_VenusShaderProgram, "u_light_color_mars");
    GLuint u_cam_pos = glGetUniformLocation(g_VenusShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_VenusShaderProgram, "u_shininess");
    GLuint u_shininess_mars = glGetUniformLocation(g_VenusShaderProgram, "u_shininess_mars");
    GLuint u_ambient = glGetUniformLocation(g_VenusShaderProgram, "u_ambient");
    GLuint u_ambient_mars = glGetUniformLocation(g_VenusShaderProgram, "u_ambient_mars");
    GLuint u_normal_matrix = glGetUniformLocation(g_VenusShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_VenusShaderProgram, "u_transparency");

    // Model Matrix
    mat4 model_matrix = translate(mat4(1.0f), g_venusPosition);
    model_matrix *= scale(mat4(1.0f), vec3(0.7f, 0.7f, 0.7f));
    model_matrix *= rotate(mat4(1.0f), rAngle, vec3(0.0f, 0.000012f, 0.0f));

    // Normal Matrix
    mat3 normal_matrix = inverseTranspose(mat3(model_matrix));

    // View Matrix
    glm::mat4 view_matrix = glm::lookAt(
        g_cameraPosition,
        g_cameraTarget,
        g_upVector
    );

    // Light Direction
    vec3 g_lightDirection = glm::vec3(g_sunPosition.x - g_venusPosition.x, g_sunPosition.y - g_venusPosition.y,
        g_sunPosition.z - g_venusPosition.z);
    vec3 g_lightDirectionMars = glm::vec3(g_marsPosition.x - g_venusPosition.x, g_marsPosition.y - g_venusPosition.y,
        g_marsPosition.z - g_venusPosition.z);

    // Pass attributes to Shader.
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform3f(u_light_dir, g_lightDirection.x, g_lightDirection.y, g_lightDirection.z);
    glUniform3f(u_light_dir_mars, g_lightDirectionMars.x, g_lightDirectionMars.y, g_lightDirectionMars.z);
    glUniform3f(u_light_color, g_lightColor.x, g_lightColor.y, g_lightColor.z);
    glUniform3f(u_light_color_mars, g_lightColorMars.x, g_lightColorMars.y, g_lightColorMars.z);
    glUniform3f(u_cam_pos, g_cameraPosition.x, g_cameraPosition.y, g_cameraPosition.z);
    glUniform1f(u_shininess, g_shininess);
    glUniform1f(u_shininess_mars, 30.0f);
    glUniform1f(u_ambient, g_ambient);
    glUniform1f(u_ambient_mars, 30.0f);
    glUniform1f(u_transparency, g_transparency);

    // Pass texture to Shader.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureVenus);

    // Draw to back buffer.
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    // Clear everything.
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

// ------------------------------------------------------------------------------------------
// Function which'll on display everything
// ------------------------------------------------------------------------------------------
void onDisplay() {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable culling and blending buffers. Disable depth buffer.
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // Configure blending buffer.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw universe.
    drawMilkyway();

    // Enable depth buffer.
    glEnable(GL_DEPTH_TEST);

    // Draw the different planets of the Solar System.
    drawSun();
    drawEarth();
    drawJupiter();
    drawMars();
    drawNeptune();
    drawVenus();

    // Show in command lines the current camera coordinates.
    //std::cout << "x: " << g_cameraPosition.x << " y: " << g_cameraPosition.y << " z: " << g_cameraPosition.z
    //    << std::endl;

    // Swap the buffers so back buffer is on screen.
    glutSwapBuffers();
}

// ------------------------------------------------------------------------------------------
// Function called when the program is idle
// ------------------------------------------------------------------------------------------
void animate() {
    // Check how to handle the Z axis increment/decrement.
    int zLocation = g_cameraPosition.z < 0 ? -1 : 1;
    
    if (rAngle == 360) {
        rAngle = 0.0f;
    } else {
        rAngle += 0.5f;
    }

    computeDistance();

    if (g_wKey == true) {
        g_cameraPosition.z -= (zLocation * 0.08f);
        computeDistance();
        sideCameraMovement();
    } else if (g_aKey == true) {
        g_angle += 0.01f;
        sideCameraMovement();
    } else if (g_sKey == true) {
        g_cameraPosition.z += (zLocation * 0.08f);
        computeDistance();
        sideCameraMovement();
    } else if (g_dKey == true) {
        g_angle -= 0.01f;
        sideCameraMovement();
    } else if (g_fKey == true) {
        g_cameraPosition = glm::vec3(5.1019f, 0.0f, -11.3549f);
    } else if (g_gKey == true) {
        g_cameraPosition = glm::vec3(0.51124f, 0.0f, -14.4792f);
    } else {
        g_angle += 0.0005;
        sideCameraMovement();
    }

    // Redisplay with the changes made.
    glutPostRedisplay();
}

// ------------------------------------------------------------------------------------------
// Main function of the program
// ------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    glutInit(&argc, argv);                                          // Initialize GLUT.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);      // Configure the screen and format precision.
    glutInitWindowSize(g_ViewportWidth, g_ViewportHeight);          // Configure the window size.
    glutCreateWindow("Solar System");                               // Create the window.

    // Clear the background colour to our global variable.
    glClearColor(g_ColorFons.x, g_ColorFons.y, g_ColorFons.z, 1.0f);

    // Load resources.
    loadResources();

    // Tell GLUT to draw display, and operate while the program is idle.
    glutDisplayFunc(onDisplay);
    glutIdleFunc(animate);

    // Tell GLUT about the keyboard, mouse and window resizing.
    glutReshapeFunc(onReshape);
    glutKeyboardUpFunc(onKeyUp);
    glutKeyboardFunc(onKeyDown);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);

    // Start everything up.
    glutMainLoop();

    return EXIT_SUCCESS;
}
