int my_strlcpy (char *destination, const char *source, int destination_length);

int main (void) {
  
  /* do something here */

  return 0;
}

int my_strlcpy (char *d, const char *s, int d_len) {
  int d_i = 0;

  while (*s != '\0') {
    if (d_i < d_len) {
      *(d + d_i) = *s;
      d_i++;
    }
    s++;
  }
  *(d + d_i) = '\0';
  return d_i;
}

