/*
 * Before we write the code fragment present in the text, let's discuss some behavior of `select (2)` based on the argument 
 * provided. 
 *
 * The function declaration is:
 *    
 *    int select (int maxfdpl, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
 *
 * Also, some of the useful macros for working with data type `fd_set` is as follows:
 *
 *    1. FD_ZERO(fd_set *fdset);                // clear all bits in `fdset`
 *    2. FD_SET(int fd, fd_set *fdset);         // turn the bit for `fd` on in `fdset`
 *    3. FD_CLR(int fd, fd_set *fdset);         // turn the bit for `fd` off in `fdset`
 *    4. FD_ISSET(int fd, fd_set *fdset);       // test the bit for `fd` in `fdset`
 * 
 * The `struct timeval` has the members as follows:
 *
 *    struct timeval {
 *      long tv_sec;    // seconds
 *      long tv_usec;   // microseconds
 *    };
 *
 * Read the manual for `select (2)` to learn about the behavior of the call based on various argument. The basic gist is:
 *
 *  1.  Given a pointer to `struct timeval` whose members are set to zero, the call returns immediately after checking the 
 *      descriptors. This is also called **polling**.
 *  2.  Given a pointer to `struct timeval` whose members are set to non-zero, the call will return when one of the specified 
 *      descriptors is ready for I/O, BUT, it won't wait beyond the time specified in the struct.
 *  3.  Given that the pointer to `struct timeval` is supplied as NULL, the call will return only when one of the specified 
 *      descriptors is ready for I/O. The call will wait indefinitely.
*/

/*
 * A side "code fragment" which should not be bothered right now.
 * According to the manual for `select (2)`:
 *
 *    ```
 *    Although the provision of getdtablesize(2) was intended to allow user programs to be written independent 
 *    of the kernel limit on the number of open files, the dimension of a sufficiently large bit field for select 
 *    remains a problem.  The default size FD_SETSIZE (currently 1024) is somewhat smaller than the current kernel 
 *    limit to the number of open files.  However, in order to accommodate programs which might potentially use a 
 *    larger number of open files with select, it is possible to increase this size within a program by providing a
 *    larger definition of FD_SETSIZE before the inclusion of ⟨sys/types.h⟩.
 *    ```
 *
 * The definition for `FD_SETSIZE` is included in: 
 *    
 *    ```
 *    #include <sys/_types/_fd_setsize.h>
 *    ```
 *    
 * Inside the file, the FD_SETSIZE is defined as: 
 *    
 *    ```
 *    #define FD_SETSIZE      __DARWIN_FD_SETSIZE
 *    ```
 * 
 * Now, when the definition for __DARWIN_FD_SETSIZE is in `<sys/_types/_fd_def.h>`, it is as follows:
 *
 *    ```
 *    // #include <sys/_types/_fd_def.h>
 *    #ifdef FD_SETSIZE
 *    #define __DARWIN_FD_SETSIZE     FD_SETSIZE
 *    #else   // !FD_SETSIZE
 *    #define __DARWIN_FD_SETSIZE     1024
 *    #endif  // FD_SETSIZE 
 *    ```
 *
 * This means, if FD_SETSIZE is already defined, then use the already defined value, else set the default value of 1024.
 *
 * One last thing, fd_set is typedef'd as: 
 *
 *    ```
 *    typedef struct fd_set {
 *      __int32_t       fds_bits[__DARWIN_howmany(__DARWIN_FD_SETSIZE, __DARWIN_NFDBITS)];
 *    } fd_set;
 *    ```
 *
 * Looks like the bit field is implementation dependent, so I won't go into much detail. JOKING. Let's look into it.
 *
 * The macros defined are:
 *
 *    ```
 *    #define __DARWIN_NBBY           8                                                       // bits in a byte
 *    #define __DARWIN_NFDBITS        (sizeof(__int32_t) * __DARWIN_NBBY)                     // bits per mask
 *    #define __DARWIN_howmany(x, y)  ((((x) % (y)) == 0) ? ((x) / (y)) : (((x) / (y)) + 1))  // # y's == x bits?
 *    ```
 * 
 * For simplicity, I'm going to assume that __DARWIN_FD_SETSIZE is 1024, unlike the changed 2048 here.
 * The size of an `__int32_t` is 4, and when it is multiplied by __DARWIN_NBBY, which is 8, the result is 32.
 * Now, __DARWIN_howmany() macro looks like: __DARWIN_howmany(1024, 32).
 * The result according to the macro definition will be:
 *    -> 1024 % 32 = 0 (32 * 32 = 1024).
 * Hence, the result of the parameterized macro __DARWIN_howmany will be:
 *    -> 1024 / 32 = 32
 * So, the fd_set will be a struct whose member, `fd_bits`, will be an array of `32` elements of type `__int32_t`.
*/
// #include <sys/select.h>
// #ifdef  FD_SETSIZE
// #undef  FD_SETSIZE
// #define FD_SETSIZE    2048
// #endif
// #include <sys/types.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>   /* could remove this header and just uncomment the function declaration inside main */

