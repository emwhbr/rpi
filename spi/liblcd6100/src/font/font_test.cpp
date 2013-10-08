// lcd6100_font.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "lcd6100_small_font.h"


lcd6100_small_font g_font;

static void print_char(char c) {
	const uint8_t *font_table = g_font.get_font_table(c);

	for (int i=0; i < g_font.get_height(); i++) {
		printf("0x%x\n", font_table[i]);
	}
	printf("\n");

	uint8_t *char_data = (uint8_t *)font_table + g_font.get_bytes_per_char() - 1;

	for (int i=0; i < g_font.get_height(); i++) {
	
		// copy pixel row from font table and then decrement row
		uint8_t pixel_row = *char_data--;

		// loop on each pixel in the row (left to right)
		// Note: we do two pixels each loop
		uint8_t pixel_mask = 0x80;
		for (int j=0; j < g_font.get_width(); j+=2) {
	
			if ((pixel_row & pixel_mask) == 0)
				printf(" ");
			else
				printf("*");
		
			pixel_mask = pixel_mask >> 1;
			if ((pixel_row & pixel_mask) == 0)
				printf(" ");
			else
				printf("*");

			pixel_mask = pixel_mask >> 1;
		}
		printf("\n");
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Font is %u x %u, bytes per char: %u\n\n",
		   g_font.get_width(), g_font.get_height(),
		   g_font.get_bytes_per_char());

	print_char('A');
	print_char('a');

	return 0;
}

