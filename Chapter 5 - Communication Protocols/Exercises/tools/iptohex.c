#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

extern int errno;

// NOTE:  My system uses the little endian, i.e., for any integeral value, the lowest byte address stores the least significant 
//        byte and the highest byte address stores the most significant byte. Systems should have the BYTE_ORDER macro defined.
// REFERENCE: https://en.wikipedia.org/wiki/Endianness#/media/File:32bit-Endianess.svg

// 0-1023, transform decimal to bits
unsigned int int_to_int(unsigned int k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * int_to_int(k / 2)));
}

uint32_t htobe32 (uint32_t host_32_bits);

int main (int argc, char **argv) {
  if (argc != 2) {
    perror("Usage: ./iptohex <ip bytes separated by dot>");
    exit(EXIT_FAILURE);
  }
  
  unsigned int byte1, byte2, byte3, byte4;
  byte1 = byte2 = byte3 = byte4 = 0;

  // feature: use the `make test` to enable this. Uses an union whose size is 32 bits (4-bytes) to store the Internet address.
  #ifdef _TEST_FUNCTION
    union compact {
      uint32_t  compact_address;
      uint8_t   bytes_arr[4];
    } dotted_address;
    
    struct in_addr inet_aton_res;

    // `hhu` format specifier for an unsigned char (type definiton for an unisigned int of 8 bits (uint8_t))
    if ((sscanf(argv[1], "%hhu.%hhu.%hhu.%hhu", &dotted_address.bytes_arr[0], &dotted_address.bytes_arr[1], &dotted_address.bytes_arr[2], &dotted_address.bytes_arr[3])) < 4) {
      perror("[DEBUG ERROR] Insufficient Input field provided. format (./test <0-255>.<0-255>.<0-255>.<0-255>)");
    } else {
      // How the union will look after the assignment of 4 uint8_t values. For this example, I will use the value: 192.168.1.255
      // ["1100 0000", "1010 1000", "0000 0001", "1111 1111"] 
      // Now, when we represent this as a 32 bit unsigned integer (in hex), is should be: 0xC0A801FF
      printf("[DEBUG] The 32-bit unsigned representation is: %u (%x in hex).\n", dotted_address.compact_address, dotted_address.compact_address);
    }

    inet_aton(argv[1], &inet_aton_res);
    printf("[DEBUG] inet_aton_res = %x\n", inet_aton_res.s_addr);

    union compact endian_test = { 0 };
    endian_test.compact_address = htobe32(dotted_address.compact_address);
  
    // NOTE: If you use the %d format specifier instead of the %u, it will return the 2's complement of the bits representation.
    printf("[DEBUG] htobe32 = %u (0x%x in hex)\n", htobe32(dotted_address.compact_address), htobe32(dotted_address.compact_address));
    printf("[DEBUG] Storing the result of the function htobe32 in endian_test\n");
    printf("[DEBUG]\nuint8_t[0] = %u\n"      \
           "uint8_t[1] = %u\n"      \
           "uint8_t[2] = %u\n"      \
           "uint8_t[3] = %u\n", endian_test.bytes_arr[0], endian_test.bytes_arr[1], endian_test.bytes_arr[2], endian_test.bytes_arr[3]);
    printf("[DEBUG] The system uses little endian, so we will look into the addresses of the 8-bits unsigned int.\n");
    printf("[DEBUG]\n&uint8_t[0] = %p\n"      \
           "&uint8_t[1] = %p\n"                  \
           "&uint8_t[2] = %p\n"                  \
           "&uint8_t[3] = %p\n", (void *) &(endian_test.bytes_arr[0]), (void *) &endian_test.bytes_arr[1], (void *) &endian_test.bytes_arr[2], (void *) &endian_test.bytes_arr[3]);
    printf("[DEBUG] The unsigned representation is: %u (%x in hex)\n", endian_test.compact_address, endian_test.compact_address);
    printf("[DEBUG] Notice how the lowest byte address is the least significant byte (see in hex value). With this, we know for certain that the system uses the little endian\n");
    printf("[DEBUG] The purpose of illustrating this is, the htobe32() function works as intended, but since we have the little endian notation being used, the output we want is not fetched. Hence, there exists functions like inet_aton().\n");
  #endif

  if ((sscanf(argv[1], "%u.%u.%u.%u", &byte1, &byte2, &byte3, &byte4)) < 4) {
    perror("Insufficient Input field provided. format (./iptohex <0-255>.<0-255>.<0-255>.<0-255>)");
  } else {
    if (byte1 > 255 || byte2 > 255 || byte3 > 255 || byte4 > 255) {
      perror("out_of_bound_error: Decimals must be in the range <0-255>");
      exit(EXIT_FAILURE);
    }
    int bits1, bits2, bits3, bits4;
    bits1 = int_to_int(byte1);
    bits2 = int_to_int(byte2);
    bits3 = int_to_int(byte3);
    bits4 = int_to_int(byte4);

    printf("argv[1] (%s) in hex:  0x%02x%02x%02x%02x\n", argv[1], byte1, byte2, byte3, byte4);
    printf("argv[1] (%s) in bits: %08d.%08d.%08d.%08d\n", argv[1], bits1, bits2, bits3, bits4);

    printf("[LOG] inet representation (big endian) for (%s) is: 0x%02x%02x%02x%02x\n", argv[1], byte4, byte3, byte2, byte1);
  }

  exit(EXIT_SUCCESS);
}

