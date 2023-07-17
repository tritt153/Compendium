/**
* Travis Ritter, CSC350
*
* This program uses basic OpenGL vertex logic and transformations to create 3D 
* shapes and uses basic lighting and vector math to simulate different types of
*light mainly ambient, diffuse, and specular  
*/
#include <iostream>
using namespace std;

#include <GL/glew.h> // Core OpenGL
#include <GL/glut.h> // Display Window
#include "Matrix.h"
#include "math.h"

#define DELTA .001
// Not mine.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define NUM_ELEMENTS(a) sizeof(a) / sizeof(*a)

GLuint shapeBuffer;
GLuint vPosition = 0;
GLuint vColor = 1;
GLuint vNormal = 2; 
GLuint programID;

const char* vertexShaderCode =
    "#version 330\r\n"
    "\n"
    "in vec3 position; \n" //takes in 3 values that represents 3D coordinates 
    "in vec3 vertexColor;\n" //takes in 3 values that represent the RGB values
    "in vec3 normal;"
    "uniform mat4 scale;" //scale matrix
    "uniform mat4 translateXY;" //first translation matrix that only translates X and Y
    "uniform mat4 rotate;" //rotation matrix that rotates each vertex
    "uniform mat4 fullMatrix;"//matrix that includes translations to Z and perspective proj.
    "uniform mat4 translateZ;"
    "uniform float reflectAmt;"
    
  	"out vec3 theColor;\n" //sends out the color values
  	"out vec4 normalPos;"
  	"out vec3 useNormal;"
    "void main()\n"
    "{\n"
    "	 vec4 newNormal = vec4(normal, 1.0);"
   	"	 vec4 rotatedNormal = rotate * newNormal;"//only need to rotate normal
    " 	 useNormal = normalize(vec3(rotatedNormal.x, rotatedNormal.y, 	  rotatedNormal.z));"   
    ""
    " 	 vec4 v = vec4(position, 1.0);"
    "	 vec4 newPos = translateXY*rotate*scale * v;"
    "	 normalPos = translateZ * newPos;"		
    
    "    gl_Position = fullMatrix * newPos;\n"
    "	 theColor = vertexColor;\n"
    "\n"
    "}\n";

