#!/usr/bin/python
# vim: noet sw=4 ts=4

VERSION	= '@VERSION@'

import	os
import	sys
import	re
import	datetime
import	gzip
import	bz2

class	AnsiColors( object ):

	RESET_ALL      = '\033[0m';
	FOREGROUND_RED = '\033[1;31m';
	FOREGROUND_GREEN = '\033[1;32m';
	FOREGROUND_YELLOW = '\033[1;33m';
	REVERSE_VIDEO  = '\033[7m';

	def	__init__( self ):
		return

	def	reset( self ):
		return AnsiColors.RESET_ALL

	def	emphasis( self ):
		return AnsiColors.FOREGROUND_RED

	def	normal( self ):
		return AnsiColors.FOREGROUND_GREEN

	def	timestamp( self ):
		return AnsiColors.FOREGROUND_YELLOW

	def	reverse( self ):
		return AnsiColors.REVERSE_VIDEO

	def	host_color( self, id ):
		sgr = '\033[1;%dm' % (31 + (id % 6))
		return sgr

class	VlmTool( object ):

	FILTERS =	[
		# Try to keep these in alphabetical order, at least
		r'abnormal',
		r'abo?rt',
		r'access beyond',
		r'asked for',
		r'bcm5709',
		r'blocked for more',
		r'becomes',
		r'bond',
		r'call trace:',
		r'clocksource',
		r'conflict',
		r'crs',
		r'css',
		r'device-mapper',
		r'(de)?bug',
		r'dlm'
		r'disagrees.*symbol.*',
		r'don\'t',
		r'end_request',
		r'error',
		r'fail',
		r'fenc(e,ing)',
		r'fsck',
		r'\bh[au]ng',
		r'illegal:',
		r'init:',
		r'invalid:',
		r'i?scsi',
		r'kill',
		r'link',
		r'linux version',
		r'lpfc',
		r'multipathd',
		r'netdev',
		r'nfs:.*not responding',
		r'nfsd:',
		r'no space left',
		r'not supported',
		r'\bnmi',
		r'o2',
		r'ocfs',
		r'only',
		r'oom',
		r'oops',
		r'out.*of.*memory',
		r'own address',
		r'panic',
		r'please',
		r'page allocation failure',
		r'power states',
		r'qla.*loop',
		r'rds/(ib)?',
		r'read-?only',
		r'register(ed)?',
		r'require',
		r'restart',
		r'segfault',
		r'selinux',
		r'signal',
		r'sysrq.*:',
		r'taint',
		r'terminated',
		r'time-?out',
		r'unable',
		r'unknown symbol.*',
		r'unsupported',
		r'vfs:',
		r'warning',
		r'watchdog',
	]

	def __init__( self, dont_populate = True, mark = False, colorize = False ):
		self.colorize = colorize
		self.accepted = 0
		self.rejected = 0
		self.ignored  = 0
		self.entries  = []
		self.filters  = []
		self.ruleno   = 0		# State for incremental searches
		self.maxrules = 0
		self.mark     = mark
		if not dont_populate:
			self.add_filter_set( VlmTool.FILTERS )
		return

	def add_filter( self, pattern ):
		if self.colorize:
			pattern = r'^(.*)(%s)(.*)$' % pattern
			# print >>sys.stderr, 'pattern %s' % pattern
		mo = re.compile( pattern, re.IGNORECASE )
		self.filters.append( mo )
		self.maxrules += 1
		return	mo

	def show_rule( self, mo ):
		if mo is None:
			return ''
		return mo.re.pattern

	def add_filter_set( self, filters ):
		for filter in filters:
			self.add_filter( filter )
		return

	def filter_matches( self, text, ruleno ):
		matchObject = self.filters[ruleno].search( text )
		if matchObject is not None:
			return matchObject
		return None

	def apply_filters( self, line ):
		for i in xrange( 0, self.maxrules ):
			mo = self.filter_matches( line, self.ruleno )
			if mo is not None:
				return mo
			self.ruleno = (self.ruleno+1) % self.maxrules
		return None

	def	get_parts( self, mo ):
		if mo is None:
			l = '???'
			m = '???'
			r = '???'
		else:
			l = mo.group( 1 )
			m = mo.group( 2 )
			r = mo.group( 3 )
		return (l, m, r)

	def date_to_bin( self, ts ):
		try:
			retval = datetime.datetime.strptime(
				ts,
				'%b %d %H:%M:%S'
			)
		except:
			raise ValueError
		return retval

	def ingest_from( self, fyle ):
		for line in fyle:
			line = line.rstrip()
			if len(line) == 0: continue
			timestamp = line[0:15]
			try:
				ts = self.date_to_bin( timestamp )
			except:
				# Silently ignore badly-formatted line
				self.ignored += 1
				continue
			for i in xrange( 17, len(line) ):
				if line[i].isspace():
					host = line[16:i]
					resid = line[i:]
					break
			mo = self.apply_filters( resid )
			if mo is not None:
				# Rule hits are always accepted
				self.entries.append( (ts, mo, timestamp, host, resid) )
				self.accepted += 1
			else:
				if self.mark:
					# Non-hits are only kept if we are marking or colorizing
					self.entries.append( (ts, mo, timestamp, host, resid) )
				self.rejected += 1
		return

	def ingest( self, fn ):
		if fn.endswith( '.bz2' ):
			openwith = bz2.open
		elif fn.endswith( '.bz' ):
			openwith = gzip.open
		else:
			openwith = open
		try:
			f = openwith( fn, 'rt' )
		except Exception, e:
			print >>sys.stderr, "Cannot open '%s' for injesting." % fn
			raise e
		self.ingest_from( f )
		f.close()
		return

	def dump_stats( self, out = sys.stdout ):
		total = self.accepted + self.rejected + self.ignored
		l = len( '%u' % total )
		u_fmt = '%' + str(l) + 'u %s'
		print >>out, '%s' % '-' * l
		print >>out, u_fmt % ( self.accepted, 'entries accepted' )
		print >>out, u_fmt % ( self.ignored,  'entries ignored' )
		print >>out, u_fmt % ( self.rejected, 'entries rejected' )
		print >>out, '%s' % '=' * l
		print >>out, u_fmt % ( total, 'all entries' ),
		print >>out, '(%.f%% flagged)' % ((self.accepted * 100.0) / total)
		return

	def date_to_bin( self, ts ):
		return datetime.datetime.strptime(
			ts,
			'%b %d %H:%M:%S'
		)

	def sort( self ):
		self.entries.sort( key = lambda (ts,mo,timestamp,host,resid): ts )
		return

	def every( self ):
		for (t,mo,timestamp,host,resid) in self.entries:
			yield (t,mo,timestamp,host,resid)
		return

	def is_marked( self, mo ):
		if mo is None:
			retval = False
		else:
			retval = True
		return retval

	def	list_filter_set( self, out = sys.stdout ):
		for filter in self.filters:
			print >>out, "%s" % filter.pattern
		return

	def	postprocess( self ):
		starters = []
		starters.append(self.add_filter( r'kernel:.*unable to handle kernel' ))
		starters.append(self.add_filter( r'call trace:' ))
		starters.append(self.add_filter( r'kernel: eeek!' ))
		body  = self.add_filter( r'kernel:' )
		i = 0
		n = len( self.entries )
		while i < n:
			(ts, mo, timestamp, host, resid) = self.entries[i]
			for starter in starters:
				mo = starter.search( resid )
				if mo is not None:
					break
			if mo is None:
				i += 1
				continue
			self.entries[ i ] = (ts,mo,timestamp,host,resid)
			# Begin a matching clause
			i += 1
			while i < n:
				(ts, mo, timestamp,host,resid) = self.entries[i]
				mo = body.search( resid )
				if mo is None:
					break
				self.entries[ i ] = (ts,mo,timestamp,host,resid)
				i += 1
		return