int main (int argc, char **argv) {

  // long                    atol();    /* will be dynamically linked during compilation as it's a standard library function */
  static struct timeval   timeout;

  if (argc != 3) {
    // perror("usage: ./select <#seconds> <#microseconds>");
    fprintf(stderr, "usage: ./select <#seconds> <#microseconds>\n");
    return (-1);
  }

  /* See above comment on configuring `select (2)` FD_SETSIZE */
  // fprintf(stderr, "log: The value for FD_SETSIZE is: %d\n", FD_SETSIZE);

  timeout.tv_sec  = atol(argv[1]);
  timeout.tv_usec = atol(argv[2]);

  /*
   * A bit of documentation regarding the first parameter to `select (2)` as it is not described till now.
   * The first parameter, or `maxfdpl`, tells the system to check from descriptor `0` to `maxfdpl - 1` from the 
   * given `fd_set` in the second, third, and fourth parameter. For instance, if we have the following argument 
   * supplied (not in actual format, just for understanding) which is identical to the one shown in text:
   *
   *    readfds   = { 1, 4, 5 }
   *    writefds  = { 2, 7 }
   *    exceptfds = { 1, 4 }
   *
   * And say that `maxfdpl` is given as `5`. This tells the system to check from descriptor `0` to `4` (5 - 1).
   * And yeah, it checks through all three descriptor sets (read, write, and except/error). 
   * Now, readfds's descriptor 1 and 4 will be examined, writefds's descriptor 2 will be examined, and 
   * exceptfds's descriptor 1 and 4 will be examined.
   *
   * Unless the time limit expires, the return from `select (2)` will be the number of descriptors which are 
   * ready. If the time limit was to expire before any of the descriptor was ready, the return value will be 
   * 0. If any form of error was to occur, then -1 will be returned.
  */
  /*
   * Based on the text, looks like the three arguments: `readfds`, `writefds`, and `exceptfds` are value-result argument.
   * Based on `select (2)` manual, after the return from `select`, the three arguments mentioned above will contain a 
   * subset of the file descriptors which are ready for requested operation.
  */
  if ( select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
    perror("select error");
    return (-1);
  }

  return (0);
}

