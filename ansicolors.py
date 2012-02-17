#!/usr/bin/python
# vim: noet sw=4 ts=4

import	os
import	sys
import	re
import	datetime

class	AnsiColors( object ):

	RESET_ALL      = '\033[0m';
	FOREGROUND_RED = '\033[1;31m';
	REVERSE_VIDEO  = '\033[7m';

	def	__init__( self ):
		return

	def	reset( self ):
		return AnsiColors.RESET_ALL

	def	emphasis( self ):
		return AnsiColors.FOREGROUND_RED

	def	reverse( self ):
		return AnsiColors.REVERSE_VIDEO

if __name__ == '__main__':
	ac = AnsiColors()
	print '%sabc%sdef%shij' % (
		ac.reset(),
		ac.emphasis(),
		ac.reset(),
	)
