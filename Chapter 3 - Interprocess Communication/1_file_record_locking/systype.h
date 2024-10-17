#ifdef  unix              /* true for most UNIX systems, BSD and Sys 5ys5 */
                          /* but not for Xenix */
#define UNIX        1     /* OS Type */

#ifdef  vax               /* true for BSD on a VAX */ /* also true for VAX Sys 5, but we don't have to worry about that (for now) */
#define VAX         1     /* hardware */
#define BSD         1     /* OS Type */
#else /* !vax */
#ifdef  pyr
#define PYRAMID     1     /* hardware */
#define BSD         1     /* OS Type */
#else /* !vax && !pyr */
#ifdef  mc68k             /* assume AT&T UNIX pc, aka 7300 or 3b1 */ /* what about other 68000 unix systems?? */
#define UNIXPC      1     /* hardware */
#define SYS5        1     /* OS Type */
#else /* !vax && !pyr && !mc68k */
#ifdef  i386              /* AT&T System V Release 3.2 on the Intel 80836 */
#define IBMPC       1     /* hardware */
#define SYS5        1     /* OS Type */
#else /* !vax && !pyr && !mc68k && !i386 */
#ifdef accel
#define CELEBRITY   1     /* hardware */
#define BSD         1     /* OS Type */
#else
/* What type of unix system is this */
#endif  /* accel */
#endif  /* i386 */
#endif  /* mc68k */
#endif  /* pyr */
#endif  /* vax */

#endif  /* unix */

#ifdef  M_XENIX           /* true for SCO Xenix */
#define UNIX        1     /* OS Type */
#define XENIX       1     /* OS Type */
#define SYS5        1     /* OS Type */
#define IBMPC       1     /* hardware */
#endif  /* M_XENIX */

#ifdef  MSDOS             /* true for Microsoft C and Lattice, assume former */
#define IBMPC             /* hardware */
#define MICROSOFT         /* C Compiler Type */
#endif  /* MSDOS */

/*
 * Define Replacement names for the BSD names that we use
*/

#ifdef  SYS5
#define rindex      strrchr
#define index       strchr
#define u_char      unchar
#define u_short     ushort
#define u_int       uint
#define u_long      ulong
#endif  /* SYS5 */

#ifdef  MICROSOFT
#define rindex      strrchr
#define index       strchr
#define u_char      uchar
#define u_short     ushort
#define u_int       uint
#define u_long      ulong
#endif  /* MICROSOFT */
