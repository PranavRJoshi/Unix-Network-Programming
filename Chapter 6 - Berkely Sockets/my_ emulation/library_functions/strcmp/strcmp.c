int my_strcmp (const char *string_1, const char *string_2);

int main (void) {
  
  /* do something here */

  return 0;
}

int my_strcmp (const char *str_1, const char *str_2) {
  for (; *str_1 == *str_2; str_1++, str_2++) {
    if (*str_1 == '\0') {
      return 0;
    }
  }
  return *str_1 - *str_2;
}
