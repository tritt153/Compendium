/**
* Travis Ritter, CSC350
*
* This program uses basic OpenGL vertex logic and transformations to create 3D pixel art, by 
* creating colored cubes in a pattern to replicate a pixelized picture. 
*/
#include <iostream>
using namespace std;

#include <GL/glew.h> // Core OpenGL
#include <GL/glut.h> // Display Window
#include "Matrix.h"

// Not mine.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLuint buffer;
GLuint vPosition = 0;
GLuint vColor = 1;
GLuint programID;

const char* vertexShaderCode =
    "#version 330\r\n"
    "\n"
    "in vec3 position; \n" //takes in 3 values that represents 3D coordinates 
    "in vec3 vertexColor;\n" //takes in 3 values that represent the RGB values
    "out vec3 theColor;\n" //sends out the color values
    "uniform mat4 scale;" //scale matrix
    "uniform mat4 translateXY;" //first translation matrix that only translates X and Y
    "uniform mat4 rotate;" //rotation matrix that rotates each vertex
    "uniform mat4 fullMatrix;"//matrix that includes translations to Z and perspective proj.
    "void main()\n"
    "{\n"
    " 	 vec4 v = vec4(position, 1.0);"
    "	 vec4 newPos = scale * v;"
    "	 vec4 newPos2 = translateXY * newPos;"
    "	 vec4 newPos3 = rotate * newPos2;"
    "    gl_Position = fullMatrix * newPos3;\n"
    "	 theColor = vertexColor;\n"
    "\n"
    "}\n";

const char* fragShaderCode =
    "#version 330\r\n"
    "in vec3 theColor;\n" //takes in the colors from the vertex shader
    "out vec4 outColor;\n" //outputs the colors, with the alpha value
    "\n"
    "void main()\n"
    "{\n"
    "    outColor = vec4(theColor, 1.0);\n"
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
//This 2D array is used to map the different pixels on my picture
GLshort image[18][18] { 
   // 0 = white, 1 = black, 2 = orange, 3 = red, 4 = yellow, 5 = gray   
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //line 0
   {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, //line 1
   {0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 3, 1, 0, 0}, //line 2
   {0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 1, 3, 3, 1, 0}, //line 3
   {0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 3, 3, 3, 1, 0}, //line 4
   {0, 1, 2, 2, 2, 0, 1, 2, 2, 1, 0, 0, 1, 3, 2, 3, 1, 0}, //line 5
   {0, 1, 2, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 1, 4, 2, 1, 0}, //line 6
   {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 1, 2, 1, 0, 0}, //line 7
   {0, 0, 0, 1, 1, 1, 2, 2, 1, 2, 2, 1, 1, 2, 2, 1, 0, 0}, //line 8
   {0, 0, 0, 0, 1, 4, 2, 1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0}, //line 9
   {0, 0, 0, 0, 1, 4, 4, 4, 1, 2, 2, 1, 2, 1, 0, 0, 0, 0}, //line 10
   {0, 0, 0, 1, 0, 1, 1, 4, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0}, //line 11
   {0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0}, //line 12
   {0, 0, 0, 0, 0, 0, 0, 1, 5, 2, 5, 1, 0, 0, 0, 0, 0, 0}, //line 13
   {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}, //line 14
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //line 15
//c:0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
};

/**
* Simple method that uses 2D array coordinates to select the appropriate color. 
*/
void getColor(GLfloat *color, int row, int col) { 
	switch (image[row][col]) {
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
* Creates a cube based on the locational input. In terms of 
* a 2D array structure i.e. 0, 0 is the first location (top left).
* row: the row coordinate
* col: the column coordinate
* offset: the buffer to jump past the last set of coordinates
*         which is 216 because it takes 216 floats to make a single cube.
* vertices: the array that temporarily stores the 216 floats, then is used
*           to copy that info to the main array (allVertices)
*/
void addCubes(int row, int col, int offset, GLfloat *vertices) {

	GLfloat xLow = -1.0 + col;
	GLfloat xHigh = 0.0 + col;
	
	GLfloat yLow = 0.0 - row;
	GLfloat yHigh = 1.0 - row;
	
	GLfloat zClose = -1.0;
	GLfloat zFar = 1.0;
	
	GLfloat color[3]; 
	getColor(color, row, col);
	
	//faux lighting
	//color[0] -= .03 * (col + 1);
	//color[1] -= .03 * (col + 1);
	//color[2] -= .03 * (col + 1);
	
	GLfloat tempVertices[] = { 	
		 //left side
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2],
		 xLow,   yLow,   zFar,   color[0], color[1], color[2],
		 xLow,   yHigh,  zClose, color[0], color[1], color[2],
		 xLow,   yLow,   zFar,   color[0], color[1], color[2],
		 xLow,   yLow,   zClose, color[0], color[1], color[2],
		 
		  //right side
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xHigh,  yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xHigh,  yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2],	
		  
		 //top	 
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2], 
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xLow,   yHigh,  zClose, color[0], color[1], color[2],
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 
		  
		  //bottom
		 xLow,   yLow,   zFar,   color[0], color[1], color[2], 
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2],
		 xLow,   yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2],
		 xLow,   yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yLow,   zClose, color[0], color[1], color[2],
		  
		  //front
		 xLow,   yHigh,  zClose, color[0], color[1], color[2], 
		 xLow,   yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2],
		 xLow,   yLow,   zClose, color[0], color[1], color[2],
		 xHigh,  yHigh,  zClose, color[0], color[1], color[2], 
		 xHigh,  yLow,   zClose, color[0], color[1], color[2],
		  
		  //back
		 xLow,   yHigh,  zFar,   color[0], color[1], color[2], 
		 xLow,   yLow,   zFar,   color[0], color[1], color[2],
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xLow,   yLow,   zFar,   color[0], color[1], color[2],
		 xHigh,  yHigh,  zFar,   color[0], color[1], color[2],
		 xHigh,  yLow,   zFar,   color[0], color[1], color[2],			
	};

	for(int i = 0; i < 216; i++) {
		vertices[offset + i] = tempVertices[i];
	}
}

