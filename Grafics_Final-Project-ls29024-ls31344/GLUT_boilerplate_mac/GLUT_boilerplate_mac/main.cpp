/*
    GRAPHICS PROGRAMMING
    Final Project

    Author: Lluís Masdeu (ls31344)
    Author: Oriol Ramis (ls29024)
*/

// Standard Libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// OpenGL GLUT & GLM Libraries
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
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
GLuint g_UniverseShaderProgram = 0, g_EarthShaderProgram = 0, g_SunShaderProgram = 0;

// VAOs (Vertex Array Object) vector where we store the objects' geometry we'll use
std::vector<GLuint> vaos;

// Variables where we'll store the different textures
GLuint textureMilkyway, textureEarth, textureEarthNormal, textureEarthSpec, textureSun;

// Vector which will contain each object's number of triangles
std::vector<GLuint> numTriangles;

float g_angle = 0.0f;
float x_movement = 0.0f;
float y_movement = 0.0f;
float g_light_angle = 30;
float g_light_distance = 50;

float x_planet, y_planet, z_planet;

vec3 g_cam_pos = glm::vec3(0, 0, 5);
vec3 g_light_dir = glm::vec3(1.0 - x_planet, 1.0 - y_planet, 1.0 - z_planet);

// ------------------------------------------------------------------------------------------
// Function in which we'll open and store the different resources we'll use through the
// program.
// ------------------------------------------------------------------------------------------
void createGeometry()
{
    // Open the object and declare the varibales which will store its parameters.
    std::string basepath = "/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/";
    std::string inputfile = basepath + "sphere.obj";
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    // Save the different textures.
    Image * milkywayImage = loadBMP("/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/textures/milkywaymap.bmp");
    Image * sunImage = loadBMP("/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/textures/sunmap.bmp");
    Image * earthImage = loadBMP("/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/textures/earthmap.bmp");
    Image * earthNormal = loadBMP("/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/textures/earthnormal.bmp");
    Image * earthSpec = loadBMP("/Users/lluismasdeu/Desktop/Grafics/Projecte_Final/GLUT_boilerplate_mac/GLUT_boilerplate_mac/textures/earthspecular.bmp");

    bool ret;
    ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str(), basepath.c_str());

    // Check there is no error.
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    // Mostramos por pantalla el numero de objetos a pintar (como medida de control).
    std::cout << "# of shapes : " << shapes.size() << std::endl;

    // Variable reusable que usaremos para crear los buffers.
    GLuint buffer;

    // Llevaremos a cabo el mismo procedimiento para todos los objetos.
    for (int i = 0; i < shapes.size(); i++) {
        // Anadimos una posicion al vector dinamico de VAOs, y reasignamos a 0 el valor del buffer.
        vaos.push_back(0);
        buffer = 0;

        // Creamos el VAO donde almacenaremos toda la geometria del objeto.
        glGenVertexArraysAPPLE(1, &vaos[i]);
        glBindVertexArrayAPPLE(vaos[i]);

        // Creamos el VBO para los vertices.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * 3 * sizeof(float),
                     &(shapes[i].mesh.positions[0]), GL_STATIC_DRAW);

        // Activamos el atributo de los vertices en el Shader.
        GLuint vertexLoc = glGetAttribLocation(g_EarthShaderProgram, "a_vertexPos");
        glEnableVertexAttribArray(vertexLoc);
        glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Reasignamos a 0 el valor del buffer.
        buffer = 0;

        // Creamos el VBO para las texturas.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size() * 2 * sizeof(float),
                     &(shapes[i].mesh.texcoords[i]), GL_STATIC_DRAW);

        // Activamos el atributo de los UVS en el Shader.
        GLuint uvLoc = glGetAttribLocation(g_EarthShaderProgram, "a_uvs");
        glEnableVertexAttribArray(uvLoc);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Reasignamos a 0 el valor del buffer.
        buffer = 0;

        // Creamos el VBO para las normales.
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() * 3 * sizeof(float),
                     &(shapes[i].mesh.normals[i]), GL_STATIC_DRAW);

        // Activamos el atributo de las normales en el Shader.
        GLuint normals = glGetAttribLocation(g_EarthShaderProgram, "a_normal");
        glEnableVertexAttribArray(normals);
        glVertexAttribPointer(normals, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Creamos el VBO para los indices.
        glGenBuffers(1, &buffer); //create more new buffers
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     shapes[i].mesh.indices.size() * sizeof(unsigned int), &(shapes[i].mesh.indices[0]),
                     GL_STATIC_DRAW);
        
        // Almacenamos el numero de triangulos del objeto.
        numTriangles.push_back(shapes[i].mesh.indices.size() / 3);

        glBindVertexArrayAPPLE(0); //unbind VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers
    }

    // ANNOTATION: glTexImage2D Schema
    // glTexImage2D(
    //      target,
    //      level, (= 0, no mipmap)
    //      the way we opengl stores the data,
    //      width,
    //      height,
    //      border, (must be 0)
    //      the format of the original data,
    //      type of data,
    //      pointer to start of data
    // );

    // Universe
    glGenTextures(1, &textureMilkyway);
    glBindTexture(GL_TEXTURE_2D, textureMilkyway);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 milkywayImage->width,
                 milkywayImage->height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 milkywayImage->pixels);

    // Sun
    glGenTextures(1, &textureSun);
    glBindTexture(GL_TEXTURE_2D, textureSun);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 sunImage->width,
                 sunImage->height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 sunImage->pixels);

    // Earth
    glGenTextures(1, &textureEarth);
    glBindTexture(GL_TEXTURE_2D, textureEarth);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 earthImage->width,
                 earthImage->height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 earthImage->pixels);

    // Earth - Normals
    glGenTextures(1, &textureEarthNormal);
    glBindTexture(GL_TEXTURE_2D, textureEarthNormal);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 earthNormal->width,
                 earthNormal->height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 earthNormal->pixels);

    // Earth - Specular
    glGenTextures(1, &textureEarthSpec);
    glBindTexture(GL_TEXTURE_2D, textureEarthSpec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 earthSpec->width,
                 earthSpec->height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 earthSpec->pixels);
}

