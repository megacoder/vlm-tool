#ifndef _XPRINTF_H
#  define _XPRINTF_H
#  include <stdarg.h>
#  include <stdio.h>
#  include <gcc-compat.h>
#  define xprintf(l,a)	  if( (l) <= debug ) {printf a;}
#endif /* !_XPRINTF_H */
