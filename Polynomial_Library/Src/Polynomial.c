/**
 * @brief This library assists in creation of polynomial
 *
 * Author: Dmytro Novytskyi
 * Version: 1.1
 */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "Polynomial.h"

#define MATRIX_VALUE_PTR( pA, row, col )  (&(((pA)->pContents)[ (row * (pA)->cols) + col]))

typedef struct {
	int rows;
	int cols;
	double *pContents;
} MatrixT;

static MatrixT* Polynomial_CreateMatrix(int rows, int cols);
static MatrixT* Polynomial_TransposeMatrix(MatrixT *pMat);
static MatrixT* Polynomial_CreateProduct(MatrixT *pLeft, MatrixT *pRight);
static void Polynomial_DestroyMatrix(MatrixT *pMat);
static double Polynomial_Power(double base, uint32_t exponent);

void Polynomial_Fit(Point *points, int pointCount, double *coefficientResults, int coefficientCount) {
	// Make the A matrix:
	int degree = coefficientCount - 1;
	MatrixT *pMatA = Polynomial_CreateMatrix(pointCount, coefficientCount);
	for (int r = 0; r < pointCount; r++) {
		for (int c = 0; c < coefficientCount; c++) {
			*(MATRIX_VALUE_PTR(pMatA, r, c)) = Polynomial_Power((points[r].x), degree - c);
		}
	}

	// Make the b matrix
	MatrixT *pMatB = Polynomial_CreateMatrix(pointCount, 1);
	for (int r = 0; r < pointCount; r++) {
		*(MATRIX_VALUE_PTR(pMatB, r, 0)) = points[r].y;
	}

	// Make the transpose of matrix A
	MatrixT *pMatAT = Polynomial_TransposeMatrix(pMatA);

	// Make the product of matrices AT and A:
	MatrixT *pMatATA = Polynomial_CreateProduct(pMatAT, pMatA);

	// Make the product of matrices AT and b:
	MatrixT *pMatATB = Polynomial_CreateProduct(pMatAT, pMatB);

	// Now we need to solve the system of linear equations,
	// (AT)Ax = (AT)b for "x", the coefficients of the polynomial.
	for (int c = 0; c < pMatATA->cols; c++) {
		int pr = c;     // pr is the pivot row.
		double prVal = *MATRIX_VALUE_PTR(pMatATA, pr, c);
		// If it's zero, we can't solve the equations.
		if (0.0 == prVal) {
			break;
		}
		for (int r = 0; r < pMatATA->rows; r++) {
			if (r != pr) {
				double targetRowVal = *MATRIX_VALUE_PTR(pMatATA, r, c);
				double factor = targetRowVal / prVal;
				for (int c2 = 0; c2 < pMatATA->cols; c2++) {
					*MATRIX_VALUE_PTR(pMatATA, r, c2) -= *MATRIX_VALUE_PTR(pMatATA, pr, c2) * factor;

				}
				*MATRIX_VALUE_PTR(pMatATB, r, 0) -= *MATRIX_VALUE_PTR(pMatATB, pr, 0) * factor;
			}
		}
	}
	for (int c = 0; c < pMatATA->cols; c++) {
		int pr = c;
		// now, pr is the pivot row.
		double prVal = *MATRIX_VALUE_PTR(pMatATA, pr, c);
		*MATRIX_VALUE_PTR(pMatATA, pr, c) /= prVal;
		*MATRIX_VALUE_PTR(pMatATB, pr, 0) /= prVal;
	}

	for (int i = 0; i < coefficientCount; i++) {
		coefficientResults[i] = *MATRIX_VALUE_PTR(pMatATB, i, 0);
	}

	Polynomial_DestroyMatrix(pMatATB);
	Polynomial_DestroyMatrix(pMatATA);
	Polynomial_DestroyMatrix(pMatAT);
	Polynomial_DestroyMatrix(pMatA);
	Polynomial_DestroyMatrix(pMatB);
}

void Polynomial_GetDeviation(Point *points, int pointCount, double *coefficients, int coefficientCount,
		double *deviationResults) {
	for (int i = 0; i < pointCount; i++) {
		double result = 0;
		for (int j = 0; j < coefficientCount; j++) {
			result += coefficients[j] * Polynomial_Power(points[i].x, coefficientCount - 1 - j);
		}
		deviationResults[i] = fabs(points[i].y - result);
	}
}

double Polynomial_Calculate(double *coefficients, int coefficientCount, double value) {
	double result = 0.0;
	for (int i = 0; i < coefficientCount; i++) {
		result += coefficients[coefficientCount - 1 - i] * Polynomial_Power(value, i);
	}
	return result;
}

static MatrixT* Polynomial_CreateMatrix(int rows, int cols) {
	MatrixT *rVal = (MatrixT*) calloc(1, sizeof(MatrixT));
	if (NULL != rVal) {
		rVal->rows = rows;
		rVal->cols = cols;
		rVal->pContents = (double*) calloc(rows * cols, sizeof(double));
		if (NULL == rVal->pContents) {
			free(rVal);
			rVal = NULL;
		}
	}
	return rVal;
}

static MatrixT* Polynomial_TransposeMatrix(MatrixT *pMat) {
	MatrixT *rVal = (MatrixT*) calloc(1, sizeof(MatrixT));
	rVal->pContents = (double*) calloc(pMat->rows * pMat->cols, sizeof(double));
	rVal->cols = pMat->rows;
	rVal->rows = pMat->cols;
	for (int r = 0; r < rVal->rows; r++)
			{
		for (int c = 0; c < rVal->cols; c++)
				{
			*MATRIX_VALUE_PTR(rVal, r, c) = *MATRIX_VALUE_PTR(pMat, c, r);
		}
	}
	return rVal;
}

static MatrixT* Polynomial_CreateProduct(MatrixT *pLeft, MatrixT *pRight) {
	MatrixT *rVal = NULL;

	if ((NULL != pLeft) && (NULL != pRight) && (pLeft->cols == pRight->rows)) {
		// Allocate the product matrix.
		rVal = (MatrixT*) calloc(1, sizeof(MatrixT));
		rVal->rows = pLeft->rows;
		rVal->cols = pRight->cols;
		rVal->pContents = (double*) calloc(rVal->rows * rVal->cols, sizeof(double));

		// Initialize the product matrix contents:
		// product[i,j] = sum{k = 0 .. (pLeft->cols - 1)} (pLeft[i,k] * pRight[ k, j])
		for (int i = 0; i < rVal->rows; i++) {
			for (int j = 0; j < rVal->cols; j++) {
				for (int k = 0; k < pLeft->cols; k++) {
					*MATRIX_VALUE_PTR(rVal, i, j) += (*MATRIX_VALUE_PTR(pLeft, i, k))
							* (*MATRIX_VALUE_PTR(pRight, k, j));
				}
			}
		}
	}

	return rVal;
}

static void Polynomial_DestroyMatrix(MatrixT *pMat) {
	if (NULL != pMat) {
		if (NULL != pMat->pContents) {
			free(pMat->pContents);
		}
		free(pMat);
	}
}

static double Polynomial_Power(double base, uint32_t exponent) {
	if (base == 0.0 && exponent == 0) {
		return 1.0;
	}

	double result = 1.0;
	for (uint32_t i = 0; i < exponent; i++) {
		result *= base;
	}
	return result;
}

