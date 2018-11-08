#ifndef CODAS_BIT_H_
#define CODAS_BIT_H

#include <stdint.h>

#define GBIT8(b) (*(uint8_t *)(b))
#define GBIT16(b) (GBIT8(b) + ((uint16_t) GBIT8((uint8_t *)(b)+1) << 8))
#define GBIT32(b) (GBIT16(b) + ((uint32_t) GBIT16((uint8_t *)(b)+2) << 16))
#define GBIT64(b) (GBIT32(b) + ((uint64_t) GBIT32((uint8_t *)(b)+4) << 32))

#define PBIT8(b, v) (*(uint8_t *)(b) = (v))
#define PBIT16(b, v) (PBIT8(b, (v) & 0xFF), PBIT8((uint8_t *)(b)+1, (v) >> 8))
#define PBIT32(b, v) (PBIT16(b, (v) & 0xFFFF), PBIT16((uint8_t *)(b)+2, (v) >> 16))
#define PBIT64(b, v) (PBIT32(b, (v) & 0xFFFFFFFF), PBIT32((uint8_t *)(b)+4, (v) >> 32))

#endif /* CODAS_BIT_H_ */

