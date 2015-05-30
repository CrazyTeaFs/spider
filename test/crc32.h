#ifndef _CRC32_H_
#define _CRC32_H_

#include <stdint.h>

class CCrc32
{
public:
    CCrc32() : crc(0) {}

	// Increment Calculation
    int Update(unsigned char *ptr, uint32_t len);

    uint32_t Final();
	// Caculate Once
    uint32_t Crc32(unsigned char *ptr, uint32_t len);

private:
    uint32_t crc;
};

#endif 