// refer: https://linux.die.net/man/3/endian
// enable _TEST_FUNCTION
uint32_t htobe32 (uint32_t host_32_bits) {
  uint32_t res = 0;
  // refer: https://stackoverflow.com/questions/2182002/how-to-convert-big-endian-to-little-endian-in-c-without-using-library-functions
  /*
   * Let's try to understand all the bitwise "magic" that's happening here. For instance, say that a number: 0xaabbccdd is taken.
   * Note that each hexadecimal digit occupies 4-bytes or 1-nibble. We will first try to look into the result of each 
   * sub-epxression before all of them are bitwise OR'd. Also note that for each bit-shift, the corresponding "empty spot" is filled 
   * with a zero (0) bit.
   *
   * Now, 0xaabbccdd = 0b-1010-1010-1011-1011-1100-1100-1101-1101 (dash (-) added to make it easier to read).
   * 
   *  1. 24-bits shift to right.  Result = 0b-0000-0000-0000-0000-0000-0000-1010-1010
   *  2. 8-bits shift to left.    Result = 0b-1011-1011-1100-1100-1101-1101-0000-0000
   *  3. 8-bits shift to right.   Result = 0b-0000-0000-1010-1010-1011-1011-1100-1100
   *  4. 24-bits shift to left.   Result = 0b-1101-1101-0000-0000-0000-0000-0000-0000
   * 
   * This isn't enough. Notice that we haven't even done the bitwise AND operation yet. Realize one thing before we do it tho.
   * 0xff is equivalent to 0x000000ff, 0xff0000 is equivalent to 0x00ff0000. This is because integral constants are 32-bits.
   *
   *  1. Bitwise AND 0xff.        Result = 0b-0000-0000-0000-0000-0000-0000-1010-1010
   *  2. Bitwise AND 0xff0000.    Result = 0b-0000-0000-1100-1100-0000-0000-0000-0000
   *  3. Bitwise AND 0xff00.      Result = 0b-0000-0000-0000-0000-1011-1011-0000-0000
   *  4. Bitwise AND 0xff000000.  Result = 0b-1101-1101-0000-0000-0000-0000-0000-0000
   *
   * Maybe you have seen the "magic" here. When we bitwise OR the results, we get the final result as:
   *    
   *    res = 0b-1101-1101-1100-1100-1011-1011-1010-1010
   *    res = 0xddccbbaa.
  */
  res = (((host_32_bits >> 24)  & 0xff)       |     //  move byte 3 to byte 0
         ((host_32_bits << 8    & 0xff0000))  |     //  move byte 1 to byte 2
         ((host_32_bits >> 8    & 0xff00))    |     //  move byte 2 to byte 1
         ((host_32_bits << 24   & 0xff000000)));    //  move byte 0 to byte 3

  return res;
}

