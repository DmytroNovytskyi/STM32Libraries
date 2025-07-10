/**
 * @brief Implementation of SSD1306 OLED display driver library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include <SSD1306.h>

//Structure to store the current state
typedef struct {
	uint16_t currentX;
	uint16_t currentY;
	bool displayOn;
} SSD1306_State;

//Screenbuffer
static uint8_t buffer[2048];

//Screen state
static SSD1306_State state;

//Send a byte to the command register
static void SSD1306_WriteCommand(SSD1306_HandleTypeDef *handle, uint8_t byte);

//Send data
static void SSD1306_WriteData(SSD1306_HandleTypeDef *handle, uint8_t *buffer, size_t buff_size);

//Convert Degrees to Radians
static float SSD1306_DegToRad(float parDeg);

//Normalize degree to [0;360]
static uint16_t SSD1306_NormalizeTo0_360(uint16_t parDeg);

HAL_StatusTypeDef SSD1306_Init(SSD1306_HandleTypeDef *handle) {
	if (handle == NULL || handle->hi2c == NULL || handle->width > 128
			|| (handle->height != 32 && handle->height != 64 && handle->height != 128)) {
		return HAL_ERROR;
	}

	//Wait for the screen to boot
	HAL_Delay(100);

	//Init OLED
	SSD1306_SetDisplayOn(handle, false); //display off

	SSD1306_WriteCommand(handle, 0x20); //Set Memory Addressing Mode
	//00b -> Horizontal Addressing Mode; 01b -> Vertical Addressing Mode;
	//10b -> Page Addressing Mode (RESET); 11b -> Invalid
	SSD1306_WriteCommand(handle, 0x00);

	SSD1306_WriteCommand(handle, 0xB0); //Set Page Start Address for Page Addressing Mode,0-7

	if (handle->mirrorVertically) {
		SSD1306_WriteCommand(handle, 0xC0); //Mirror vertically
	} else {
		SSD1306_WriteCommand(handle, 0xC8); //Set COM Output Scan Direction
	}

	SSD1306_WriteCommand(handle, 0x00); //Low column address
	SSD1306_WriteCommand(handle, 0x10); //High column address

	SSD1306_WriteCommand(handle, 0x40); //Start line address - CHECK

	SSD1306_SetContrast(handle, 0xFF);

	if (handle->mirrorHorizontally) {
		SSD1306_WriteCommand(handle, 0xA0); //Mirror horizontally
	} else {
		SSD1306_WriteCommand(handle, 0xA1); //Segment re-map 0 to 127 - CHECK
	}

	if (handle->invertColor) {
		SSD1306_WriteCommand(handle, 0xA7); //Inverse color
	} else {
		SSD1306_WriteCommand(handle, 0xA6); //Normal color
	}

	// Set multiplex ratio.
	if (handle->height == 128) {
		SSD1306_WriteCommand(handle, 0xFF);
	} else {
		SSD1306_WriteCommand(handle, 0xA8); //Multiplex ratio(1 to 64) - CHECK
	}

	if (handle->height == 32) {
		SSD1306_WriteCommand(handle, 0x1F);
	} else if (handle->height == 64 || handle->height == 128) {
		SSD1306_WriteCommand(handle, 0x3F);
	}

	SSD1306_WriteCommand(handle, 0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	SSD1306_WriteCommand(handle, 0xD3); //Display offset - CHECK
	SSD1306_WriteCommand(handle, 0x00); //No offset

	SSD1306_WriteCommand(handle, 0xD5); //Display clock divide ratio/oscillator frequency
	SSD1306_WriteCommand(handle, 0xF0); //Divide ratio

	SSD1306_WriteCommand(handle, 0xD9); //Pre-charge period
	SSD1306_WriteCommand(handle, 0x22);

	SSD1306_WriteCommand(handle, 0xDA); //Com pins hardware configuration - CHECK
	if (handle->height == 32) {
		SSD1306_WriteCommand(handle, 0x02);
	} else if (handle->height == 64) {
		SSD1306_WriteCommand(handle, 0x12);
	} else if (handle->height == 128) {
		SSD1306_WriteCommand(handle, 0x12);
	}

	SSD1306_WriteCommand(handle, 0xDB); //Set vcomh
	SSD1306_WriteCommand(handle, 0x20); //0x20,0.77xVcc

	SSD1306_WriteCommand(handle, 0x8D); //DC-DC enable
	SSD1306_WriteCommand(handle, 0x14);
	SSD1306_SetDisplayOn(handle, true); //Turn on state panel

	SSD1306_Fill(handle, BLACK); //Clear screen

	SSD1306_UpdateScreen(handle); //Flush buffer to screen

	// Set default values for screen state
	state.currentX = 0;
	state.currentY = 0;
	return HAL_OK;
}

void SSD1306_Fill(SSD1306_HandleTypeDef *handle, SSD1306_Color color) {
	memset(buffer, (color == BLACK) ? 0x00 : 0xFF, handle->height * handle->width / 8);
}

void SSD1306_UpdateScreen(SSD1306_HandleTypeDef *handle) {
	for (uint8_t i = 0; i < handle->height / 8; i++) {
		SSD1306_WriteCommand(handle, 0xB0 + i); // Set the current RAM page address.
		SSD1306_WriteCommand(handle, 0x00);
		SSD1306_WriteCommand(handle, 0x10);
		SSD1306_WriteData(handle, &buffer[handle->width * i], handle->width);
	}
}

void SSD1306_DrawPixel(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, SSD1306_Color color) {
	if (x >= handle->width || y >= handle->height) {
		//Don't write outside the buffer
		return;
	}

	//Draw in the right color
	if (color == WHITE) {
		buffer[x + (y / 8) * handle->width] |= 1 << (y % 8);
	} else {
		buffer[x + (y / 8) * handle->width] &= ~(1 << (y % 8));
	}
}

char SSD1306_WriteChar(SSD1306_HandleTypeDef *handle, char ch, SSD1306_Font font, SSD1306_Color color) {
	uint32_t i, b, j;

	//Check if character is valid
	if (ch < 32 || ch > 126)
		return 0;

	//Char width is not equal to font width for proportional font
	const uint8_t char_width = font.charWidth ? font.charWidth[ch - 32] : font.width;
	//Check remaining space on current line
	if (handle->width < (state.currentX + char_width) ||
			handle->height < (state.currentY + font.height)) {
		//Not enough space on current line
		return 0;
	}

	//Use the font to write
	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < char_width; j++) {
			if ((b << j) & 0x8000) {
				SSD1306_DrawPixel(handle, state.currentX + j, (state.currentY + i), (SSD1306_Color) color);
			} else {
				SSD1306_DrawPixel(handle, state.currentX + j, (state.currentY + i), (SSD1306_Color) !color);
			}
		}
	}

	//The current space is now taken
	state.currentX += char_width;

	//Return written char for validation
	return ch;
}

char SSD1306_WriteString(SSD1306_HandleTypeDef *handle, char *str, SSD1306_Font font, SSD1306_Color color) {
	while (*str) {
		if (SSD1306_WriteChar(handle, *str, font, color) != *str) {
			//Char could not be written
			return *str;
		}
		str++;
	}

	//Everything ok
	return *str;
}

void SSD1306_SetCursor(uint8_t x, uint8_t y) {
	state.currentX = x;
	state.currentY = y;
}

void SSD1306_Line(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_Color color) {
	int32_t deltaX = abs(x2 - x1);
	int32_t deltaY = abs(y2 - y1);
	int32_t signX = ((x1 < x2) ? 1 : -1);
	int32_t signY = ((y1 < y2) ? 1 : -1);
	int32_t error = deltaX - deltaY;
	int32_t error2;

	SSD1306_DrawPixel(handle, x2, y2, color);

	while ((x1 != x2) || (y1 != y2)) {
		SSD1306_DrawPixel(handle, x1, y1, color);
		error2 = error * 2;
		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
		}

		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
		}
	}
}

void SSD1306_Polyline(SSD1306_HandleTypeDef *handle, const SSD1306_Vertex *parVertex, uint16_t parSize,
		SSD1306_Color color) {
	uint16_t i;
	if (parVertex == NULL) {
		return;
	}

	for (i = 1; i < parSize; i++) {
		SSD1306_Line(handle, parVertex[i - 1].x, parVertex[i - 1].y, parVertex[i].x, parVertex[i].y, color);
	}
}

void SSD1306_DrawArc(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, uint8_t radius, uint16_t startAngle,
		uint16_t sweep, SSD1306_Color color) {
	static const uint8_t CIRCLE_APPROXIMATION_SEGMENTS = 36;
	float approx_degree;
	uint32_t approx_segments;
	uint8_t xp1, xp2;
	uint8_t yp1, yp2;
	uint32_t count;
	uint32_t loc_sweep;
	float rad;

	loc_sweep = SSD1306_NormalizeTo0_360(sweep);

	count = (SSD1306_NormalizeTo0_360(startAngle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
	approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
	approx_degree = loc_sweep / (float) approx_segments;
	while (count < approx_segments)
	{
		rad = SSD1306_DegToRad(count * approx_degree);
		xp1 = x + (int8_t) (sinf(rad) * radius);
		yp1 = y + (int8_t) (cosf(rad) * radius);
		count++;
		if (count != approx_segments) {
			rad = SSD1306_DegToRad(count * approx_degree);
		} else {
			rad = SSD1306_DegToRad(loc_sweep);
		}
		xp2 = x + (int8_t) (sinf(rad) * radius);
		yp2 = y + (int8_t) (cosf(rad) * radius);
		SSD1306_Line(handle, xp1, yp1, xp2, yp2, color);
	}

	return;
}

void SSD1306_DrawArcWithRadiusLine(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, uint8_t radius,
		uint16_t startAngle, uint16_t sweep, SSD1306_Color color) {
	const uint32_t CIRCLE_APPROXIMATION_SEGMENTS = 36;
	float approx_degree;
	uint32_t approx_segments;
	uint8_t xp1;
	uint8_t xp2 = 0;
	uint8_t yp1;
	uint8_t yp2 = 0;
	uint32_t count;
	uint32_t loc_sweep;
	float rad;

	loc_sweep = SSD1306_NormalizeTo0_360(sweep);

	count = (SSD1306_NormalizeTo0_360(startAngle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
	approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
	approx_degree = loc_sweep / (float) approx_segments;

	rad = SSD1306_DegToRad(count * approx_degree);
	uint8_t first_point_x = x + (int8_t) (sinf(rad) * radius);
	uint8_t first_point_y = y + (int8_t) (cosf(rad) * radius);
	while (count < approx_segments) {
		rad = SSD1306_DegToRad(count * approx_degree);
		xp1 = x + (int8_t) (sinf(rad) * radius);
		yp1 = y + (int8_t) (cosf(rad) * radius);
		count++;
		if (count != approx_segments) {
			rad = SSD1306_DegToRad(count * approx_degree);
		} else {
			rad = SSD1306_DegToRad(loc_sweep);
		}
		xp2 = x + (int8_t) (sinf(rad) * radius);
		yp2 = y + (int8_t) (cosf(rad) * radius);
		SSD1306_Line(handle, xp1, yp1, xp2, yp2, color);
	}

	//Radius line
	SSD1306_Line(handle, x, y, first_point_x, first_point_y, color);
	SSD1306_Line(handle, x, y, xp2, yp2, color);
	return;
}

void SSD1306_DrawCircle(SSD1306_HandleTypeDef *handle, uint8_t parX, uint8_t parY, uint8_t parR,
		SSD1306_Color color) {
	int32_t x = -parR;
	int32_t y = 0;
	int32_t err = 2 - 2 * parR;
	int32_t e2;

	if (parX >= handle->width || parY >= handle->height) {
		return;
	}

	do {
		SSD1306_DrawPixel(handle, parX - x, parY + y, color);
		SSD1306_DrawPixel(handle, parX + x, parY + y, color);
		SSD1306_DrawPixel(handle, parX + x, parY - y, color);
		SSD1306_DrawPixel(handle, parX - x, parY - y, color);
		e2 = err;

		if (e2 <= y) {
			y++;
			err = err + (y * 2 + 1);
			if (-x == y && e2 <= x) {
				e2 = 0;
			}
		}

		if (e2 > x) {
			x++;
			err = err + (x * 2 + 1);
		}
	} while (x <= 0);

	return;
}

void SSD1306_FillCircle(SSD1306_HandleTypeDef *handle, uint8_t parX, uint8_t parY, uint8_t parR,
		SSD1306_Color color) {
	int32_t x = -parR;
	int32_t y = 0;
	int32_t err = 2 - 2 * parR;
	int32_t e2;

	if (parX >= handle->width || parY >= handle->height) {
		return;
	}

	do {
		for (uint8_t _y = (parY + y); _y >= (parY - y); _y--) {
			for (uint8_t _x = (parX - x); _x >= (parX + x); _x--) {
				SSD1306_DrawPixel(handle, _x, _y, color);
			}
		}

		e2 = err;
		if (e2 <= y) {
			y++;
			err = err + (y * 2 + 1);
			if (-x == y && e2 <= x) {
				e2 = 0;
			}
		}

		if (e2 > x) {
			x++;
			err = err + (x * 2 + 1);
		}
	} while (x <= 0);
}

void SSD1306_DrawRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		SSD1306_Color color) {
	SSD1306_Line(handle, x1, y1, x2, y1, color);
	SSD1306_Line(handle, x2, y1, x2, y2, color);
	SSD1306_Line(handle, x2, y2, x1, y2, color);
	SSD1306_Line(handle, x1, y2, x1, y1, color);

	return;
}

void SSD1306_FillRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		SSD1306_Color color) {
	uint8_t xStart = ((x1 <= x2) ? x1 : x2);
	uint8_t xEnd = ((x1 <= x2) ? x2 : x1);
	uint8_t yStart = ((y1 <= y2) ? y1 : y2);
	uint8_t yEnd = ((y1 <= y2) ? y2 : y1);

	for (uint8_t y = yStart; (y <= yEnd) && (y < handle->height); y++) {
		for (uint8_t x = xStart; (x <= xEnd) && (x < handle->width); x++) {
			SSD1306_DrawPixel(handle, x, y, color);
		}
	}
}

HAL_StatusTypeDef SSD1306_InvertRectangle(SSD1306_HandleTypeDef *handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	if ((x2 >= handle->width) || (y2 >= handle->height)) {
		return HAL_ERROR;
	}
	if ((x1 > x2) || (y1 > y2)) {
		return HAL_ERROR;
	}
	uint32_t i;
	if ((y1 / 8) != (y2 / 8)) {
		/* if rectangle doesn't lie on one 8px row */
		for (uint32_t x = x1; x <= x2; x++) {
			i = x + (y1 / 8) * handle->width;
			buffer[i] ^= 0xFF << (y1 % 8);
			i += handle->width;
			for (; i < x + (y2 / 8) * handle->width; i += handle->width) {
				buffer[i] ^= 0xFF;
			}
			buffer[i] ^= 0xFF >> (7 - (y2 % 8));
		}
	} else {
		/* if rectangle lies on one 8px row */
		const uint8_t mask = (0xFF << (y1 % 8)) & (0xFF >> (7 - (y2 % 8)));
		for (i = x1 + (y1 / 8) * handle->width;
				i <= (uint32_t) x2 + (y2 / 8) * handle->width; i++) {
			buffer[i] ^= mask;
		}
	}
	return HAL_OK;
}