const char* fragShaderCode =
    "#version 330\r\n"
    "in vec3 theColor;\n" //takes in the colors from the vertex shader
    "in vec4 normalPos;"
    "in vec3 useNormal;"
    "in vec3 FragPos;"
    "out vec4 outColor;\n" //outputs the colors, with the alpha value
    "uniform vec3 eyePosition;"
    ""
    "uniform vec3 dirLightPosition;"
    "uniform vec3 dirLightColor;"
    "uniform float dirIntensity;"
    ""
    "uniform vec3 pointLightColor;"
    "uniform vec3 pointLightPosition;"
    "uniform float constant;"
    "uniform float linear;"
    "uniform float quadratic;"
    "uniform float pointIntensity;"
    ""
    "uniform vec3 spotLightColor;"
    "uniform vec3 spotLightPosition;"
    "uniform vec3 spotLightDirection;"
    "uniform float innerCutOff;"
	"uniform float outerCutOff;"
	"uniform float spotIntensity;"
	""
    "uniform vec3 ambient;"
	""
    "uniform float reflectAmt;"
    "\n"
    "void main()\n"
    "{\n"
    //for directional light
    "	 vec3 dirLightDirection = normalize(dirLightPosition-FragPos);"
    "	 vec3 dirHalfVector = normalize(eyePosition + dirLightPosition);"
    "    vec3 dirDiffuse = vec3(max(0.0, dot(useNormal, dirLightDirection))* (1 - reflectAmt)); "
    "	 vec3 dirSpecular = vec3(min(1.0, dot(dirHalfVector, useNormal)) * (reflectAmt);"
    "	 vec3 dirResult = (theColor + (dirSpecular * (dirLightColor)) + (dirDiffuse * (dirLightColor))) * dirIntensity;"
    
    //for point light
    "	 vec3 pointLightDirection = normalize(pointLightPosition - FragPos);"
    "	 vec3 pointHalfVector = normalize(eyePosition + pointLightPosition);"	
    "	 vec3 pointDiffuse = vec3(max(0.0, dot(useNormal, pointLightPosition))* (1 - reflectAmt));"
    "	 vec3 pointSpecular = vec3(min(1.0, dot(pointHalfVector, useNormal)) * (reflectAmt));"
    ""//attenuation
    "	 float distance = length(pointLightPosition - vec3(normalPos.x, normalPos.y, normalPos.z));"
    "	 float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));"
    "	 if(attenuation < .22)"
    "	    attenuation = 0;"
    "	 pointDiffuse *= attenuation;"
    "	 pointSpecular *= attenuation;"
	""
	"    vec3 pointResult = vec3(0);"
	"	 if(attenuation >= .22) {"
	"		pointResult = (pointSpecular + (pointLightColor)) + (pointDiffuse + (pointLightColor)) * pointIntensity;"
	"	 }"
	""
	""
	//for spot light
	/* 
	"	 vec3 spotLightDir = normalize(spotLightPosition - FragPos);"
	"	 vec3 spotHalfVector = normalize(eyePosition+spotLightPosition);"
	"	 vec3 spotDiffuse = vec3(max(0.0, dot(useNormal, spotLightPosition))* (1 - reflectAmt));"
	""
    "	 vec3 spotSpecular = vec3(min(1.0, dot(spotHalfVector, useNormal)) * (reflectAmt));"
    ""
    "	 float theta = dot(spotLightDir, normalize(-spotLightDirection));"
    "	 float epsilon = (innerCutOff - outerCutOff);"
    "	 float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);"	
    ""
    "	 distance = length(spotLightPosition-FragPos);"
    "	 attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));"
    "	 spotDiffuse *= intensity * attenuation;"
    "	 spotSpecular *= intensity * attenuation;"
	"	 vec3 spotResult = ((spotSpecular * (spotLightColor)) + (spotDiffuse * (spotLightColor))) * spotIntensity;"
	""
	*/
	"	 vec3 test = dirResult + pointResult + ambient;"
    "    outColor = vec4(test, 1.0);\n"
    "}\n";

/**
* Method to make sure shaders are created and linked to the program properly
*/
void checkForShaderError(GLuint shader) {
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE)
    {
        cout << compileStatus << endl;
        GLint maxLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        cout << maxLength << endl;
        GLchar* errorLog = new GLchar[maxLength];
        GLsizei bufferSize;
        glGetShaderInfoLog(shader, maxLength, &bufferSize, errorLog);
        cout << errorLog << endl;
        delete [] errorLog;
    }
}

/**
* Method to make sure the OpenGL program is created properly
*/ 
void checkForProgramError(GLuint programID) {
    GLint linkStatus;
    glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLint maxLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
        GLchar* errorLog = new GLchar[maxLength];
        GLsizei bufferSize;
        glGetProgramInfoLog(programID, maxLength, &bufferSize, errorLog);
        cout << errorLog << endl;
        delete [] errorLog;
    }
}

/**
*A method that consolidates my shader initlization for the sake of clealiness
*and to make the code below more readable
*/
void setupShaders() { 
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER); // VSO
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER); // FSO 
    
    const char* adapter[1];
    adapter[0] = vertexShaderCode;
    glShaderSource(vertexShaderID, 1, adapter, 0);

    adapter[0] = fragShaderCode;
    glShaderSource(fragShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    checkForShaderError(vertexShaderID);
    
    glCompileShader(fragShaderID);
    checkForShaderError(fragShaderID);

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);
    
    checkForProgramError(programID);
	
    glUseProgram(programID);
}

