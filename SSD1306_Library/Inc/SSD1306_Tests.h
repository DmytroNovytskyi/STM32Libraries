/**
 * @brief STM32 HAL-based SSD1306 OLED display driver test library
 *
 * This library provides functions for testing the SSD1306 OLED display driver.
 * It includes various test routines for drawing primitives, text, and display functionalities.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef SSD1306_TEST_H
#define SSD1306_TEST_H

#include <string.h>
#include <stdio.h>
#include <SSD1306.h>
#include <SSD1306_Fonts.h>

void SSD1306_TestBorder(SSD1306_HandleTypeDef *handle);
void SSD1306_TestFonts1(SSD1306_HandleTypeDef *handle);
void SSD1306_TestFonts2(SSD1306_HandleTypeDef *handle);
void SSD1306_TestFPS(SSD1306_HandleTypeDef *handle);
void SSD1306_TestAll(SSD1306_HandleTypeDef *handle);
void SSD1306_TestLine(SSD1306_HandleTypeDef *handle);
void SSD1306_TestRectangle(SSD1306_HandleTypeDef *handle);
void SSD1306_TestRectangleFill(SSD1306_HandleTypeDef *handle);
void SSD1306_TestRectangleInvert(SSD1306_HandleTypeDef *handle);
void SSD1306_TestCircle(SSD1306_HandleTypeDef *handle);
void SSD1306_TestArc(SSD1306_HandleTypeDef *handle);
void SSD1306_TestPolyline(SSD1306_HandleTypeDef *handle);
void SSD1306_TestDrawBitmap(SSD1306_HandleTypeDef *handle);

#endif // SSD1306_TEST_H
