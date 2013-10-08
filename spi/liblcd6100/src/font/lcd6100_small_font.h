#pragma once
#include "lcd6100_font.h"

class lcd6100_small_font : public lcd6100_font {

public:
	lcd6100_small_font(void);
	~lcd6100_small_font(void);

	const uint8_t* get_font_table(char c);

private:
	static const uint8_t m_font_table[96][8];
};
