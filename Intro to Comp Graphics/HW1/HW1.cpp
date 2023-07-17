/**
* Travis Ritter, CSC350
*
* This program uses basic OpenGL vertex logic to create pixel art, by creating
* colored squares in a pattern to replicate a pixelized picture. 
*/
#include <iostream>
using namespace std;

#include <GL/glew.h> // Core OpenGL
#include <GL/glut.h> // Display Window

// Not mine.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLuint buffer;
GLuint vPosition = 0; 
GLuint vColor = 1;

const GLfloat squareSize = 1.0 / 8.0; //Since the OpenGL
									  //window is 2 units wide, 1/8 makes
									  //it possible to fit a 16x16 image
const char* vertexShaderCode =
    "#version 330\r\n"
    "\n"
    "in vec2 position; \n" //takes in 2 values that represents 2D coordinates 
    "in vec3 vertexColor;\n" //takes in 3 values that represent the RGB values
    "out vec3 theColor;\n" //sends out the color values
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(position, 0.0, 1.0);\n"
    "	 theColor = vertexColor;\n"
    "\n"
    "}\n";

const char* fragShaderCode =
    "#version 330\r\n"
    "in vec3 theColor;\n" //takes in the colors from the vertex shader
    "out vec4 outColor;\n" //outputs the colors
    "\n"
    "void main()\n"
    "{\n"
    "    outColor = vec4(theColor, 1.0);\n"
    "}\n";

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

//This 2D array is used to map the different squares on my picture
GLshort image[16][16] { 
   // 0 = white, 1 = black, 2 = orange, 3 = red, 4 = yellow, 5 = gray   
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //line 0
   {0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, //line 1
   {0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 3, 1, 0}, //line 2
   {0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 1, 3, 3, 1}, //line 3
   {0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 3, 3, 3, 1}, //line 4
   {1, 2, 2, 2, 0, 1, 2, 2, 1, 0, 0, 1, 3, 2, 3, 1}, //line 5
   {1, 2, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 1, 4, 2, 1}, //line 6
   {0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 1, 2, 1, 0}, //line 7
   {0, 0, 1, 1, 1, 2, 2, 1, 2, 2, 1, 1, 2, 2, 1, 0}, //line 8
   {0, 0, 0, 1, 4, 2, 1, 2, 2, 2, 1, 2, 2, 1, 0, 0}, //line 9
   {0, 0, 0, 1, 4, 4, 4, 1, 2, 2, 1, 2, 1, 0, 0, 0}, //line 10
   {0, 0, 1, 0, 1, 1, 4, 2, 2, 2, 1, 1, 0, 0, 0, 0}, //line 11
   {0, 0, 0, 0, 0, 1, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0}, //line 12
   {0, 0, 0, 0, 0, 0, 1, 5, 2, 5, 1, 0, 0, 0, 0, 0}, //line 13
   {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0}, //line 14
   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //line 15
//c:0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
};

/**
* Creates a square based on the locational input. In terms of 
* a 2D array structure i.e. 0, 0 is the first location (top left).
* row: the row coordinate
* col: the column coordinate
* offset: the buffer to jump past the last set of coordinates
*         which is 30 because it takes 30 floats to make a single square.
* vertices: the array that temporarily stores the 30 floats, then is used
*           to copy that info to the main array (allVertices)
*/
void addSquares(int row, int col, int offset, GLfloat *vertices) { 
	GLfloat x = -1.0; // starts from the top left, which in OpenGL
	GLfloat y = 1.0;  // is -1, 1
	
	GLfloat color[3]; // Array used to represent RGB values
	
	//Checks the value in the 2D array and sets the equivalent RGB values.
	switch (image[row][col]) {
		case 0: //white
			color[0] = 1.0;
			color[1] = 1.0;
			color[2] = 1.0;
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
	
	GLfloat tempVertices[] {
		x + squareSize * col, y - squareSize * row, // Triangle 1 Coordinate
		color[0], color[1], color[2], // Triangle 1 Color               
									
		x + squareSize * col, y - squareSize * (row + 1),
		color[0], color[1], color[2],
		
		x + squareSize * (col + 1), y - squareSize * (row + 1),
		color[0], color[1], color[2],              // First Triangle end
	
		
		x + squareSize * col, y - squareSize * row,  //Triangle 2 Coordinate
		color[0], color[1], color[2], // Triangle 2 Color
		
		x + squareSize * (col + 1), y - squareSize * (row + 1),
		color[0], color[1], color[2], 
		
		x + squareSize * (col + 1), y - squareSize * row,
		color[0], color[1], color[2],	    	     //Second Triangle end
	};
	
	//copys square info into allVertices
	for(int i = 0; i < 30; i++) {
		vertices[offset + i] = tempVertices[i];
	}
}

/**
* This method is where the OpenGL buffer and shaders are created and linked to 
* the OpenGL program. A nested for loop is used to call addSquares, and add all
* the squares at their intended location.
*/
void init(void) {
    cout << glGetString(GL_VERSION) << endl;
    
	GLfloat allVertices[7680];
	
	int offset = 0;
	for(int row = 0; row < 16; row++) {
		for(int col = 0; col < 16; col++) {		
			addSquares(row, col, offset, allVertices);
			offset += 30;
		}
	} 
	
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(allVertices), allVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
    
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), BUFFER_OFFSET(2 * sizeof(float))); 
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

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragShaderID);
    glLinkProgram(programID);
    
    checkForProgramError(programID);
	
    glUseProgram(programID);
}


void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);  
    glDrawArrays(GL_TRIANGLES, 0, 1536); //6 vertices per square * 256 squares
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
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