// ------------------------------------------------------------------------------------------
// Load all resources (shaders and geometry in this case)
// ------------------------------------------------------------------------------------------
void loadResources()
{
    // Carrega els shaders i crea el programa de shaders
    Shader earthShader("earthShader.vsh", "earthShader.fsh");
    g_EarthShaderProgram = earthShader.program;
    
    Shader universeShader("universeShader.vsh", "universeShader.fsh");
    g_UniverseShaderProgram = universeShader.program;

    Shader sunShader("sunShader.vsh", "sunShader.fsh");
    g_SunShaderProgram = sunShader.program;
    
    // Crea la geomtria i desa-la al VAO
    createGeometry();
}

// --------------------------------------------------------------
// GLUT callback for keyboard
// if we want to capture key information, we do so here
// --------------------------------------------------------------
void onKeyUp(unsigned char key, int x, int y)
{
    //'key' is a value which represents a key on the keyboard.
    //if we want to find the value we could write
    //cout << key << endl;
    
    if (key == 27 )		// 27 is the code for ESC key
    {
        // exit the programme
        exit(EXIT_SUCCESS);
    }
    
}

void onKeyDown(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'w':
            g_cam_pos.z -= 0.1f;
            break;

        case 'a':
            g_cam_pos.x -= 0.1f;
            break;

        case 's':
            g_cam_pos.z += 0.1f;
            break;

        case 'd':
            g_cam_pos.x += 0.1f;
            break;
    }

    //Redisplay with the changes made.
    glutPostRedisplay();
}

// --------------------------------------------------------------
// GLUT callback detects window resize
// --------------------------------------------------------------
void onReshape(int w, int h)
{
    g_ViewportWidth = w;
    g_ViewportHeight = h;
    
    // Modify the gl viewport so that it covers all the window
    glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);
}

