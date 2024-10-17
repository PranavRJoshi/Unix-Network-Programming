/*
 * Usage: 
 *  ->  Prepare the executable using the `make` command in the current working directory.
 *  ->  execute `./time_of_day`
 *  ->  To clean, run `make clean`
*/

#include <stdio.h>
#include <sys/time.h>
/* for the times function, used by System V */
#include <sys/times.h>
#include <sys/param.h>

/* Members in the structure of tag timeval */
/*
 *  typedef for:
 *    __darwin_time_t       = long
 *    __darwin_suseconds_t  = int
*/
/*
  struct timeval {
	  __darwin_time_t         tv_sec;         // seconds
	  __darwin_suseconds_t    tv_usec;        // and microseconds
  };
*/

/* Members in the structure of tag tms */
/*
 *  typedef for:
 *    clock_t               = unsigned long
*/
/*
  struct tms {
    clock_t tms_utime;      // [XSI] User CPU time
    clock_t tms_stime;      // [XSI] System CPU time
    clock_t tms_cutime;     // [XSI] Terminated children user CPU time
    clock_t tms_cstime;     // [XSI] Terminated children System CPU time
  };
*/

int main (void) {

  struct timeval  current_time_day;
  struct tms      current_time_day_sys_v;
  clock_t         times_ret;

  if (gettimeofday(&current_time_day, NULL) == 0) {
    printf("The returned time in seconds is: %ld\n", current_time_day.tv_sec);
  }

  times_ret = times(&current_time_day_sys_v);       /* returns the value in CPU's CLK_TCK of a second since Unix epoch time */
  printf("[LOG] The value of times_ret is: %lu\n", times_ret);
  /* returns the amount of processor time used by the calling process, in addition to the current clock time. */
  printf("User CPU time is: %lu\n", current_time_day_sys_v.tms_utime);
  printf("System CPU time is: %lu\n", current_time_day_sys_v.tms_stime);

  return 0;
}
