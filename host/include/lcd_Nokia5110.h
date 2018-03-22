#ifndef __lcd_Nokia5110_H__
#define __lcd_Nokia5110_H__

#include "nrf.h"
#include <stdint.h>
#include "mydefinitions.h"

//================================== rozmiary LCD =========================================
#define LCD_WIDTH					84
#define LCD_HEIGHT					48

#define SCK_p 						24
#define MOSI_p 						23

#define LCD_DC 						22						//linia ustalajaca, czy przesylamy dane, czy komendy dla wyswietlacza
#define LCD_CE 						20  					//jest to linia CS interfejsu SPI, nazwa pochodzi od Chip Enable
#define LCD_RST 					19						//linia resetujaca rejestry wyswietlacza

#define LCD_CE_LOW() 				GPIO->OUTCLR = (1U << LCD_CE);
#define LCD_CE_HIGH() 				GPIO->OUTSET = (1U << LCD_CE);

#define LCD_DC_LOW() 				GPIO->OUTCLR = (1U << LCD_DC);
#define LCD_DC_HIGH() 				GPIO->OUTSET = (1U << LCD_DC);

#define LCD_RST_LOW() 				GPIO->OUTCLR = (1U << LCD_RST);
#define LCD_RST_HIGH() 				GPIO->OUTSET = (1U << LCD_RST);


void lcd_cmd(uint8_t cmd);
void lcd_init(void);
 
void lcd_clear(void);																				// clear screen
void lcd_clear_pixel(int x, int y);
void lcd_draw_bitmap(const uint8_t* data);									// draw a bitmap for hex table
void lcd_draw_text(int row, int col, const char* text);			// displays text
void lcd_draw_pixel(int x, int y);													// draw pixel
void lcd_draw_line(int x1, int y1, int x2, int y2);					// draw a line (algorithm Brensenhama)
 
void lcd_copy(void);

#endif /* __lcd_Nokia5110_H__ */
