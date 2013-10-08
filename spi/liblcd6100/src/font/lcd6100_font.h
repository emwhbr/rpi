#pragma once

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef unsigned __int8  uint8_t;
#else
#include <stdint.h>
#endif

class lcd6100_font {

public:
	lcd6100_font(void);
	~lcd6100_font(void);

	uint8_t get_height(void) { return m_height;};	
	uint8_t get_width(void)  { return m_width;};
	
	uint8_t get_bytes_per_char(void) {
		return m_bytes_per_char;
	};

	virtual const uint8_t* get_font_table(char c) = 0;

protected:
	uint8_t m_height;
	uint8_t m_width;
	uint8_t m_bytes_per_char;
};
