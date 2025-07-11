/**
 * @brief This library assists in creation of polynomial
 *
 * This library provides functions for fitting polynomials to data points and calculating deviations.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.1
 */

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

/**
 * @brief Represents a 2D point.
 *
 * Fields:
 * - x: The x-coordinate of the point.
 * - y: The y-coordinate of the point.
 */
typedef struct {
	double x;
	double y;
} Point;

/**
 * @brief Fits a polynomial to a set of points.
 *
 * This function calculates the coefficients of a polynomial that best fits
 * the given set of points using a least squares approach.
 *
 * @param points             Pointer to an array of Point structures.
 * @param pointCount         The number of points in the array.
 * @param coefficientResults Pointer to an array where the calculated polynomial
 * 							 coefficients will be stored. The array should be large enough
 * 							 to hold 'coefficientCount' doubles. The coefficients are ordered
 * 							 from the highest degree term to the constant term.
 * @param coefficientCount   The number of coefficients (which is degree + 1).
 */
void Polynomial_Fit(Point *points, int pointCount, double *coefficientResults, int coefficientCount);

/**
 * @brief Calculates the deviation between points and a polynomial.
 *
 * This function calculates the difference between the actual y-values of the
 * given points and the y-values predicted by the polynomial for the corresponding
 * x-values.
 *
 * @param points            Pointer to an array of Point structures.
 * @param pointCount        The number of points in the array.
 * @param coefficients      Pointer to an array of polynomial coefficients.
 * @param coefficientCount  The number of coefficients (degree + 1) in the polynomial.
 * @param deviationResults  Pointer to an array where the calculated deviations
 * 							will be stored. The array should be large enough to
 *							hold 'pointCount' doubles.
 */
void Polynomial_GetDeviation(Point *points, int pointCount, double *coefficients, int coefficientCount,
		double *deviationResults);

/**
 * @brief Calculates the value of a polynomial for a given 'x' value.
 *
 * This function evaluates a polynomial $P(x)$ using its coefficients at 'value'.
 * The polynomial is $P(x) = c_nx^n + c_{n-1}x^{n-1} + \dots + c_1x + c_0$.
 *
 * @param coefficients     Array of polynomial coefficients ($c_n$ to $c_0$).
 * @param coefficientCount Number of coefficients (degree + 1).
 * @param value            The 'x' value for evaluation.
 * @return 				   The calculated polynomial value.
 */
double Polynomial_Calculate(double *coefficients, int coefficientCount, double value);

#endif // POLYNOMIAL_H
