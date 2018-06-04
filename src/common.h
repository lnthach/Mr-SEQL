#ifndef FREQT_COMMON_H
#define FREQT_COMMON_H

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>

template <class Iterator>
static inline unsigned int tokenize (char *str, char *del, Iterator out, unsigned int max)
{
  char *stre = str + strlen (str);
  char *dele = del + strlen (del);
  unsigned int size = 1;

  while (size < max) {
    char *n = std::find_first_of (str, stre, del, dele);
    *n = '\0';
    *out++ = str;
    ++size;
    if (n == stre) break;
    str = n + 1;
  }
  *out++ = str;

  return size;
}




#endif