if __name__ == '__main__':
	import	optparse
	prog = os.path.basename( sys.argv[0] )
	p = optparse.OptionParser(
		prog = prog,
		version = '%%prog v%s' % VERSION,
		usage = 'usage: %prog [options] [messages..]',
		description = """Read and filter groups of /var/log/messages files.""",
		epilog="""Consult the man(1) page for more information."""
	)
	p.add_option(
		'-a',
		'--add',
		dest='filters',
		metavar='re',
		default = [],
		action='append',
		help = 'Python regular expression'
	)
	p.add_option(
		'-b',
		'--bulk',
		dest='from_here',
		metavar='FILE',
		default = None,
		help = 'Load bulk filters from here, one per line.'
	)
	p.add_option(
		'-c',
		'--colorize',
		dest='colorize',
		default = False,
		action='store_true',
		help = 'Use colors (implies -m).'
	)
	p.add_option(
		'-l',
		'--list',
		dest='list_rules',
		default = False,
		action = 'store_true',
		help = 'Write rule set to stdout.'
	)
	p.add_option(
		'-m',
		'--mark',
		dest='mark',
		default = False,
		action = 'store_true',
		help = 'Output all lines, marking matches.'
	)
	p.add_option(
		'-n',
		'--nofilters',
		dest='no_filters',
		default = False,
		action = 'store_true',
		help = 'Do not populate standard filters.'
	)
	p.add_option(
		'-o',
		'--out',
		dest='ofile',
		metavar='FILE',
		default = None,
		help = 'Write output here, else to stdout.'
	)
	p.add_option(
		'-r',
		'--rules',
		dest='show_rule',
		default = False,
		action = 'store_true',
		help = 'Show matching rule for line (implies -m).'
	)
	p.add_option(
		'-s',
		'--stats',
		dest='show_stats',
		default = False,
		action = 'store_true',
		help = 'Show counts of filter hits.'
	)
	def_thumb = '-'
	p.add_option(
		'-t',
		'--thumb',
		dest='thumb',
		default = def_thumb,
		metavar = 'THUMB',
		help = 'Mark matching lines with this; default="%s".' % def_thumb
	)
	(opts,args) = p.parse_args()
	if opts.ofile is not None:
		out = open( opts.ofile, 'wt' )
	else:
		out = sys.stdout
	opts.mark |= opts.colorize
	vt = VlmTool(
		dont_populate = opts.no_filters,
		mark = opts.mark,
		colorize = opts.colorize
	)
	vt.add_filter_set( opts.filters )
	if( opts.from_here is not None ):
		try:
			f = open( opts.from_here, 'rt' )
		except Exception, e:
			print >>sys.stderr, "Could not read '%s'; bye." % opts.from_here
		bulk = []
		for line in f:
			bulk.append( line.strip() )
		f.close()
		vt.add_filter_set( bulk )
	if( opts.list_rules ):
		vt.list_filter_set()
		sys.exit(0)
	if len(args) == 0:
		for f in os.listdir( '/var/log' ):
			if f.startswith( 'messages' ):
				vt.ingest( os.path.join( '/var/log', f ) )
	else:
		for f in args:
			if f == '-':
				vt.ingest_from( sys.stdin )
			else:
				vt.ingest( f )
	vt.sort()
	thumb = opts.thumb
	nothumb = ' ' * len( thumb )
	if opts.colorize:
		opts.mark = True
		ac = AnsiColors()
	vt.postprocess()
	hosts = {}
	host_number = 1
	for (ts,mo,timestamp,host,resid) in vt.every():
		if opts.mark:
			# Will get both marked and unmarked lines here
			marked = vt.is_marked( mo )
			if marked is False:
				leadin = nothumb
			else:
				leadin = thumb
			print >>out, '%s ' % leadin,
			if opts.show_rule:
				if not marked:
					rule = ''
				else:
					rule = vt.show_rule( mo )
				print >>out, '%-15.15s ' % rule,
			if opts.colorize:
				try:
					color = hosts[ host ]
				except KeyError, e:
					color = host_number
					hosts[ host ] = host_number
					host_number += 1
				host = '%s%s%s' %	(
					ac.host_color(color),
					host,
					ac.reset()
				)
			if marked and opts.colorize:
				l,m,r = vt.get_parts( mo )
				resid = '%s%s%s%s%s' % (
					l,
					ac.emphasis(),
					m,
					ac.reset(),
					r
				)
		print >>out, '%s %s %s' % (timestamp,host,resid)
	if opts.show_stats:
		vt.dump_stats( out )
	sys.exit(0)