void SSD1306_DrawBitmap(SSD1306_HandleTypeDef *handle, uint8_t x, uint8_t y, const unsigned char *bitmap, uint8_t w,
		uint8_t h, SSD1306_Color color) {
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	if (x >= handle->width || y >= handle->height) {
		return;
	}

	for (uint8_t j = 0; j < h; j++, y++) {
		for (uint8_t i = 0; i < w; i++) {
			if (i & 7) {
				byte <<= 1;
			} else {
				byte = (*(const unsigned char*) (&bitmap[j * byteWidth + i / 8]));
			}

			if (byte & 0x80) {
				SSD1306_DrawPixel(handle, x + i, y, color);
			}
		}
	}
}

void SSD1306_SetContrast(SSD1306_HandleTypeDef *handle, const uint8_t value) {
	SSD1306_WriteCommand(handle, 0x81); //Set contrast control register
	SSD1306_WriteCommand(handle, value);
}

void SSD1306_SetDisplayOn(SSD1306_HandleTypeDef *handle, bool status) {
	uint8_t value;
	if (status) {
		value = 0xAF;   //Display on
		state.displayOn = true;
	} else {
		value = 0xAE;   //Display off
		state.displayOn = false;
	}
	SSD1306_WriteCommand(handle, value);
}

bool SSD1306_GetDisplayOn() {
	return state.displayOn;
}

static void SSD1306_WriteCommand(SSD1306_HandleTypeDef *handle, uint8_t byte) {
	HAL_I2C_Mem_Write(handle->hi2c, handle->address, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

static void SSD1306_WriteData(SSD1306_HandleTypeDef *handle, uint8_t *buffer, size_t buff_size) {
	HAL_I2C_Mem_Write(handle->hi2c, handle->address, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

static float SSD1306_DegToRad(float parDeg) {
	return parDeg * (3.14f / 180.0f);
}

static uint16_t SSD1306_NormalizeTo0_360(uint16_t parDeg) {
	uint16_t loc_angle;
	if (parDeg <= 360) {
		loc_angle = parDeg;
	} else {
		loc_angle = parDeg % 360;
		loc_angle = (loc_angle ? loc_angle : 360);
	}
	return loc_angle;
}
