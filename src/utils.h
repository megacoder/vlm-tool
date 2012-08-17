#ifndef	_UTILS_H
#define	_UTILS_H

#include <sys/types.h>
#include <vlm-tool.h>
#include <x-funcs.h>
#include <xprintf.h>

#include <stanzas.h>

#define	NUM_MATCHES	(10)

void	trigger_compile( char const * const, trigger_t * const );

int	trigger_match( char const * const, trigger_t * const, regmatch_t * );

#endif	/* _UTILS_H */