void drawMilkyway()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Select the shader we use to paint
    glUseProgram(g_UniverseShaderProgram);
    
    GLuint u_model = glGetUniformLocation(g_UniverseShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_UniverseShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_UniverseShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_UniverseShaderProgram, "u_texture");
    
    //model
    mat4 translate_matrix = translate(mat4(1.0f), vec3(1,1,1));
    mat4 model_matrix = translate_matrix;
    
    glm::mat4 view_matrix = glm::lookAt(
        glm::vec3(1,1,1),   // cameraPosition
        glm::vec3(0,0,0),   // cameraTarget
        glm::vec3(0,1,0)    // upVector
    );
    
    //create projection matrix and pass to shader
    glm::mat4 projection_matrix = glm::perspective(
        60.0f,          // Field of view
        3.0f / 3.0f,    // Aspect ratio
        0.1f,           // near plane (distance from camera)
        50.0f           // far plane (distance from camera)
    );
    
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    glUniform1i(u_texture, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureMilkyway);

    // Draw to back buffer
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);

    //clear everything
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}


void drawEarth()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Select the shader we use to paint
    glUseProgram(g_EarthShaderProgram);

    GLuint u_model = glGetUniformLocation(g_EarthShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_EarthShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_EarthShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_EarthShaderProgram, "u_texture");
    GLuint u_texture_normal = glGetUniformLocation(g_EarthShaderProgram, "u_texture_normal");        // new
    GLuint u_texture_spec = glGetUniformLocation(g_EarthShaderProgram, "u_texture_spec");            // new
    GLuint u_light_dir = glGetUniformLocation(g_EarthShaderProgram, "u_light_dir");
    GLuint u_light_color = glGetUniformLocation(g_EarthShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_EarthShaderProgram, "u_cam_pos");
    GLuint u_shininess = glGetUniformLocation(g_EarthShaderProgram, "u_shininess");
    GLuint u_ambient = glGetUniformLocation(g_EarthShaderProgram, "u_ambient");
    GLuint u_normal_matrix = glGetUniformLocation(g_EarthShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_EarthShaderProgram, "u_transparency");

    vec3 planet_pos = glm::vec3(1.0f, 1.0f, -5.0f);
    x_planet = planet_pos.x;
    y_planet = planet_pos.y;
    z_planet = planet_pos.z;
    
    //model
    //mat4 identity_matrix = mat4();
    //mat4 translate_matrix = translate(mat4(1.0f), vec3(1.0f, 1.0f, -5.0f));
    mat4 translate_matrix = translate(mat4(1.0f), planet_pos);

    mat3 normal_matrix = inverseTranspose(mat3(translate_matrix));

    //view
    glm::mat4 view_matrix = glm::lookAt(
        g_cam_pos,              // cameraPosition
        glm::vec3(0,0,0),       // cameraTarget
        glm::vec3(0,1,0)        // upVector
    );
    
    //create projection matrix and pass to shader
    glm::mat4 projection_matrix = glm::perspective(
        60.0f,          // Field of view
        3.0f / 3.0f,    // Aspect ratio
        0.1f,           // near plane (distance from camera)
        50.0f           // far plane (distance from camera)
    );

    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(translate_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    glUniform1i(u_texture_normal, 1);                                                           // new
    glUniform1i(u_texture_spec, 2);                                                             // new
    //glUniform3f(u_light_dir, 100, 100, 100);
    glUniform3f(u_light_dir, g_light_dir.x, g_light_dir.y, g_light_dir.z);
    glUniform3f(u_light_color, 1.0, 1.0, 1.0);
    glUniform3f(u_cam_pos, g_cam_pos.x, g_cam_pos.y, g_cam_pos.z);
    glUniform1f(u_shininess, 75.0);
    glUniform1f(u_ambient, 0.1);
    glUniform1f(u_transparency, 0.85);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureEarth);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureEarthNormal);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureEarthSpec);

    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);
    
    //clear everything
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