/*
 * Some footnote, cause I saw this paragraph in the text:
 *
 *      ```
 *      The problem the designers of this [select] system call had was how to specify one or more descriptor values for each of 
 *      these three arguments. The decision was made to represent the set of all possible descriptors using an array of integers,
 *      where each bit corresponds to a descriptor. For example, using 32-bit integers, the first element of the array corresponds 
 *      to descriptors 0 through 31, the second element of the array corresponds to descriptor 32 through 63, and so on. All this 
 *      implementation detail is hidden through the `typedef` of the `fd_set` structure and the `FD_XXX` macros.
 *      ```
 *
 * The type definition for `fd_set` is shown in the comment above. Moving on, let's try to understand the hidden implementation of 
 * `FD_XXX` macros. I'll post the code snippets found in my machine.
 *
 *    ```
 *    #define FD_ZERO(p)      __DARWIN_FD_ZERO(p)       // defined in #include <sys/_types/_fd_zero.h>
 *    #define FD_SET(n, p)    __DARWIN_FD_SET(n, p)     // defined in #include <sys/_types/_fd_set.h>
 *    #define FD_CLR(n, p)    __DARWIN_FD_CLR(n, p)     // defined in #include <sys/_types/_fd_clr.h>
 *    #define FD_ISSET(n, p)  __DARWIN_FD_ISSET(n, p)   // defined in #include <sys/_types/_fd_clr.h>
 *    ```
 *
 * All the implementation for __DARWIN_FD_XXX can be found in `#include <sys/_types/_fd_def.h>`.
 * The replacement for the macro and it's definitions are as follows:
 *
 *    ``` For __DARWIN_FD_ZERO
 *    #if __GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3
 *    // Use the built-in bzero function instead of the library version so that
 *    // we do not pollute the namespace or introduce prototype warnings.
 *    #define __DARWIN_FD_ZERO(p)     __builtin_bzero(p, sizeof(*(p))) 
 *    #else 
 *    #define __DARWIN_FD_ZERO(p)     bzero(p, sizeof(*(p))) 
 *    #endif
 *    ```
 *
 *    ``` For remaining replacement macros
 *    #define __DARWIN_FD_SET(n, p)   __darwin_fd_set((n), (p)) 
 *    #define __DARWIN_FD_CLR(n, p)   __darwin_fd_clr((n), (p)) 
 *    #define __DARWIN_FD_ISSET(n, p) __darwin_fd_isset((n), (p))
 *    ```
 *
 * Note that the replacement for `__DARWIN_FD_XXX` macros are function, as we'll see below:
 *
 *    ``` Replacement for macro `__DARWIN_FD_SET`
 *    __header_always_inline void 
 *    __darwin_fd_set(int _fd, struct fd_set *const _p) 
 *    { 
 *      if (__darwin_check_fd_set(_fd, (const void *) _p)) { 
 *        (_p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] |= ((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS))));
 *      }
 *    }
 *    ```
 *
 *    ``` Replacement for macro `__DARWIN_FD_CLR`
 *    __header_always_inline void 
 *    __darwin_fd_clr(int _fd, struct fd_set *const _p) 
 *    { 
 *      if (__darwin_check_fd_set(_fd, (const void *) _p)) { 
 *        (_p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] &= ~((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS)))); 
 *      }
 *    }
 *    ```
 *
 *    ``` Replacement for macro `__DARWIN_FD_ISSET`
 *    // This inline avoids argument side-effect issues with FD_ISSET() 
 *    __header_always_inline int 
 *    __darwin_fd_isset(int _fd, const struct fd_set *_p) 
 *    { 
 *      if (__darwin_check_fd_set(_fd, (const void *) _p)) { 
 *        return _p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] & ((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS))); 
 *      }
 *      return 0; 
 *    } 
 *    ```
 *
 * The function `__darwin_check_fd_set` won't be much of our concern as it's implementation is hidden in system's defined function, This
 * function in turn calls `__darwin_check_fd_set_overflow` (whose function is only declared, not defined) given that the pointer to 
 * `struct fd_set` (provided as function's argument) is not null, else the function returns 1. We won't discuss this further.
 *
 * FD_ZERO isn't much of our concern either. We can see that it calls a form of `bzero` to populate each byte-field of the array with 
 * zero in the supplied array. 
 * Also note that `__DARWIN_NFDBITS` is 32 (as mentioned in above comments).
 * 
 * Due to limited information on `__darwin_check_fd_set`, I will assume that the call to this function will always succeed. Note that 
 * all the replacement functions for macros `__DARWIN_FD_XXX` call this function to verify the fd_set supplied. 
 * One more thing, I won't concern too much about the use of implementation defined `inline` declaration specifier for the functions
 * as the main interest of this documentation is to understand how `FD_XXX` macros work.
 *
 * We will use the file descriptor of `2` as an example for all the functions described below.
 *
 *  ************************************************__DARWIN_FD_SET****************************************************
 *
 * Moving on, first let's see what the `__DARWIN_FD_SET`'s replaced function does. The main thing here is: 
 *
 *    ```
 *    _p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] |= ((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS)))
 *    ```
 * 
 * See the function definition above, simply `_p` of type `struct fd_set` (excluding the defined const type qualifier) and `_fd` is the 
 * file descriptor that is to be added to the set.
 *
 * {expression 1} -> _fd (2) / __DARWIN_NFDBITS (32) = 0
 * {expression 2} -> _fd (2) % __DARWIN_NFDBITS (32) = 2 
 * {expression 3} -> 1 << {expression 2} (2) = 4 
 *
 * So, _p->fd_bits[{expression 1}] = _p->fd_bits[{expression 1}] | (__int32_t) {expression 3}
 *
 * Let's not trouble with explicit casting for now. 
 * Let's assume that variable (_p) of type `fd_set` is an argument for `FD_ZERO`, so all byte-field was 0.
 * The final expression will look like this:
 *
 * _p->fds_bits[0] = _p->fd_bits[0] | 4 
 * _p->fds_bits[0] = 0 | 4   // all byte field with 0 means the number stored there is 0
 * _p->fds_bits[0] = 0b-0000-0000-0000-0000-0000-0000-0000-0000
 *                   0b-0000-0000-0000-0000-0000-0000-0000-0100
 *                 ----------------------------------------------
 *                   0b-0000-0000-0000-0000-0000-0000-0000-0100    (Result from bitwise inclusive OR operation)
 * 
 * Therefore, _p->fds_bits[0] = 4.
 *
 *  ************************************************__DARWIN_FD_SET****************************************************
 *
 *  ************************************************__DARWIN_FD_CLR****************************************************
 *
 * Now, let's look into the replaced function for macro `__DARWIN_FD_CLR`. The main thing this function does is: 
 * 
 *    ```
 *    _p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] &= ~((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS)))
 *    ```
 * Let's see what this expression results in:
 *
 * {expression 1} -> _fd (2) / __DARWIN_NFDBITS (32) = 0
 * {expression 2} -> _fd (2) % __DARWIN_NFDBITS (32) = 2
 * {expression 3} -> 1 << {expression 2} (2) = 4
 * {expression 4} -> (__int32_t) 4 = 0b-0000-0000-0000-0000-0000-0000-0000-0100
 * {expression 5} -> ~{expression 4} (4) = 0b-1111-1111-1111-1111-1111-1111-1111-1011
 *
 * So, _p->fds_bits[{expression 1}] = _p->fds_bits[{expression 1}] & {expression 5}
 *
 * The final expression will look like this: 
 *
 * _p->fds_bits[0] = _p->fds_bits[0] & -5 (2's complement of {expression 5})
 *
 * I know working with 2's complement and other stuff can be convoluting, So I'll stay with the binary representation. 
 * But a little side note. The highest order bit in a singed type (signed int, signed long, etc...) represent the sign bit.
 * When this bit is set (1), 2's complement of the number is done to represent it in decimal.
 * 2's complement just tells to flip the bits and and 1 to the result. Hence, for above value.
 *
 *    {expression 5}  -> 0b-1111-1111-1111-1111-1111-1111-1111-1011
 *    bits flipped    -> 0b-0000-0000-0000-0000-0000-0000-0000-0100
 *    add 1           -> 0b-0000-0000-0000-0000-0000-0000-0000-0101
 *
 * A negative sign is prepended before the number to signify that 2's complement has taken place. Hence the `-5` as result.
 *
 * Realize that I've previously set the file descriptor (using __DARWIN_FD_SET), hence `_p->fds_bits[0]` is still 4.
 *
 * _p->fds_bits[0] = 0b-0000-0000-0000-0000-0000-0000-0000-0100
 *                   0b-1111-1111-1111-1111-1111-1111-1111-1011
 *                  ---------------------------------------------
 *                   0b-0000-0000-0000-0000-0000-0000-0000-0000   (Result from bitwise AND operation)
 *
 * This means that `_p->fds_bits[0]` will now store 0 (all bits are 0, duh). The requested file descriptor has been cleared.
 *
 *  ************************************************__DARWIN_FD_CLR****************************************************
 *
 *  ************************************************__DARWIN_FD_ISSET****************************************************
 *
 * Finally, let's see what the function substition for macro `__DARWIN_FD_ISSET` does. Let's see the actual expression of our concern.
 *
 *    ```
 *    _p->fds_bits[(unsigned long)_fd / __DARWIN_NFDBITS] & ((__int32_t)(((unsigned long)1) << ((unsigned long)_fd % __DARWIN_NFDBITS)))
 *    ```
 *
 * Like before, let's break down the important bits from the expression. NOTE that the result from this expression is returned, not 
 * assigned like in previous functions.
 *
 * {expression 1} -> _fd (2) / __DARWIN_NFDBITS (32) = 0
 * {expression 2} -> _fd (2) % __DARWIN_NFDBITS (32) = 2
 * {expression 3} -> 1 << {expression 2} (2) = 4
 * {expression 4} -> (__int32_t) {expression 3} = 0b-0000-0000-0000-0000-0000-0000-0000-0100
 * {expression 5} -> _p->fds_bits[{expression 1} (0)] = 0 (recall that I have cleared it just recently)
 *
 * Now, the final expression will look like this: 
 *    
 *    _p->fds_bits[0] & 0b-0000-0000-0000-0000-0000-0000-0000-0100
 *    
 *    returns           0b-0000-0000-0000-0000-0000-0000-0000-0000    (_p->fds_bits[0])
 *                      0b-0000-0000-0000-0000-0000-0000-0000-0100    (expression 4)
 *                    ----------------------------------------------
 *                      0b-0000-0000-0000-0000-0000-0000-0000-0000    (Result from bitwise AND operation)
 *
 * Notice that it returns 0, which means that the given file descriptor was not set (1) in the given set of descriptors (struct fd_set).
 *
 * Let's also see what the return would look like if the descriptor wasn't cleared recently. I know it will be 4, but let's do it!
 *
 *    _p->fds_bits[0] will be: 0b-0000-0000-0000-0000-0000-0000-0000-01000
 *    expression 4 remains unchanged, as it depends on _fd (in expression 2).
 *
 * So, it:
 *    
 *    returns           0b-0000-0000-0000-0000-0000-0000-0000-0100    (_p->fds_bits[0], not cleared previously)
 *                      0b-0000-0000-0000-0000-0000-0000-0000-0100    (expression 4)
 *                    ----------------------------------------------
 *                      0b-0000-0000-0000-0000-0000-0000-0000-0100    (Result from bitwise AND operation)
 *
 * As expected, the return from `__DARWIN_FD_ISSET` will be 4, which is non-zero, indicating that the provided file descriptor is 
 * set (1) in the given file descriptor set (struct fd_set).
 *
 *  ************************************************__DARWIN_FD_ISSET****************************************************
*/