/**
*Simple method that allows for different color selection based on an integer input. It 'returns' a color array that is of lenght 3
*/
void getColor(GLfloat *color, int num) { 
	switch (num) {
		case 0: //white
			color[0] = .95;
			color[1] = .95;
			color[2] = .95;
			break;
	 	case 1: //black
	 		color[0] = 0.0;
			color[1] = 0.0;
			color[2] = 0.0;
			break;
		case 2: //orange
			color[0] = 1.0;
			color[1] = 0.407;
			color[2] = 0.0;
			break;
		case 3: //red
			color[0] = 0.75;
			color[1] = 0.0;
			color[2] = 0.0;
			break;
		case 4: //yellow
			color[0] = 1.0;
			color[1] = .850;
			color[2] = 0.0;
			break;
		case 5: //gray
			color[0] = 0.502;
			color[1] = 0.502;
			color[2] = 0.502;
			break;	
		default: 
			break;
	}
}

/**
*Method that calculates the surface normals for a shape it takes in the vertices and returns a pointer to the normals. 
*/
GLfloat* computeNormals(GLfloat* verts, int numVerts)
{
   	GLfloat* normals = (GLfloat*)malloc(numVerts*sizeof(GLfloat));
    
    for(int i = 0; i < numVerts; i+=9) {
    	GLfloat p1[3] = { verts[i], verts[i + 1], verts[i + 2] };     //first pt
    	GLfloat p2[3] = { verts[i + 3], verts[i + 4], verts[i + 5] }; //second pt
    	GLfloat p3[3] = { verts[i + 6], verts[i + 7], verts[i + 8] }; //third pt
    	
    	GLfloat U[3] = {(p2[0] - p1[0]), (p2[1] - p1[1]), (p2[2] - p1[2])}; //p2 - p1
    	GLfloat V[3] = {(p3[0] - p1[0]), (p3[1] - p1[1]), (p3[2] - p1[2])}; //p3 - p1
    					     //x                y                z
    	GLfloat normal[3] = {(U[1] * V[2] - U[2] * V[1]), 
    						 (U[2] * V[0] - U[0] * V[2]),	  //cross product
    						 (U[0] * V[1] - U[1] * V[0]), };
    	for(int j = i; j < i + 9; j += 3) {
    		normals[j] = normal[0];
    		normals[j + 1] = normal[1];
    		normals[j + 2] = normal[2];
    	} 		
    }  
    return normals;
}
/**
*Another shape method, similar to the cube, but with different vertices to make 
*an octahedron, also I could not manually connect the noramls to each point,
*like I could the cube because the sides of an octahedron are not straight
*So I take the points call ComputeNoramls on them and get the color, and
*use a for loop to put all of that info into a single array, then pass that down
*to opengl
*/
void addOctahedron(GLfloat *vertices, GLint colorNum) { 
	
	GLfloat tempVertices[] = {
	   //top front x
		0,  1,  0,
	    1,  0, -1,
	   -1,  0, -1, 
	   //top back
	    0,  1,  0,  
	   -1,  0,  1,  
	    1,  0,  1,  
	   //top left
	    0,  1,  0, 
	   -1,  0, -1,	     
	   -1,  0,  1, 
	   //top right x
	   	0,  1,  0, 
	    1,  0,  1,
	    1,  0, -1, 
	   //bottom front x
	    0, -1,  0, 
	   -1,  0, -1,
	    1,  0, -1,  
	    
	   //bottom back
	    0, -1,  0, 
	    1,  0,  1, 	    
	   -1,  0,  1, 
	    
	   //bottom left
	    0, -1,  0,
	   -1,  0,  1, 
	   -1,  0, -1,  
	    
	   //bottom right
	    0, -1,  0,
	   	1,  0, -1,  
	    1,  0,  1,
    
	};
		
	int tempSize = NUM_ELEMENTS(tempVertices);
	
	GLfloat c[3];
	getColor(c, colorNum);

	GLfloat* normals = computeNormals(tempVertices, tempSize);
	
	GLfloat fullInfo[tempSize * 3];
	
	for(int i = 0; i < tempSize; i += 3) {
		fullInfo[i*3] = tempVertices[i];
		fullInfo[i*3+1] = tempVertices[i+1];
		fullInfo[i*3+2] = tempVertices[i+2];
		
		fullInfo[i*3+3] = c[0];
		fullInfo[i*3+4] = c[1];
		fullInfo[i*3+5] = c[2];
		
		fullInfo[i*3+6] = normals[i];
		fullInfo[i*3+7] = normals[i+1]; 
		fullInfo[i*3+8] = normals[i+2];
	}
	
	for(int i = 0; i < tempSize * 3; i++) {
		vertices[i] = fullInfo[i];
	}
}
/**
*One of the few shape methods that I have, this one is almost purely hard-coded
*since a square has all flat sides,the normals are very simple to calculate, and
*I just attach them directly when I declare the vertices array
*/
void addCube(GLfloat *vertices, GLint colorNum) {
	
	GLfloat xLow = -1.0;
	GLfloat xHigh = 1.0;
	
	GLfloat yLow = -1.0;
	GLfloat yHigh = 1.0;
	
	GLfloat zClose = -1.0;
	GLfloat zFar = 1.0;
	
	GLfloat xNormal = 1.0;
	GLfloat yNormal = 1.0;
	GLfloat zNormal = 1.0;
	
	GLfloat color[3];
	getColor(color, colorNum);
	
	GLfloat tempVertices[] = { 	
		 //left side
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], -xNormal, 0, 0,
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2], -xNormal, 0, 0,
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], -xNormal, 0, 0,
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], -xNormal, 0, 0,
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], -xNormal, 0, 0,
		 xLow,   yLow,   zClose, color[0], color[1], color[2], -xNormal, 0, 0,
		 
		  //right side
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], xNormal, 0, 0,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], xNormal, 0, 0,
		 xHigh,  yLow,   zClose, color[0], color[1], color[2], xNormal, 0, 0,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], xNormal, 0, 0,
		 xHigh,  yLow,   zClose, color[0], color[1], color[2], xNormal, 0, 0,
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2], xNormal, 0, 0,
		   
		 //top	 
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2], 0, yNormal, 0,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], 0, yNormal, 0,
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 0, yNormal, 0,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], 0, yNormal, 0,
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 0, yNormal, 0,
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 0, yNormal, 0,
		  
		  //bottom
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], 0, -yNormal, 0,
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2], 0, -yNormal, 0,
		 xLow,   yLow,   zClose, color[0], color[1], color[2], 0, -yNormal, 0,
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2], 0, -yNormal, 0,
		 xLow,   yLow,   zClose, color[0], color[1], color[2], 0, -yNormal, 0,
		 xHigh,  yLow,   zClose, color[0], color[1], color[2], 0, -yNormal, 0,
		  
		  //front
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		 xLow,   yLow,   zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		 xLow,   yLow,   zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		 xHigh,  yLow,   zClose, color[0], color[1], color[2], 0, 0, -zNormal,
		  
		  //back
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2], 0, 0, zNormal,
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], 0, 0, zNormal,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], 0, 0, zNormal,
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], 0, 0, zNormal,
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2], 0, 0, zNormal,
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2], 0, 0, zNormal,
	};	
	
	for(int i = 0; i < NUM_ELEMENTS(tempVertices); i++) {
		vertices[i] = tempVertices[i];
	}
	
}

