#include "Matrix.h"

#include <iostream>
using namespace std;

#include <math.h>

#include <GL/glew.h> // Core OpenGL
#include <GL/glut.h> // Display Window

#define PI 3.1415926535

Matrix::Matrix() {
}

Matrix Matrix::createIdentity() {
	Matrix result;
	for(int i = 0; i < MATRIX_DIMENSION; i++) {
		for(int j = 0; j < MATRIX_DIMENSION; j++) { 
			result[i][j] = 0; 
		}
	}
	result[0][0] = 1;
	result[1][1] = 1;
	result[2][2] = 1;
	result[3][3] = 1;
	
	return result;
}

float* Matrix::operator[](int i) { 
	return matrix[i];
}	 

Matrix Matrix::operator*(Matrix input) { 
	Matrix result;
        for(int i = 0; i < MATRIX_DIMENSION; i++)
        {
            for(int j = 0; j < MATRIX_DIMENSION; j++)
            {
                result[i][j] = 0;
                for(int k = 0; k < MATRIX_DIMENSION; k++)
                {
                    result[i][j] += matrix[i][k] * input[k][j];        
                }
            }
        }
	return result;
}

Matrix Matrix::createTranslationMatrix(GLfloat xAmt, GLfloat yAmt, GLfloat zAmt) { 
	Matrix result = result.createIdentity();

	result[0][3] = xAmt;
	result[1][3] = yAmt;
	result[2][3] = zAmt;
	
	return result;
} 

Matrix Matrix::createXRotationMatrix(float degrees) {
	Matrix result = result.createIdentity();
	
	float radians = (degrees * PI)/180.0;
	
	float cosRes = cos(radians);
	float sinRes = sin(radians);
	
	result[1][1] = cosRes;
	result[1][2] = -sinRes;
	result[2][1] = sinRes;
	result[2][2] = cosRes;
	
	return result;
}

Matrix Matrix::createYRotationMatrix(float degrees) {
	Matrix result = result.createIdentity();
	
	float radians = (degrees * PI)/180.0;
	
	float cosRes = cos(radians);
	float sinRes = sin(radians);
	
	result[0][0] = cosRes;
	result[0][2] = sinRes;
	result[2][0] = -sinRes;
	result[2][2] = cosRes;
	
	return result;
}

Matrix Matrix::createZRotationMatrix(float degrees) { 	
	Matrix result = result.createIdentity();
	
	float radians = (degrees * PI)/180.0;
	
	float cosRes = cos(radians);
	float sinRes = sin(radians);
	
	result[0][0] = cosRes;
	result[0][1] = -sinRes;
	result[1][0] = sinRes;
	result[1][1] = cosRes;
	
	return result;
}

Matrix Matrix::createScaleMatrix(GLfloat scale) {
	Matrix result = result.createIdentity();
	
	result[0][0] = scale;
	result[1][1] = scale;
	result[2][2] = scale; 	
	
	return result;
}

Matrix Matrix::createProjectionMatrix(GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far) {
	Matrix result = result.createIdentity();
	
	GLfloat tanHalfAngle = tan(((fov * PI) / 360.0));
	
	result[0][0] = (1 / tanHalfAngle); //a
	result[1][1] = (1 / tanHalfAngle); //b
	result[2][2] = (far + near) / (far - near); //c
	result[2][3] = (2 * far * near) / (far - near); //d
	result[3][2] = 1.0; //e
	result[3][3] = 0.0; //w
	
	return result;
}
void Matrix::printMatrix() {
	for(int i = 0; i < MATRIX_DIMENSION; i++) {
		for(int j = 0; j < MATRIX_DIMENSION; j++) { 
			cout << matrix[i][j] << " ";
		}
		cout << "\n";
	}
	cout << "\n";
}
/*
int main() {	
	Matrix a = a.createProjectionMatrix(60, 1, 1, 10);
	a.printMatrix();
	
	Matrix z = z.createTranslationMatrix(-2, 2, 10);
	z.printMatrix();
	
	Matrix c;
	c = a * z;
	c.printMatrix();
}*/














