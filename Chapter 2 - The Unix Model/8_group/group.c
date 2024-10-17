/*
 * Usage: 
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Run the executable `./group`.
 *      It will display the information of the user group id structure
 *      found in `etc/group`.
 *  ->  To remove the executable, run the `make clean` command.
*/

#include <stdio.h>
#include <grp.h>
#include <unistd.h>

/* 
 * getgrgid checks information from the /etc/group file which keeps records in the format as:
 *    group_name:encrypted_password:group-ID:user-list
 *
 * getgrnam checks information from the /etc/group given that the file contains the name that is provided as the argument for the function.
 *
 * refer to the man page: man 3 getgrgid, man 3 getgrnam to checks which file is scanned to get the group name.
*/

/* Structure of the group tag structure on my machine */
/* 
 * typedef for:
 * gid_t        = __uint32_t
*/
/*
  struct group {
    char	*gr_name;		  // [XBD] group name
    char	*gr_passwd;		// [???] group password
    gid_t	gr_gid;			  // [XBD] group id
    char	**gr_mem;		  // [XBD] group members
  };
*/

main () {
  
  struct group *grp, *e_grp, *grp_by_name;
  int group_count;
  grp = getgrgid(getgid());
  if (!grp) {
    printf("[ERROR] Invalid Effective Group ID.\n");
  } else {
    printf("[LOG] The information regarding the getgrgid for real group ID is: \n");
    printf("[LOG] Group Name: %s\n", grp->gr_name);
    printf("[LOG] Group Password: %s\n", grp->gr_passwd);
    printf("[LOG] Group ID: %d\n", grp->gr_gid);
    printf("[LOG] Group members: \n");

    for (group_count = 0; grp->gr_mem[group_count] != (char *) 0; group_count++) {
      printf("[MEM LOG] %d. %s\n", group_count + 1, grp->gr_mem[group_count]);
    }
  }

  /*
    e_grp = getgrgid(getegid());
    if (!e_grp) {
      printf("[ERROR] Invalid Group ID.\n");
    } else {
      printf("[LOG] The information regarding the getgrgid for effective group ID is: \n");
      printf("[LOG] Group Name: %s\n", e_grp->gr_name);
      printf("[LOG] Group Password: %s\n", e_grp->gr_passwd);
      printf("[LOG] Group ID: %d\n", e_grp->gr_gid);
      printf("[LOG] Group members: \n");

      for (group_count = 0; e_grp->gr_mem[group_count] != (char *) 0; group_count++) {
        printf("[MEM LOG] %d. %s\n", group_count + 1, e_grp->gr_mem[group_count]);
      }
    }
  */

  grp_by_name = getgrnam("staff");
  if (!grp_by_name) {
    printf("[ERROR] Invalid Group Name.\n");
  } else {
    printf("[LOG] The information regarding the getgrnam for group name staff is: \n");
    printf("[LOG] Group Name: %s\n", grp->gr_name);
    printf("[LOG] Group Password: %s\n", grp->gr_passwd);
    printf("[LOG] Group ID: %d\n", grp->gr_gid);
    printf("[LOG] Group members: \n");

    for (group_count = 0; grp->gr_mem[group_count] != (char *) 0; group_count++) {
      printf("[MEM LOG] %d. %s\n", group_count + 1, grp->gr_mem[group_count]);
    }
  }
}