/**
*Simple shape method that creates a triangle about the origin and manually adds color and
*the surface normal
*/
void addTriangle(GLfloat *vertices, GLint colorNum) {
	GLfloat color[3];
	getColor(color, colorNum);
	
	GLfloat tempVertices[] = {
		0, 1, 0, color[0], color[1], color[2], 0, 0, -1,
	    1, 0, 0, color[0], color[1], color[2], 0, 0, -1,
		-1, 0, 0, color[0], color[1], color[2], 0, 0, -1,
	};
	
	for(int i = 0; i < NUM_ELEMENTS(tempVertices); i++) {
		vertices[i] = tempVertices[i];
	}
}

/**
*Simple shape method that creates a square about the origin and manually adds color and
*the surface normal
*/
void addSquare(GLfloat *vertices, GLint colorNum) { 
	GLfloat color[3];
	getColor(color, colorNum);
	
	GLfloat tempVertices[] { 
		-1,  1, 0, color[0], color[1], color[2], 0, 0, -1,
		 1,  1, 0, color[0], color[1], color[2], 0, 0, -1,
		-1, -1, 0, color[0], color[1], color[2], 0, 0, -1,
		
		 1,  1, 0, color[0], color[1], color[2], 0, 0, -1,
		 1, -1, 0, color[0], color[1], color[2], 0, 0, -1,
	    -1, -1, 0, color[0], color[1], color[2], 0, 0, -1,
	};
	
	for(int i = 0; i < NUM_ELEMENTS(tempVertices); i++) {
		vertices[i] = tempVertices[i];
	}
}

