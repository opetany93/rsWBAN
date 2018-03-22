#include "lcd_Nokia5110.h"
#include <string.h>
#include "font.h"
#include "spi.h"

#define PCD8544_FUNCTION_SET		0x20
#define PCD8544_DISP_CONTROL		0x08
#define PCD8544_DISP_NORMAL			0x0C
#define PCD8544_SET_Y				0x40
#define PCD8544_SET_X				0x80
#define PCD8544_H_TC				0x04
#define PCD8544_H_BIAS				0x10
#define PCD8544_H_VOP				0x80
 
#define LCD_BUFFER_SIZE				(84 * 48 / 8)
 
uint8_t lcd_buffer[LCD_BUFFER_SIZE];

static spiHandle_t lcdSpiHandle;

//======================================================================================
void lcd_init(void)
{
	// argumenty: SCK, MOSI, MISO, CS, frequency, mode
	lcdSpiHandle = spiInit(SPI0, SCK_p, MOSI_p, SPI_PIN_NONE, LCD_CE, SPI_FREQUENCY_M4, SPI_MODE_0, SPI_ORDER_MSB_FIRST);
	
	
	//LCD_DC
	NRF_GPIO->PIN_CNF[LCD_DC] =  (GPIO_PIN_CNF_DIR_Output   << GPIO_PIN_CNF_DIR_Pos)
							   | (GPIO_PIN_CNF_PULL_Pullup  << GPIO_PIN_CNF_PULL_Pos);
	LCD_DC_HIGH();
	
	//LCD_RST
	NRF_GPIO->PIN_CNF[LCD_RST] = (GPIO_PIN_CNF_DIR_Output   << GPIO_PIN_CNF_DIR_Pos)
							   | (GPIO_PIN_CNF_PULL_Pullup  << GPIO_PIN_CNF_PULL_Pos);
	
	LCD_RST_HIGH();
	
	LCD_RST_LOW();
	LCD_RST_HIGH();							//w razie problemów dodac opoznienie
	
	lcd_cmd(PCD8544_FUNCTION_SET | 1);
	lcd_cmd(PCD8544_H_BIAS | 4);
	lcd_cmd(PCD8544_H_VOP | 0x2F);			//kontrast
	lcd_cmd(PCD8544_FUNCTION_SET);
	lcd_cmd(PCD8544_DISP_NORMAL);
}

//======================================================================================
void lcd_clear(void)
{
	memset(lcd_buffer, 0, LCD_BUFFER_SIZE);
}

//======================================================================================
void lcd_draw_bitmap(const uint8_t* data)
{
	memcpy(lcd_buffer, data, LCD_BUFFER_SIZE);
}

//======================================================================================
void lcd_draw_text(int row, int col, const char* text)
{
	int i;
	uint8_t* pbuf = &lcd_buffer[row * 84 + col];

	while ((*text) && (pbuf < &lcd_buffer[LCD_BUFFER_SIZE - 6]))
	{
		int ch = *text++;
		const uint8_t* font = &font_ASCII[ch - ' '][0];
		for (i = 0; i < 5; i++)
		{
			*pbuf++ = *font++;
		}

		*pbuf++ = 0;
	}
}

//======================================================================================
void lcd_draw_pixel(int x, int y)
{
	lcd_buffer[ x + (y >> 3) * LCD_WIDTH] |= 1 << (y & 7);
}

//======================================================================================
void lcd_clear_pixel(int x, int y)
{
	lcd_buffer[ x + (y >> 3) * LCD_WIDTH] &= ~(1 << (y & 7));
}

//======================================================================================
void lcd_draw_line(int x1, int y1, int x2, int y2)
{
	int dx, dy, sx, sy, dx2, dy2, err;
	
	 if (x2 >= x1)
	 {
			dx = x2 - x1;
			sx = 1;
	 }
	 else
	 {
			dx = x1 - x2;
			sx = -1;
	 }

	 if (y2 >= y1)
	 {
			dy = y1 - y2;
			sy = 1;
	 }
	 else
	 {
			dy = y2 - y1;
			sy = -1;
	 }
	 
	 dx2 = dx << 1;
	 dy2 = dy << 1;

	 err = dx2 + dy2;
 
	while (1)
	{
		lcd_draw_pixel(x1, y1);
		if (err >= dy)
		{
			if (x1 == x2) break;
			err += dy2;
			x1 += sx;
		}
			
		if (err <= dx)
		{
			if (y1 == y2) break;
			err += dx2;
			y1 += sy;
		}
	}
}

//======================================================================================
void lcd_cmd(uint8_t cmd)
{
	LCD_DC_LOW();
	spiControlChipSelect(lcdSpiHandle, 0);
	
	spiSend(lcdSpiHandle, cmd);
	
	spiControlChipSelect(lcdSpiHandle, 1);
	LCD_DC_HIGH();
}

//======================================================================================
void lcd_copy(void)
{
	int i;
	LCD_DC_HIGH();
	spiControlChipSelect(lcdSpiHandle, 0);
	
	for (i = 0; i < LCD_BUFFER_SIZE; i++) spiSend(lcdSpiHandle, lcd_buffer[i]);
	
	spiControlChipSelect(lcdSpiHandle, 1);
}
