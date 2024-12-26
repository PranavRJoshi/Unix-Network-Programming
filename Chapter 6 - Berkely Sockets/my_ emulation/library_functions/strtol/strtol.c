// The function prototype of strtol, which is defined in stdlib.h is as follows:
//    
//    long strtol(const char *restrict str, char **restrict endptr, int base);
//
// According to `strtol (3)`, the function works as follows:
//
//      ```
//      The strtol() function converts the string in str to a long value. 
//      
//      The conversion is done according to the given base, which must be between 2 and 36 inclusive, or be the special value 0.
//      
//      The string may begin with an arbitrary amount of white space (as determined by isspace(3)) followed by a single optional 
//      ‘+’ or ‘-` sign. If base is zero or 16, the string may then include a “0x” prefix, and the number will be read in base 16; 
//      otherwise, a zero base is taken as 10 (decimal) unless the next character is ‘0’, in which case it is taken as 8 (octal).
//
//      The remainder of the string is converted to a long, stopping at the first character which is not a valid digit in the given 
//      base. (In bases above 10, the letter ‘A’ in either upper or lower case represents 10, ‘B’ represents 11, and so forth, with 
//      ‘Z’ representing 35.)
//
//      If endptr is not NULL, strtol() stores the address of the first invalid character in *endptr.  If there were no digits at all,
//      however, strtol() stores the original value of str in *endptr.  (Thus, if *str is not ‘\0’ but **endptr is ‘\0’ on return, 
//      the entire string was valid.)
//
//      The strtol() function return the result of the conversion, unless the value would underflow or overflow.  If no conversion 
//      could be performed, 0 is returned and the global variable errno is set to EINVAL (the last feature is not portable across 
//      all platforms).  If an overflow or underflow occurs, errno is set to ERANGE and the function return value is clamped according 
//      to the following table.
//
//          Function         underflow         overflow
//          strtol()         LONG_MIN          LONG_MAX
//      ```

// For now, I'll only focus on four bases: binary, octal, decimal, and hexadecimal.

#include <sys/errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>   /* for LONG_MIN and LONG_MAX */

extern int errno;

long my_strtol (const char *restrict str, char **restrict endptr, int base);
long power (int number, int exponent);