/**
*Set the specularity for each shape, since things with more specularity have less diffuse
*lighting, I simply made the diffuse multiplier (1 - input)
*/
void setReflectivity(GLfloat input) { 
	GLfloat reflectAmt  = input;
	GLint reflect = glGetUniformLocation(programID, "reflectAmt");
	glUniform1f(reflect, reflectAmt);
}

/**
*Simple method that sends a certain color ambient light to the shader pipeline
*/
void createAmbientLight(float r, float g, float b) {
	GLfloat ambientColor[3] = {r, g, b};
	GLint ambient = glGetUniformLocation(programID, "ambient");
	glUniform3fv(ambient, 1, &ambientColor[0]);
}

/**
*Method that takes inputs and sends uniform values representing a direciton light that we can
*use to perform math on to simulate directional lighting
*/
void createDirectionalLight(float x, float y, float z, float r, float g, float b, float dirIntensity) {
	GLfloat dirLightColor[3] = {r, g, b};
	GLint direct = glGetUniformLocation(programID, "dirLightColor");
	glUniform3fv(direct, 1, &dirLightColor[0]);
	
	GLfloat dirLightPosition[3] = {x, y, z};
	GLint dirLight = glGetUniformLocation(programID, "dirLightPosition");
	glUniform3fv(dirLight, 1, &dirLightPosition[0]);
	
	GLfloat lightAmt  = dirIntensity;
	GLint intensity = glGetUniformLocation(programID, "dirIntensity");
	glUniform1f(intensity, lightAmt);
}

/**
*Method that takes inputs and sends uniform values representing a point light that we can
*use to perform math on to simulate point lighting
*/
void createPointLight(float x, float y, float z, float r, float g, float b, float c, float l, float q, float pointIntensity) {
	GLfloat pointLightColor[3] = {r, g, b};
	GLint point = glGetUniformLocation(programID, "pointLightColor");
	glUniform3fv(point, 1, &pointLightColor[0]);
	
	GLfloat pointLightPosition[3] = {x, y, z};
	GLint pointLight = glGetUniformLocation(programID, "pointLightPosition");
	glUniform3fv(pointLight, 1, &pointLightPosition[0]);
	
	GLfloat pointLightAmt  = pointIntensity;
	GLint intensity = glGetUniformLocation(programID, "pointIntensity");
	glUniform1f(intensity, pointLightAmt);
	
	GLfloat constant = c;
	GLint constantLoc = glGetUniformLocation(programID, "constant");
	glUniform1f(constantLoc, constant);
	
	GLfloat linear = l;
	GLint linearLoc = glGetUniformLocation(programID, "linear");
	glUniform1f(linearLoc, linear);
	
	GLfloat quadratic = q;
	GLint quadraticLoc = glGetUniformLocation(programID, "quadratic");
	glUniform1f(quadraticLoc, quadratic);
}

