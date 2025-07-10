/**
 * @brief STM32 HAL-based SSD1306 OLED display driver library
 *
 * This library provides a simple abstraction for controlling SSD1306 OLED displays using I2C protocol.
 * It includes functions for initialization, drawing pixels, lines, circles, rectangles, text, and bitmaps.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f1xx_hal.h"

/**
 * @brief Comment out fonts that are not being used
 */
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26
#define SSD1306_INCLUDE_FONT_16x24
#define SSD1306_INCLUDE_FONT_16x15

/**
 * @brief Enumeration for screen colors.
 */
typedef enum {
	BLACK = 0x00,
	WHITE = 0x01
} SSD1306_Color;

/**
 * @brief SSD1306 OLED display handle structure.
 *
 * This structure holds the configuration for an SSD1306 display device.
 *
 * Fields:
 * - hi2c:               Pointer to the HAL I2C handle associated with this display.
 * - address:            The shifted 7-bit I2C device address of the display.
 * - height:             The height of the display in pixels.
 * - width:              The width of the display in pixels.
 * - mirrorVertically:   Flag to indicate if the display content should be mirrored vertically.
 * - mirrorHorizontally: Flag to indicate if the display content should be mirrored horizontally.
 * - invertColor:        Flag to indicate if the display colors should be inverted.
 */
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;
	uint8_t height;
	uint8_t width;
	bool mirrorVertically;
	bool mirrorHorizontally;
	bool invertColor;
} SSD1306_HandleTypeDef;

/**
 * @brief Structure for a vertex, used in drawing polygons.
 *
 * Fields:
 * - x: X-coordinate of the vertex.
 * - y: Y-coordinate of the vertex.
 */
typedef struct {
	uint8_t x;
	uint8_t y;
} SSD1306_Vertex;

/**
 * @brief Font structure.
 *
 * Fields:
 * - width:     Font width in pixels.
 * - height:    Font height in pixels.
 * - data:      Pointer to font data array.
 * - charWidth: Proportional character width in pixels (NULL for monospaced fonts).
 */
typedef struct {
	const uint8_t width;
	const uint8_t height;
	const uint16_t *const data;
	const uint8_t *const charWidth;
} SSD1306_Font;

/**
 * @brief Initializes the SSD1306 OLED display.
 *
 * Configures the SSD1306 display with the provided handle settings and performs
 * the necessary I2C commands for display setup.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure to initialize.
 * @return 		 HAL_StatusTypeDef HAL_OK if initialization is successful, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef SSD1306_Init(SSD1306_HandleTypeDef *handle);

/**
 * @brief Fills the entire display screen with the specified color.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param color  The color to fill the screen with (BLACK or WHITE).
 */
void SSD1306_Fill(SSD1306_HandleTypeDef *handle, SSD1306_Color color);

/**
 * @brief Writes the screen buffer content to the OLED display.
 *
 * This function transfers the pixel data from the internal screen buffer
 * to the display RAM, making the changes visible on the screen.
 * The number of pages written depends on the screen height (e.g., 8 pages for 64px height).
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 */
void SSD1306_UpdateScreen(SSD1306_HandleTypeDef *handle);

/**
 * @brief Draws a single pixel on the screen buffer at the specified coordinates.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x      X-coordinate of the pixel.
 * @param y      Y-coordinate of the pixel.
 * @param color  Color of the pixel (BLACK or WHITE).
 */
void SSD1306_DrawPixel(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, SSD1306_Color color);

/**
 * @brief Writes a single character to the screen buffer.
 *
 * The character is drawn at the current cursor position with the specified font and color.
 * The cursor position is updated after writing the character.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param ch     The character to write.
 * @param font   The font to use for drawing the character.
 * @param color  The color of the character (BLACK or WHITE).
 * @return 		 The character that was written, or 0 if the character could not be written
 * 				 due to invalid character or insufficient space.
 */
char SSD1306_WriteChar(SSD1306_HandleTypeDef *handle, char ch, SSD1306_Font font, SSD1306_Color color);

/**
 * @brief Writes a null-terminated string to the screen buffer.
 *
 * This function calls SSD1306_WriteChar for each character in the string.
 * It stops if a character cannot be written (e.g., due to lack of space).
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param str    Pointer to the null-terminated string to write.
 * @param font   The font to use for drawing the string.
 * @param color  The color of the string (BLACK or WHITE).
 * @return 		 The last character that was successfully written, or the character
 * 				 that could not be written if an error occurred. Returns 0 if the
 * 				 entire string was written successfully (as *str would be null terminator).
 */
char SSD1306_WriteString(SSD1306_HandleTypeDef *handle, char *str, SSD1306_Font font, SSD1306_Color color);

/**
 * @brief Sets the cursor position for text drawing.
 *
 * @param x X-coordinate for the cursor.
 * @param y Y-coordinate for the cursor.
 */
void SSD1306_SetCursor(uint8_t x, uint8_t y);

/**
 * @brief Draws a line on the screen buffer using Bresenham's algorithm.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x1     X-coordinate of the starting point.
 * @param y1     Y-coordinate of the starting point.
 * @param x2     X-coordinate of the ending point.
 * @param y2     Y-coordinate of the ending point.
 * @param color  Color of the line (BLACK or WHITE).
 */
void SSD1306_Line(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color color);

/**
 * @brief Draws an arc on the screen buffer.
 *
 * The angle is measured clockwise, starting from the 4th quadrant of the trigonometric circle (3pi/2 or 270 degrees).
 *
 * @param handle     Pointer to the SSD1306_HandleTypeDef structure.
 * @param x          X-coordinate of the center of the arc.
 * @param y          Y-coordinate of the center of the arc.
 * @param radius     Radius of the arc.
 * @param startAngle Starting angle in degrees (0-360).
 * @param sweep      Sweep angle in degrees (0-360).
 * @param color      Color of the arc (BLACK or WHITE).
 */