int main (int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: ./strtol <number-in-string>\n");
    exit(EXIT_FAILURE);
  }

  int base    = 0;
  long result = 0L;

  int arglen = 0;
  char *argptr = argv[1];

  while (*argptr++) {
    arglen++;
  }

  fprintf(stdout, "%s log: the provided argument is of length: %d\n", argv[0], arglen);

  // char test[100]  = {0};    /* could also use the strlen to determine `argv[1]` length and dynamically allocate sufficient memory */
  char *test = malloc(sizeof(arglen + 1));
  if (test == (char *) 0) {
    fprintf(stderr, "%s malloc error: failed to allocate sufficient memory for holding the string.\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  char *testptr   = test;

  // fprintf(stdout, "LONG_MIN: %ld\n", LONG_MIN);
  // fprintf(stdout, "LONG_MAX: %ld\n", LONG_MAX);

  fprintf(stdout, "Enter the base for the provided C-string format number: ");
  if (scanf("%d", &base) == 1) {
    result = my_strtol(argv[1], &testptr, 0);
    if (errno == EINVAL) {
      perror("cannot convert the given string to long integer");
    } else if (errno == ERANGE) {
      perror("provided number exceeds the 64-bit representation of a number");
    } else {
      fprintf(stdout, "%s log: The provided number in decimal is: %ld\n", argv[0], result);
      if (!(*testptr)) {    /* if ( !((char) 0) ), or check if testptr points to null character. */
        fprintf(stdout, "%s log: given string is fully valid.\n", argv[0]);
      } else {              /* testptr contains the `unread` characters. */
        fprintf(stdout, "%s log: given string is not fully valid. Stopped reading since: %s\n", argv[0], testptr);
      }
    }
  } else {
    fprintf(stderr, "%s error: failed to provide a vaild base [2-36].\n", argv[0]);
  }

  free(test);
  test = NULL;

  exit(EXIT_SUCCESS);
}

/*
 *  my_strtol:  Attempt to emulate the library function `strtol` which converts the string to long value.
 *  NOTE: This function uses some "ASCII magics" used to map the alphabet to it's proper numeric representation. It will
 *  not work for other locales.
 *  NOTE: Fails to convert "0" given in base 0. It would only check if the number is in octal or not, and return with EINVAL.
 *  Similar behavior for "0x" or "0X" given in base 0.
 *  
 *  How this works:
 *    - Checks for any leading whitespaces, and if any found, removes it.
 *    - Checks for any optional unary `+` or `-` symbol, and returns based on the sign.
 *    - Based on the `base` provided, the following thing can happen:
 *        1.  If the `base` provided is 0, then the string is checked. If 0 is the first character, it is taken as octal,
 *            and if the first character is 0 AND the second character is x or X, then it is taken as a hexadecimal 
 *            number. Else, it is taken as a decimal number (given that the string contains number from 1 to 9).
 *        2.  If the `base` is explicitly provided, then for bases 8 and 16, explicit checking for the prefix mentioned 
 *            above is done. For instance, if the provided base is 16, the function checks if the string contains the 
 *            prefix `0x` or `0X`, and if found, removes it, as it is just an indicator, not a part of the number.
 *        3.  If the `base` provided is 2, only numbers 1 or 0 is accepted. This goes for all the bases ranging from 2 to 36
 *            inclusive. If the base is, say, 11, then digits 0-9 AND character 'a' will be taken as valid number.
 *    - Can prematurely return if the base is 0, and no valid character was found in the initial position as well as prefixes.
 *      If this is to occur, sets EINVAL and returns 0.
 *    - Also returns EINVAL if the provided base is one which is out of range. <= 1 (excluding 0) and > 36. returns 0.
 *    - Once the function reaches `checkstr` label, the task carried out is to parse the string and extract upto the last 
 *      valid character. If all the characters were found to be valid in the string, the `endptr` parameter is set to point to 
 *      the null character, but if the string contained invalid characters, then the `endptr` will point to the string which 
 *      points to the first invalid character and characters following it. So for instance, if the supplied string is "123abcd1", 
 *      and the provided base is 11, then the endptr will point to the string whose first character is `b`, and the string 
 *      is "bcd1". This indicates that even if there exists valid character, it won't be read as the invalid character preceeded it.
 *    - After fetching all the valid characters, the calculation takes place. The general formulae for converting a number from any 
 *      base to base 10 is:
 *          1. Start from the rightmost digit. Also initialize result as 0. Initialize the exponent as 0 as well.
 *          2. For the given digit, use the formula: `result = result [+|-] (base ^ exponent) * digit`
 *          3. Increment the exponent after each digit.
 *
 *      The result depends on whether any explicit unary `-` was given. If `-` is present in the string, the - from [+|-] is used 
 *      to decrement the number, and + is used by default.
 *    - The result is stored in another variable `prev_res` as well. This is done to verify if any form of overflow or underflow 
 *      has occurred. After each iteration and assignment to the variable `result`, the following scenario would cause underflow 
 *      or overflow:
 *          1.  For the unary + calculation, if the `prev_res` is greater than new assignment to `result`, we know that overflow has
 *              occurred. When `result` goes beyond LONG_MAX, then the value is "reset", and negative number is obtained.
 *          2.  For the unary - calculation, if the `prev_res` is less than new assignment to `result`, we know that underflow has 
 *              occurred. When `result` goes beyond LONG_MIN, then the value is "reset", and positive number is obtained.
 *    - If any form of underflow or overflow is observed, the function returns 0, with errno set to ERANGE. Note that based on 
 *      my understanding, the library function `strtol` returns "clamped" value--LONG_MIN for underflow and LONG_MAX for overflow.
 *      I could have also done that, but for now, felt like returning 0 and setting errno would suffice.
 *    - Returns the actual result if it passed all the checks above.
*/
long my_strtol (const char *restrict str, char **restrict endptr, int base) {
  /* check for leading whitespaces, if any */
  while (*str == ' ') {
    str++;
  }

  int     valid_alphabet_to_digit = 0;    /* used if base > 0 or base == 0 but is a hexadecimal character */
  int     digit                   = 0;    /* used as a digit placeholder for the respective character */
  int     exponent                = 0;    /* exponent tracker, used when calculating the `result`, incremented after each iteration */
  int     sign                    = 0;    /* sign bit, 1 if minus is given, 0 if + or by default */
  int     i                       = 0;    /* used for looping through string up until the last valid character. also acts as index */
  long    result                  = 0;    /* hold the result from the digit-wise calculation */
  long    base_n_power            = 0;    /* hold the base ^ exponent result. Not completely required. */
  long    prev_res                = 0;    /* holds the previous result, used to check if any form of underflow or overflow occurred */

  if (*str == '+') {
    sign = 0;
    str++;
  } else if (*str == '-') {
    sign = 1;
    str++;
  }

  /* check for case when user supplies base as 16 or 8, and also contains 0x/0X or 0 prefix. */
  if (base == 8) {
    if (*str == '0') {    /* explicit adding of 0 for octal number */
      str++;
    }
  }

  if (base == 16) {
    if (*str == '0' && (*(str+1) == 'x' || *(str + 1) == 'X')) {
      str = str + 2;
    }
  }

  /* 
   * If the given base is 0, determine the base according to the provided string. 
   *  
   *    - contains 0x or 0X:  Hexadecimal
   *    - contains 0:         Octal
   *    - contains 1-9:       Decimal
   * 
   * If the given base is 2, only check if the first character is either 1 or 0, else it is not a valid binary.
  */
  if (base == 0) {
    if (*str == '0') {      /* could represent Octal OR Hexadecimal */
      str++;    /* advance to next character to check if it is hexadecimal instead of octal. even it isn't, it will point to next possible digit (for an octal number) in character form */
      if (*str == 'x' || *str == 'X') {
        str++;
        base = 16;      /* change the function parameter's value only */
        goto checkstr;
      }
      base = 8;
      goto checkstr;
    } else if (*str >= '1' && *str <= '9') {    /* Check if decimal */
      base = 10;
      goto checkstr;
    } else {      /* not a valid "number" */
      errno = EINVAL;     /* invalid argument, return 0 as no conversion could be performed. */
      if (endptr != (char **) 0) {    /* could also use (char *) 0, did the other one to make the compiler happy */
        *endptr = str;
      }
      return (0L);
    }
  } else if (base == 2) {
    if (*str == '1' || *str == '0') {
      goto checkstr;
    } else {
      errno = EINVAL;
      if (endptr != (char **) 0) {
        *endptr = str;
      }
      return 0L;
    }
  }
  
  /* base, if zero, is modified above based on the string provided. If base is not modified, it means the input was invalid. */
  if (base <= 1 || base > 36) {   /* if base <= 1 or base > 36, don't care about the string, just set EINVAL and return. */
    errno = EINVAL;
    return (0);
  }


checkstr:
  /* set up the max array index that can be accessed by the respective alphabet. */
  if (base > 10) {
    valid_alphabet_to_digit = base - 10;    /* number of valid character which can be represented numerically */
  }

  /* 
   * determine the string length. Must be a valid C-string. 
   * Notice how str is not incremented by itself, but an index is used to check the characters throughout the 
   * string. This makes sure that we can use this index later to fetch all the *possible* character that are present 
   * in the string.
  */
  for (i = 0; *(str + i); i++) {
    if (base > 10) {
      if (isalpha(*(str + i)) && ((tolower(*(str + i))) - 'a') >= valid_alphabet_to_digit) {
        /* terminate, the given character is not valid in the provided base. */
        break;
      }
    } else if (base <= 10 && base != 0) {    /* base <= 10 */
      if (isalpha(*(str + i))) {
        /* found an alphabet where the given base is <= 10. terminate and find the digit, if any. */
        break;
      } else {
        if (*(str + i) - '0' > (base - 1)) {
          /* 
           * example: for base of 8 and given character is `9`, then '9' - '0' = 9 > (8 - 1). terminate. 
           * also, for base 8 and given character is `8`, then `8` - `0` = 8 > (8 - 1). terminate.
          */
          break;
        }
      }
    }
  }
  /* 
   * By the time the loop terminates or breaks, `i` will be the index which points to either null character of invalid character 
  */

  fprintf(stdout, "[%s LOG]: valid characters read are: %d\n", __func__, i);

  if (i == 0) {   /* failed to get any valid character for the respective base. */
    if (endptr != (char **) 0) {
      *endptr = str;
    }
    errno = EINVAL;
    return (0L);
  }

  /*
   * The reason we did not instantly decrement `i` above is the fact that the code below checks if the index `i`
   * points to a null character or not. If yes, all the characters were valid, and if no, then point to the 
   * first invalid character and the characters following it.
  */
  if (endptr != (char **) 0) {
    if (!(*(str + i))) {        /* if pointer to (str + i) is a null character, we know that the provided string was "valid" */
      **endptr  = '\0';
    } else {
      *endptr   = (str + i);    /* point to the first non-valid character */
    }
  }

  fprintf(stdout, "[%s LOG]: i has the value of: %d and the char str is pointing to is %c\n", __func__, i, *str);
  fprintf(stdout, "[%s LOG]: %s is the string that is not read as it is not valid.\n", __func__, *endptr);

  /* 
   * Even if all the characters in the strings are valid, `i` will point to the null character, so revert back the index by 1.
   * Realize that `for` loop only terminates if the character is a null character, else it breaks on conditions mentioned.
   * If we reached here by `break`ing the loop, we know the index `i` makes the string `str` point to invalid character, so 
   * revert back again.
  */
  i--;

  /*
   * `i` here is an index, so we need to use it up until it is 0. Realize that string `str` was made to point to the 
   * first valid character. This is true for bases in range: [2, 36]. For base as 0, the prefix of the `str` is 
   * "removed" to determine the base of the number, but after determination, `str` will point to the first "possible"
   * valid character.
   *
   * Also realize that the function uses basic ascii magic to determine the numeric value of alphabets. This means this 
   * program won't work for other locales apart from ascii.
   *
   * We first check if the base being used requires numeric representation of alphabets, but one could also loop first and 
   * check for base and numeric representation of alphabets used. I prefer the former one as it seems to be somewhat 
   * efficient. The only difference between base > 10 and base <= 10 is the checking of alpahbet character in the string.
   * Apart from that, everything is the same. I thought of making a function to do this operation, but I don't want to call 
   * multuple functions and make a mess of the process stack.
  */
  if (base > 10) {      /* base > 10 */
    while (i >= 0) {
      if (isalpha(*(str + i))) {      /* if the provided character is an alphabet, could be both uppercase or lowercase */
        digit = tolower(*(str + i)) - 'a' + 10;   /* Change alphabet to appropriate numeric representation */
      } else {                        /* not alphabet, must be numeric digit [0-9]. */
        digit = *(str + i) - '0';                 /* convert a digit character to numeric digit form. */
      }
      base_n_power = power(base, exponent);   /* base ^ exponent. exponent is initially 0, incremented after each iteration */
      if (sign) {       /* if explicit unary `-` was present in the string */
        result = result - base_n_power * digit;
        if (result > prev_res) {      /* underflow has occurred, report and return 0 */
          fprintf(stderr, "[%s WARN]: result underflow detected, function returns 0 instead of \"clamped\" result.\n", __func__);
          errno = ERANGE;
          return (0L);                /* we could also `return (LONG_MIN);` */
        }
      } else {          /* could be explicit `+` character in string, or if none (default) */
        result = result + base_n_power * digit;
        if (result < prev_res) {      /* overflow has occurred, report and return 0 */
          fprintf(stderr, "[%s WARN]: result overflow detected, function returns 0 instead of \"clamped\" result.\n", __func__);
          errno = ERANGE;
          return (0L);                /* we could also `return (LONG_MAX);` */
        }
      }
      fprintf(stdout, "[%s LOG]: result = %ld and digit = %d (%c in char)\n", __func__, result, digit, *(str + i));
      exponent++;     /* increment exponent before using it for the next digit. */
      i--;            /* decrease i so that *(str + i) will point to the character to the left. */
      prev_res = result;    /* store previous result to check for any underflow/overflow scenario */
    }
  } else {              /* base <= 10 */    /* same logic as above, except for alphabetic characters */
    while (i >= 0) {
      digit = *(str + i) - '0';
      base_n_power = power(base, exponent);
      if (sign) {
        result = result - base_n_power * digit;
        if (result > prev_res) {      /* underflow has occurred, report and return 0 */
          fprintf(stderr, "[%s WARN]: result underflow detected, function returns 0 instead of \"clamped\" result.\n", __func__);
          errno = ERANGE;
          return (0L);                /* we could also `return (LONG_MIN);` */
        }
      } else {
        result = result + base_n_power * digit;
        if (result < prev_res) {      /* underflow has occurred, report and return 0 */
          fprintf(stderr, "[%s WARN]: result overflow detected, function returns 0 instead of \"clamped\" result.\n", __func__);
          errno = ERANGE;
          return (0L);                /* we could also `return (LONG_MAX);` */
        }
      }
      fprintf(stdout, "[%s LOG]: result = %ld and digit = %d (%c in char)\n", __func__, result, digit, *(str + i));
      exponent++;
      i--;
      prev_res = result;
    }
  }

  return (result);
}

long power (int number, int exponent) {
  long res = 1;

  while (exponent-- != 0) {
    res = res * number;
  }

  return (res);
}