/**
*Non-Functional
*/
void createSpotLight(float x, float y, float z, float r, float g, float b, float inner, float outer, float spotIntensity) {
	GLfloat spotLightColor[3] = {r, g, b};
	GLint spot = glGetUniformLocation(programID, "spotLightColor");
	glUniform3fv(spot, 1, &spotLightColor[0]);
	
	GLfloat pointLightPosition[3] = {x, y, z};
	GLint pointLight = glGetUniformLocation(programID, "spotLightPosition");
	glUniform3fv(pointLight, 1, &pointLightPosition[0]);
	
	GLfloat spotLightAmt  = spotIntensity;
	GLint intensity = glGetUniformLocation(programID, "spotIntensity");
	glUniform1f(intensity, spotLightAmt);
	
	GLfloat innerCutOff = inner;
	GLint innerCutOffLoc = glGetUniformLocation(programID, "innerCutOff");
	glUniform1f(innerCutOffLoc, innerCutOff);
	
	GLfloat outerCutOff = outer;
	GLint outerCutOffLoc = glGetUniformLocation(programID, "outerCutOff");
	glUniform1f(outerCutOffLoc, outerCutOff);
}

/**
*Method that defines where out POV is, sent as a uniform value to OpenGL
*/
void setEyePosition(float x, float y, float z) { 
	GLfloat eyePosition[3] = {x, y, z};
	GLint eye = glGetUniformLocation(programID, "eye");
	glUniform3fv(eye, 1, &eyePosition[0]);
}
void init(void) {
    cout << glGetString(GL_VERSION) << endl;
    setupShaders();
}
/**
*Mehtod that uses my custom Matrix class to send the transformation information
*to OpenGL this info is then utilized to uniformly change all vertices of the 
*shape to transform it in such a way that lets the light illuminate it well.
*
*/
void transform(float scaleFac, float x, float y, float z, int axis, float deg) { 
    Matrix scale = scale.createScaleMatrix(scaleFac);
    Matrix translateXY = translateXY.createTranslationMatrix(x, y, 0);    
    Matrix translateZ = translateZ.createTranslationMatrix(0, 0, z); 
    Matrix project = project.createProjectionMatrix(45, 1, -1, 10);
    Matrix rotate = rotate.createIdentity(); 
    switch(axis) { 
    	case 1:
    		rotate = rotate.createXRotationMatrix(deg);
    		break;
    	case 2: 
    		rotate = rotate.createYRotationMatrix(deg);
    		break;
    	case 3: 
    		rotate = rotate.createZRotationMatrix(deg);
    		break;
    	default:
    		cout << "1 = x, 2 = y, 3 = z";
    }
    
    Matrix fullMatrix = project * translateZ;
    
    Matrix normalMatrix = translateXY * rotate * scale; 
    
    GLint scaleUniformLocation = glGetUniformLocation(programID, "scale"); 
    GLint translateXYUniformLocation = glGetUniformLocation(programID, "translateXY"); 
   	GLint rotateUniformLocation = glGetUniformLocation(programID, "rotate");
    GLint fullMatrixUniformLocation = glGetUniformLocation(programID, "fullMatrix");    
    GLint translateZUniformLocation = glGetUniformLocation(programID, "translateZ"); 
    
    glUniformMatrix4fv(scaleUniformLocation, 1, GL_TRUE, &scale[0][0]); 
    glUniformMatrix4fv(translateXYUniformLocation, 1, GL_TRUE, &translateXY[0][0]);
    glUniformMatrix4fv(rotateUniformLocation, 1, GL_TRUE, &rotate[0][0]);
    glUniformMatrix4fv(fullMatrixUniformLocation, 1, GL_FALSE, &fullMatrix[0][0]);
    glUniformMatrix4fv(translateZUniformLocation, 1, GL_TRUE, &translateZ[0][0]);
}

