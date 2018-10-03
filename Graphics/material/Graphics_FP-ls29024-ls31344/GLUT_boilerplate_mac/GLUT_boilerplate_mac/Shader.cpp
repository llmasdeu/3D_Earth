//
//  Shader.cpp
//
//  Created by Alun on 10/06/14.
//  Copyright (c) 2014 Alun. All rights reserved.
//

#include "Shader.h"
#include <vector>
#include <sstream>


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

char* Shader::readFile(const char* filename)
{
    FILE* fp=fopen(filename,"r");
    fseek(fp,0,SEEK_END);
    long file_length=ftell(fp);
    fseek(fp,0,SEEK_SET);
    char* contents=new char[file_length+1];
    for(int i=0;i<file_length+1;i++)
    {
        contents[i]=0;
    }
    fread(contents,1,file_length,fp);
    contents[file_length+1]='\0';
    fclose(fp);
    return contents;
}

Shader::Shader(const char* vertSource, const char* fragSource) {
    
    char* vertexShaderSourceCode=readFile(vertSource);
    char* fragmentShaderSourceCode=readFile(fragSource);
    makeShaderProgram(makeVertexShader(vertexShaderSourceCode), makeFragmentShader(fragmentShaderSourceCode));
}

GLuint Shader::makeVertexShader(const char* shaderSource)
{
    GLuint vertexShaderID=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID,1,(const GLchar**)&shaderSource, NULL);
    glCompileShader(vertexShaderID);
    
    GLint compile=0;
    glGetShaderiv(vertexShaderID,GL_COMPILE_STATUS,&compile);
    
    //we want to see the compile log if we are in debug (to check warnings)
    if (!compile)
    {
        saveShaderInfoLog(vertexShaderID);
        std::cout << "Shader code:\n " << std::endl;
        std::string code = shaderSource;
        std::vector<std::string> lines = split( code, '\n' );
        for( size_t i = 0; i < lines.size(); ++i)
            std::cout << i << "  " << lines[i] << std::endl;
    
    }
    
    
    return vertexShaderID;
}
GLuint Shader::makeFragmentShader(const char* shaderSource)
{
    GLuint fragmentShaderID=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID,1,(const GLchar**)&shaderSource, NULL);
    glCompileShader(fragmentShaderID);
    
    GLint compile=0;
    glGetShaderiv(fragmentShaderID,GL_COMPILE_STATUS,&compile);
    
    //we want to see the compile log if we are in debug (to check warnings)
    if (!compile)
    {
        saveShaderInfoLog(fragmentShaderID);
        std::cout << "Shader code:\n " << std::endl;
        std::string code = shaderSource;
        std::vector<std::string> lines = split( code, '\n' );
        for( size_t i = 0; i < lines.size(); ++i)
            std::cout << i << "  " << lines[i] << std::endl;
        
    }
    
    return fragmentShaderID;
}

void Shader::saveShaderInfoLog(GLuint obj)
{
    int len = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    
    if (len > 0)
    {
        char* ptr = new char[len+1];
        GLsizei written=0;
        glGetShaderInfoLog(obj, len, &written, ptr);
        ptr[written-1]='\0';
        log.append(ptr);
        delete[] ptr;
        
        printf("LOG **********************************************\n%s\n",log.c_str());
    }
}

void Shader::saveProgramInfoLog(GLuint obj)
{
    int len = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    
    if (len > 0)
    {
        char* ptr = new char[len+1];
        GLsizei written=0;
        glGetProgramInfoLog(obj, len, &written, ptr);
        ptr[written-1]='\0';
        log.append(ptr);
        delete[] ptr;
        
        printf("LOG **********************************************\n%s\n",log.c_str());
    }
}

void Shader::makeShaderProgram(GLuint vertexShaderID,GLuint fragmentShaderID)
{
    program=glCreateProgram();
    glAttachShader(program, vertexShaderID);
    glAttachShader(program,fragmentShaderID);
    
    glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        saveProgramInfoLog(program);
    
    }
}

GLint Shader::bindAttribute(const char* attribute_name) {
    GLint attribute_ID = glGetAttribLocation(program, attribute_name);
    if (attribute_ID == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }
    else return attribute_ID;
}

GLint Shader::bindUniform(const char* uniform_name) {
    GLint uniform_ID = glGetUniformLocation(program, uniform_name);
    if (uniform_ID == -1) {
        fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
        return 0;
    }
    else return uniform_ID;
}