void SSD1306_DrawArc(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, uint8_t radius, uint16_t startAngle,
		uint16_t sweep, SSD1306_Color color);

/**
 * @brief Draws an arc with radius lines on the screen buffer.
 *
 * Draws an arc and connects its endpoints to the center with lines.
 * Angle is measured clockwise, starting from the 4th quadrant of the trigonometric circle (3pi/2 or 270 degrees).
 *
 * @param handle     Pointer to the SSD1306_HandleTypeDef structure.
 * @param x          X-coordinate of the center.
 * @param y          Y-coordinate of the center.
 * @param radius     Radius of the arc.
 * @param startAngle Starting angle in degrees (0-360).
 * @param sweep      Sweep angle in degrees (0-360).
 * @param color      Color of the arc and radius lines (BLACK or WHITE).
 */
void SSD1306_DrawArcWithRadiusLine(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, uint8_t radius,
		uint16_t startAngle, uint16_t sweep, SSD1306_Color color);

/**
 * @brief Draws a circle on the screen buffer using Bresenham's algorithm.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param parX   X-coordinate of the center of the circle.
 * @param parY   Y-coordinate of the center of the circle.
 * @param parR   Radius of the circle.
 * @param color  Color of the circle (BLACK or WHITE).
 */
void SSD1306_DrawCircle(SSD1306_HandleTypeDef *handle, uint8_t parX, uint8_t parY, uint8_t parR, SSD1306_Color color);

/**
 * @brief Draws a filled circle on the screen buffer.
 *
 * Pixel positions are calculated using Bresenham's algorithm.
 *
 * @param handle    Pointer to the SSD1306_HandleTypeDef structure.
 * @param parX      X-coordinate of the center of the circle.
 * @param parY      Y-coordinate of the center of the circle.
 * @param parR      Radius of the circle.
 * @param par_color Color of the filled circle (BLACK or WHITE).
 */
void SSD1306_FillCircle(SSD1306_HandleTypeDef *handle, uint8_t parX, uint8_t parY, uint8_t parR,
		SSD1306_Color par_color);

/**
 * @brief Draws a polyline (a series of connected line segments) on the screen buffer.
 *
 * @param handle    Pointer to the SSD1306_HandleTypeDef structure.
 * @param parVertex Pointer to an array of SSD1306_Vertex structures defining the polyline points.
 * @param parSize   The number of vertices in the polyline.
 * @param color     Color of the polyline (BLACK or WHITE).
 */
void SSD1306_Polyline(SSD1306_HandleTypeDef *handle, const SSD1306_Vertex *parVertex, uint16_t parSize,
		SSD1306_Color color);

/**
 * @brief Draws a rectangle on the screen buffer.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x1     X-coordinate of the top-left corner.
 * @param y1     Y-coordinate of the top-left corner.
 * @param x2     X-coordinate of the bottom-right corner.
 * @param y2     Y-coordinate of the bottom-right corner.
 * @param color  Color of the rectangle (BLACK or WHITE).
 */
void SSD1306_DrawRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		SSD1306_Color color);

/**
 * @brief Draws a filled rectangle on the screen buffer.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x1     X-coordinate of the top-left corner.
 * @param y1     Y-coordinate of the top-left corner.
 * @param x2     X-coordinate of the bottom-right corner.
 * @param y2     Y-coordinate of the bottom-right corner.
 * @param color  Color of the filled rectangle (BLACK or WHITE).
 */
void SSD1306_FillRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		SSD1306_Color color);

/**
 * @brief Inverts the color of pixels within a specified rectangular area.
 *
 * This function flips the color of each pixel within the rectangle defined by
 * (x1, y1) and (x2, y2), including the border.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x1     X-coordinate of the top-left corner.
 * @param y1     Y-coordinate of the top-left corner.
 * @param x2     X-coordinate of the bottom-right corner.
 * @param y2     Y-coordinate of the bottom-right corner.
 * @return 		 HAL_StatusTypeDef status HAL_OK if successful, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef SSD1306_InvertRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Draws a bitmap image on the screen buffer.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param x      X-coordinate of the top-left corner of the bitmap.
 * @param y      Y-coordinate of the top-left corner of the bitmap.
 * @param bitmap Pointer to the array containing the bitmap data.
 * @param w      Width of the bitmap in pixels.
 * @param h      Height of the bitmap in pixels.
 * @param color  Color to draw the bitmap (BLACK or WHITE).
 */
void SSD1306_DrawBitmap(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, const unsigned char *bitmap, uint8_t w,
		uint8_t h, SSD1306_Color color);

/**
 * @brief Sets the contrast of the OLED display.
 *
 * The contrast increases as the value increases.
 * The reset value for contrast is 0x7F.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param value  The contrast value to set (0x00 to 0xFF).
 */
void SSD1306_SetContrast(SSD1306_HandleTypeDef *handle, const uint8_t value);

/**
 * @brief Sets the display ON or OFF.
 *
 * @param handle Pointer to the SSD1306_HandleTypeDef structure.
 * @param state  'true' to turn the display ON, 'false' to turn it OFF.
 */
void SSD1306_SetDisplayOn(SSD1306_HandleTypeDef *handle, bool state);

/**
 * @brief Reads the current display ON/OFF state.
 *
 * @return 'false' if the display is OFF, 'true' if the display is ON.
 */
bool SSD1306_GetDisplayOn();

#endif // SSD1306_H
