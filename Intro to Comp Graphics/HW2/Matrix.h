#ifndef Matrix_h
#define Matrix_h

#include <GL/glew.h> // Core OpenGL
#include <GL/glut.h> // Display Window

#define MATRIX_DIMENSION 4

class Matrix {
	private:
		GLfloat matrix[MATRIX_DIMENSION][MATRIX_DIMENSION]; 
	public:
		Matrix();
		Matrix createIdentity();
		Matrix operator*(Matrix input);
		float* operator[](int i);
		static Matrix createXRotationMatrix(float degrees);
		static Matrix createYRotationMatrix(float degrees);
		static Matrix createZRotationMatrix(float degrees);
		static Matrix createTranslationMatrix(GLfloat xAmt, GLfloat yAmt, GLfloat zAmt); 
		static Matrix createScaleMatrix(GLfloat scale);
		static Matrix createProjectionMatrix(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far); 
		void printMatrix();
};

#endif