void sendShapeData(int shapeNum, int color, int size) {
	GLfloat vertices[size];
	
	switch(shapeNum) {
		case 1:
		 	addCube(vertices, color);
		 	break;
	 	case 2:
		 	addOctahedron(vertices, color);
		 	break;
		case 3:
			addTriangle(vertices, color);
			break;
		case 4:
			addSquare(vertices, color);
			break;
		default:
			cout << "Shape does not exist";
	}
	 
	glGenBuffers(1, &shapeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, shapeBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float))); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float))); 
    glEnableVertexAttribArray(vNormal);
}


/**
*OpenGL looks here for what to draw. I create the shapes using a method, *transform the data using a method and send it to OpenGL (GPU) using another
*method I then tell OpenGL to draw the shapes and a flush the pipeline.
*
*/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_DEPTH_TEST);
    
    setEyePosition(0, 0, -1);
    
    createAmbientLight(.2, .2, .2); //white ambient light
    
    createDirectionalLight(-1, 0, 0,   //position
    					    1, .40, 0, //color (orange)
    					    .25);      //intensity
    
    createPointLight(5, 6, -2, 	    //position 
    				.5, 0, .5,	    //color (purple)
    				 1, 0.1, 0.035, //attenuation factors
    				 .35); 	  	    //intensity
    				 
    /*createSpotLight(.5, -.5, -1,  //position
    				 0, .5, .5, 	//color (dark green)
    				 5.5, 7.5, 	    //inner and outer cone sizes
    				 0.25);*/	    //intensity
 
    //cube 1 red
   	sendShapeData(1, 3, 324);
    transform(1.0/9.0, .1, .25, 2, 2, 45); 
    setReflectivity(.25); 
    glDrawArrays(GL_TRIANGLES, 0, 36);  
    glFlush(); 
    
    //cube 2 orange
   	sendShapeData(1, 2, 324);
    transform(1.0/9.0, -.5, -.3, 2, 2, 45);  
    setReflectivity(1); 
    glDrawArrays(GL_TRIANGLES, 0, 36); 
    glFlush(); 
    
    //triangle 1 yellow
    sendShapeData(3, 4, 27);
    transform(1.0/9.0, 0, -.7, 2, 1, 0);  
    setReflectivity(.6); 
    glDrawArrays(GL_TRIANGLES, 0, 3); 
    glFlush();
    
    //triangle 2 red
    sendShapeData(3, 3, 27);
    transform(1.0/7.0, -.5, .3, 2, 2, 65);  
    setReflectivity(.98); 
    glDrawArrays(GL_TRIANGLES, 0, 3); 
    glFlush();
    
     //octahedron 1 black
    sendShapeData(2, 1, 216);
    transform(1.0/8.0, .5, .5, 2, 2, 32);  
    setReflectivity(.2); 
    glDrawArrays(GL_TRIANGLES, 0, 48); 
    glFlush();
 
 	 //octahedron 2 yellow
    sendShapeData(2, 4, 216);
    transform(1.0/8.0, .5, -.65, 2, 1, 52);  
    setReflectivity(.8); 
    glDrawArrays(GL_TRIANGLES, 0, 48); 
    glFlush();
    
    //square 1 red
    sendShapeData(4, 3, 54);
    transform(1.0/8.0, .25, -.65, 5, 1, 0);  
    setReflectivity(.345); 
    glDrawArrays(GL_TRIANGLES, 0, 6); 
    glFlush();
    
    //square 2 orange
    sendShapeData(4, 2, 54);
    transform(1.0/8.0, -1, -1.5, 5, 2, 36);  
    setReflectivity(.4); 
    glDrawArrays(GL_TRIANGLES, 0, 6); 
    glFlush();
}

/**
* Initialzes necessary methods for OpenGL to define the display mode and window size.
*/
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow(argv[0]);

    if (glewInit()) {
	    cerr << "Unable to initialize GLEW ... exiting" << endl;
	    exit(EXIT_FAILURE);
    }

    init();
    glutDisplayFunc(display);
    glutMainLoop();
}
