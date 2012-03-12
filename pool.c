#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <gcc-compat.h>

#include <pool.h>

#define	POOL_CHUNK_SIZE	(getpagesize() * 4)

static	void *		_inline
xmalloc(
	size_t const	size
)
{
	void * const	retval = malloc( size );

	if( !retval )	{
		perror("out of memory");
		abort();
	}
	return( retval );
}

pool_t *
pool_new(
	size_t		size,		/* Size of one object		 */
	int		(*ctor)(void *), /* Called after alloc		 */
	void		(*dtor)(void *)	/* Called before free		 */
)
{
	pool_t *	retval;

	retval = NULL;
	if( size )	{
		retval        = xmalloc( sizeof( *retval ) );
		retval->qty   = (POOL_CHUNK_SIZE - sizeof(pool_chain_t *)) /
				size;
		retval->size  = size;
		retval->head  = NULL;
		retval->chain = NULL;
		retval->ctor  = ctor;
		retval->dtor  = dtor;
	}
	return( retval );
}

void
pool_free(
	pool_t * *	pp		/* Addr of where handle is kept	 */
)
{
	do	{
		pool_t *	pool;
		pool_chain_t *	chain;
		pool_chain_t *	next;

		/* Ensure we have a pool to work with			 */
		if( !pp || ((pool = *pp) == NULL) )	{
			break;
		}
		/* Free chain segments, if any				 */
		for( chain = pool->head; chain; chain = next )	{
			next = chain->next;
			if( pool->dtor )	{
				size_t		i;

				for( i = 0; i < chain->used; ++i )	{
					if( pool->dtor )	{
						(*pool->dtor)(
							chain->data	+
							(pool->size * i)
						);
					}
				}
			}
			free( chain );
		}
		/* Release pool descriptor				 */
		free( pool );
		/* Remove our tracks					 */
		*pp = NULL;
	} while( 0 );
}

void *
pool_alloc(
	pool_t *	pool		/* Pool of interest		 */
)
{
	void *		retval;

	retval = NULL;
	do	{
		pool_chain_t *	chain = pool->chain;

		/* Make sure we have a chain				 */
		if( !chain || (chain->used >= pool->qty) )	{
			chain = xmalloc( POOL_CHUNK_SIZE );
			/* Put on tail of chain				 */
			if( pool->chain )	{
				pool->chain->next = chain;
			}
			pool->chain = chain;
			/* Remember first link				 */
			if( !pool->head )	{
				pool->head = chain;
			}
			chain->used = 0;
			chain->next = NULL;
		}
		/* Consume an object					 */
		retval = chain->data + (chain->used * pool->size);
		chain->used += 1;
		if( pool->ctor )	{
			(*pool->ctor)( retval );
		}
	} while( 0 );
	return( retval );
}

pool_iter_t *
pool_iter_new(
	pool_t *	pool		/* Pool of interest		 */
)
{
	pool_iter_t *	retval;

	retval = NULL;
	if( pool )	{
		retval        = xmalloc( sizeof(*retval) );
		retval->pool  = pool;
		retval->chain = pool->head;
		retval->pos   = 0;
	}
	return( retval );
}

void
pool_iter_free(
	pool_iter_t * *	pp		/* Addr where handle is kept	 */
)
{
	pool_iter_t *	pit;

	if( pp && (pit = *pp) )	{
		free( pit );
		*pp = NULL;
	}
}

void *
pool_iter_next(
	pool_iter_t *	iter		/* Iterator of interest		 */
)
{
	void *		retval;

	retval = NULL;
	while( iter->chain )	{
		/* If we are within this link, return addr and advance	 */
		if( iter->pos < iter->chain->used )	{
			retval = iter->chain->data + (
				iter->pos * iter->pool->size
			);
			iter->pos += 1;
			break;
		}
		iter->chain = iter->chain->next;
		iter->pos = 0;
	}
	return( retval );
}

pool_iter_t *
pool_iter_dup(
	pool_iter_t *	iter		/* Iterator to be copied	 */
)
{
	pool_iter_t *	retval;

	retval = NULL;
	if( iter )	{
		retval = xmalloc( sizeof(*retval) );
		memcpy( retval, iter, sizeof(*retval) );
	}
	return( retval );
}

int
pool_foreach(
	pool_t *	pool,
	int		(*func)(void *)
)
{
	int		retval;
	pool_iter_t *	iter;

	retval = 0;
	do	{
		void *	data;

		iter = pool_iter_new( pool );
		while( (data = pool_iter_next( iter )) != NULL )	{
			retval = (*func)( data );
			if( retval )	{
				break;
			}
		}
		pool_iter_free( &iter );
	} while( 0 );
	return( retval );
}
