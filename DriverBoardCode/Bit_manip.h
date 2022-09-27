#pragma once

#define _BV(bit) (1 << (bit))
#define set_bit(register,bit) (register |= _BV(bit))
#define clear_bit(register,bit) (register &= (~_BV(bit)))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

/*In summary, here’s a recap of the three important bit-twiddling operations. Let’s say you’re trying to write
bit i in byte BYTE:
Set a bit
BYTE |= (1 << i);
Clear a bit
BYTE &= ~(1 << i);
Toggle a bit
BYTE ^= (1 << i);
#define BV(bit) (1 << (bit))
#define setBit(byte, bit) (byte |= BV(bit))
#define clearBit(byte, bit) (byte &= ~BV(bit))
#define toggleBit(byte, bit) (byte ^= BV(bit))
#define _BV(bit) (1 << (bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))
you need use library #include avr/io.h
*/