/**
* Method that initialzes all the OpenGL logic and buffers to store, link and create the
* program, also includes the array includes all the vertices and their respective colors
* (allVertices)
*/
void init(void) {
    cout << glGetString(GL_VERSION) << endl;
    
	GLfloat allVertices[69984]; //55296 for 256 cubes (216 * 256)
	
	int offset = 0; 
    for(int row = 0; row < 18; row++) { 
		for(int col = 0; col < 18; col++) {		
			addCubes(row, col, offset, allVertices);
			offset += 216;
		}
	} 
	
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(allVertices), allVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(vPosition);
    
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float))); 
    glEnableVertexAttribArray(vColor);

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
* This method is what calls OpenGL methods to enable depth testing, and clear the depth bit
* Also, It utilizes the Matrix helper class and uses Matrix math to perform transformations
* on each point to make 3D images with actual depth.
*/
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_DEPTH_TEST);
    
    GLfloat factor = 1.0/9.0;
    //Matrix scale = scale.createIdentity();
    Matrix scale = scale.createScaleMatrix(factor);
    scale.printMatrix();
    
    //Matrix translateXY = translateXY.createIdentity();
    Matrix translateXY = translateXY.createTranslationMatrix(-1 + factor, 1 - factor, 0);
    translateXY.printMatrix();
    
    //Matrix rotate = rotate.createIdentity();
    Matrix rotate = rotate.createYRotationMatrix(45);
    rotate.printMatrix();
    
    //Matrix translateZ = translateZ.createIdentity();
    Matrix translateZ = translateZ.createTranslationMatrix(0, 0, 10);
    translateZ.printMatrix();
    
    //Matrix project = project.createIdentity();
    Matrix project = project.createProjectionMatrix(45, 1, .80, 10);
    project.printMatrix();
    
    Matrix fullMatrix = project * translateZ;
    fullMatrix.printMatrix();
    
    
    GLint scaleUniformLocation = glGetUniformLocation(programID, "scale"); 
    GLint translateXYUniformLocation = glGetUniformLocation(programID, "translateXY"); 
   	GLint rotateUniformLocation = glGetUniformLocation(programID, "rotate");
    GLint fullMatrixUniformLocation = glGetUniformLocation(programID, "fullMatrix");  
    
    
    glUniformMatrix4fv(scaleUniformLocation, 1, GL_TRUE, &scale[0][0]); 
    glUniformMatrix4fv(translateXYUniformLocation, 1, GL_TRUE, &translateXY[0][0]);
    glUniformMatrix4fv(rotateUniformLocation, 1, GL_TRUE, &rotate[0][0]);
    glUniformMatrix4fv(fullMatrixUniformLocation, 1, GL_FALSE, &fullMatrix[0][0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 11664); //9126 is what it should be 36 * (16 * 16)
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