void drawSun()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Select the shader we use to paint
    glUseProgram(g_SunShaderProgram);
    
    GLuint u_model = glGetUniformLocation(g_SunShaderProgram, "u_model");
    GLuint u_view = glGetUniformLocation(g_SunShaderProgram, "u_view");
    GLuint u_projection = glGetUniformLocation(g_SunShaderProgram, "u_projection");
    GLuint u_texture = glGetUniformLocation(g_SunShaderProgram, "u_texture");
    //GLuint u_light_dir = glGetUniformLocation(g_SunShaderProgram, "u_light_dir");
    //GLuint u_light_color = glGetUniformLocation(g_SunShaderProgram, "u_light_color");
    GLuint u_cam_pos = glGetUniformLocation(g_SunShaderProgram, "u_cam_pos");
    //GLuint u_shininess = glGetUniformLocation(g_SunShaderProgram, "u_shininess");
    //GLuint u_ambient = glGetUniformLocation(g_SunShaderProgram, "u_ambient");
    //GLuint u_normal_matrix = glGetUniformLocation(g_SunShaderProgram, "u_normal_matrix");
    GLuint u_transparency = glGetUniformLocation(g_SunShaderProgram, "u_transparency");
    
    //model
    mat4 identity_matrix = mat4();
    //mat4 translate_matrix = translate(mat4(1.0f), vec3(4.5f, 2.0f, -5.0f));
    
    //mat3 normal_matrix = inverseTranspose(mat3(identity_matrix));
    
    //view
    glm::mat4 view_matrix = glm::lookAt(
                                        g_cam_pos,              // cameraPosition
                                        glm::vec3(0,0,0),       // cameraTarget
                                        glm::vec3(0,1,0)        // upVector
                                        );
    
    //create projection matrix and pass to shader
    glm::mat4 projection_matrix = glm::perspective(
                                                   60.0f,          // Field of view
                                                   3.0f / 3.0f,    // Aspect ratio
                                                   0.1f,           // near plane (distance from camera)
                                                   50.0f           // far plane (distance from camera)
                                                   );
    
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(identity_matrix));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
    //glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
    glUniform1i(u_texture, 0);
    //glUniform3f(u_light_dir, 100, 100, 100);
    //glUniform3f(u_light_color, 1.0, 1.0, 1.0);
    glUniform3f(u_cam_pos, g_cam_pos.x, g_cam_pos.y, g_cam_pos.z);
    //glUniform1f(u_shininess, 30.0);
    //glUniform1f(u_ambient, 0.1);
    glUniform1f(u_transparency, 1.0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureSun);
    
    glBindVertexArrayAPPLE(vaos[0]);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles[0], GL_UNSIGNED_INT, 0);
    
    //clear everything
    glBindVertexArrayAPPLE(0);
    glUseProgram(0);
}

void onDisplay()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawMilkyway();
    drawSun();
    drawEarth();

    // Swap the buffers so back buffer is on screen
    glutSwapBuffers();
}

//new function, called when program is idle
void animate() {
    g_angle -= 0.01f;
    glutPostRedisplay(); //tells window to redraw
}

int main(int argc, char* argv[]) {
    
    //initalise GLUT - GLUT is a windowing toolkit designed to help us make
    //OpenGL apps on MS Windows
    glutInit(&argc, argv);
    //screen format and precision
    glutInitDisplayMode( GLUT_RGBA  | GLUT_DOUBLE | GLUT_DEPTH );
    //window size
    glutInitWindowSize(g_ViewportWidth, g_ViewportHeight);
    //create the window
    glutCreateWindow("Solar System");

    // Clear the background colour to our global variable
    glClearColor(g_ColorFons.x, g_ColorFons.y, g_ColorFons.z, 1.0f );
    
    // Load resources
    loadResources();
    
    //this line is very important, as it tell GLUT what function to execute
    //every frame (which should be our draw function)
    glutDisplayFunc( onDisplay );
    glutIdleFunc( animate );

    //tell GLUT about the keyboard and window resizing
    glutKeyboardUpFunc( onKeyUp );
    glutKeyboardFunc( onKeyDown );  // this line is new
    glutReshapeFunc( onReshape );
    
    //start everything up
    glutMainLoop();
    
    return EXIT_SUCCESS;
}
