/*
 * Usage:
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Run the `./passwd` executable.
 *      The program will display the information about the current user 
 *      in the `etc/passwd` directory.
 *  ->  To remove the executable, run the `make clean` command.
*/
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

/* 
 * getpwuid checks in the /etc/passwd (might be different on different version, refer to the man page: man 3 getpwuid), which holds each record in the format: 
 *    login-name:encrypted-password:user-ID:group-ID:miscellany:login-directory:shell
 * 
 * getpwnam is similar, but it checks for the login-name which is provided as the argument in the etc/passwd directory.
 *
 * As stated, different machine might have different implementation to search, for instance, the man page states that the function also looks into /etc/master.passwd file. 
*/

/* Structure of passwd struct tag in my machine */
/* 
 * typdef for:
 * __darwin_time_t  = long
 * uid_t            = __uint32_t
 * gid_t            = __uint32_t
*/
/*
  struct passwd {
    char	            *pw_name;		            // user name 
    char	            *pw_passwd;		          // encrypted password
    uid_t	            pw_uid;			            // user uid
    gid_t	            pw_gid;			            // user gid
    __darwin_time_t   pw_change;		          // password change time
    char	            *pw_class;		          // user access class
    char	            *pw_gecos;		          // Honeywell login info
    char	            *pw_dir;		            // home directory
    char	            *pw_shell;		          // default shell
    __darwin_time_t   pw_expire;		          // account expiration 
  };
*/


main () {
  struct passwd *pwd;             /* for getpwuid */
  struct passwd *pwd_by_name;     /* for getpwnam */

  /* Printing the contents obtained from getpwuid() */
  pwd = getpwuid(getuid());
  if (!pwd) {
    printf("[ERROR] Invalid User ID.\n");
  } else {
    printf("[LOG] The size of the struct passwd is: %zu\n", sizeof(*pwd));
    printf("[LOG] User Name: %s\n", pwd->pw_name);
    printf("[LOG] Encrypted Password: %s\n", pwd->pw_passwd);
    printf("[LOG] User UID: %d\n", pwd->pw_uid);
    printf("[LOG] User User GID: %d\n", pwd->pw_gid);
    printf("[LOG] User Password Change time: %ld\n", pwd->pw_change);
    printf("[LOG] User User Access Class: %s\n", pwd->pw_class);
    printf("[LOG] User Honeywell Login Info: %s\n", pwd->pw_gecos);
    printf("[LOG] User Home Directory: %s\n", pwd->pw_dir);
    printf("[LOG] User Default Shell: %s\n", pwd->pw_shell);
    printf("[LOG] User Account Expiration: %ld\n", pwd->pw_expire);
  }


  pwd_by_name = getpwnam("root");
  if (!pwd_by_name) {
    printf("[ERROR] Invalid User Name.\n");
  } else {
    printf("[LOG] User UID: %d\n", pwd_by_name->pw_uid);
  }
}
