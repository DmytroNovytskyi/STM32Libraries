# Polynomial Library

## Version

### *1.1*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to use polynomial library.

---

## Example Usage

```c
#define DATA_SIZE 7
#define COEFFICIENTS_SIZE 3

Point points[DATA_SIZE] = {
  { -2.0, 12.0 },
  { -1.0, 6.0 },
  { 0.0, 2.0 },
  { 1.0, 0.0 },
  { 2.0, 0.0 },
  { 3.0, 2.0 },
  { 4.0, 6.0 }
};
double coefficients[COEFFICIENTS_SIZE];
double deviation[DATA_SIZE] = { 0 };
double result;

Polynomial_Fit(points, DATA_SIZE, coefficients, COEFFICIENTS_SIZE);
Polynomial_GetDeviation(points, DATA_SIZE, coefficients, COEFFICIENTS_SIZE, deviation);
result = Polynomial_Calculate(coefficients, COEFFICIENTS_SIZE, -1);
```

## Results

Given the input points for the polynomial $y=x^2-3x+2$, and with the `Polynomial_GetDeviation` function updated to calculate absolute residuals, the `coefficients` and `deviation` arrays would contain values similar to these:

`coefficients` **array (for $x^2-3x+2$):**

| Index             | Value                 |
|:------------------|:----------------------|
| `coefficients[0]` | `1`                   |
| `coefficients[1]` | `-2.9999999999999996` |
| `coefficients[2]` | `1.9999999999999996`  |

---

`deviation` **array (absolute residuals):**

| Index          | Value       |
|:---------------|:------------|
| `deviation[0]` | `0`         |
| `deviation[1]` | `~8.88e-16` |
| `deviation[2]` | `~4.40e-16` |
| `deviation[3]` | `0`         |
| `deviation[4]` | `~4.40e-16` |
| `deviation[5]` | `~1.33e-15` |
| `deviation[6]` | `~1.77e-15` |

---

**P(-1):** `result` = `5.9999999999999991`
