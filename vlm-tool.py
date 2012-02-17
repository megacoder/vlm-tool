#!/usr/bin/python
# vim: noet sw=4 ts=4

import	os
import	sys
import	re
import	datetime

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
		r'dlm'
		r'don\'t',
		r'end_request',
		r'error',
		r'fail',
		r'\bh[au]ng',
		r'illegal:',
		r'init:',
		r'invalid:',
		r'kill',
		r'link',
		r'linux version',
		r'lpfc',
		r'multipathd',
		r'netdev',
		r'nfs:.*not responding',
		r'nfsd:',
		r'not supported',
		r'nmi',
		r'o2',
		r'ocfs',
		r'only',
		r'oops',
		r'own address',
		r'please',
		r'qla.*loop',
		r'rds/',
		r'register',
		r'require',
		r'restart',
		r'scsi',
		r'segfault',
		r'selinux',
		r'signal',
		r'sysrq.*:',
		r'taint',
		r'terminated',
		r'time.*out',
		r'unable',
		r'unsupported',
		r'vfs:',
		r'warning',
		r'watchdog',
	]

	def __init__( self, dont_populate = True, mark = False ):
		self.accepted = 0
		self.rejected = 0
		self.lines = []
		self.filters = []
		self.ruleno = 0
		self.maxrules = 0
		self.rules = []
		self.mark = mark
		if not dont_populate:
			self.add_filter_set( VlmTool.FILTERS )
		return

	def add_filter( self, pattern ):
		self.rules.append( pattern )
		self.filters.append(
			re.compile( pattern, re.IGNORECASE )
		)
		self.maxrules += 1
		return

	def show_rule( self, ruleno ):
		if ruleno == -1:
			return ''
		if ruleno < self.maxrules:
			return self.rules[ ruleno ]
		return 'Unknown rule %d' % ruleno

	def add_filter_set( self, filters ):
		for filter in filters:
			self.add_filter( filter )
		return

	def filter_matches( self, text, ruleno ):
		if( self.filters[ruleno].search( text ) ) is not None:
			return True
		return False

	def apply_filters( self, line ):
		for i in xrange( 0, self.maxrules ):
			if self.filter_matches( line, self.ruleno ):
				return self.ruleno
			self.ruleno = (self.ruleno+1) % self.maxrules
		return -1

	def date_to_bin( self, ts ):
		return datetime.datetime.strptime(
			ts,
			'%b %d %H:%M:%S'
		)

	def ingest( self, fn ):
		try:
			f = open( fn, 'rt' )
		except Exception, e:
			print >>sys.stderr, "Cannot open '%s' for reading." % fn
			raise e
		for line in f:
			line = line.rstrip()
			ts = self.date_to_bin( line[0:14] )
			n = self.apply_filters( line )
			if n > -1:
				self.lines.append( (ts, n, line) )
				self.accepted += 1
			else:
				if self.mark:
					self.lines.append( (ts, n, line) )
				self.rejected += 1
		f.close()
		return

	def dump_stats( self, out = sys.stdout ):
		total = self.accepted + self.rejected
		l = len( '%u' % total )
		u_fmt = '%' + str(l) + 'u %s'
		print >>out, '%s' % '-' * l
		print >>out, u_fmt % ( self.accepted, 'lines accepted' )
		print >>out, u_fmt % ( self.rejected, 'lines rejected' )
		print >>out, '%s' % '=' * l
		print >>out, u_fmt % ( total, 'all lines' ),
		print >>out, '(%.f%% flagged)' % ((self.accepted * 100.0) / total)
		return

	def date_to_bin( self, ts ):
		return datetime.datetime.strptime(
			ts,
			'%b %d %H:%M:%S'
		)

	def sort( self ):
		self.lines.sort( key = lambda (t,n,r): t )
		return

	def every( self ):
		for (t,n,r) in self.lines:
			yield (t,n,r)
		return

	def is_marked( self, n ):
		return n > -1

	def	dump_filter_set( self, fn ):
		try:
			f = open( fn, 'wt' )
		except Exception, e:
			print >>sys.stderr, "File '%s' not writable." % fn
			raise e
		set = self.rules
		set.sort()
		# print >>f, 'FILTERS = ['
		for i in xrange( 0, self.maxrules ):
			# print >>f, "\tr'%s'," % set[ i ]
			print >>f, "%s" % set[ i ]
		# print >>f, ']'
		f.close()
		return

if __name__ == '__main__':
	import	optparse
	prog = os.path.basename( sys.argv[0] )
	p = optparse.OptionParser(
		prog = prog,
		usage = 'usage: %prog [options] [messages..]',
		description = """Read and filter groups of /var/log/messages files."""
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
		'-o',
		'--out',
		dest='ofile',
		metavar='FILE',
		default = None,
		help = 'Write output here, else to stdout.'
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
		'-r',
		'--rules',
		dest='show_rule',
		default = False,
		action = 'store_true',
		help = 'Show matching rule for line.'
	)
	p.add_option(
		'-s',
		'--stats',
		dest='show_stats',
		default = False,
		action = 'store_true',
		help = 'Show counts of filter hits.'
	)
	p.add_option(
		'-d',
		'--dump',
		dest='dump_rules',
		default = None,
		metavar = 'FILE',
		help = 'Write rule set to FILE.'
	)
	(opts,args) = p.parse_args()
	if opts.ofile is not None:
		out = open( opts.ofile, 'wt' )
	else:
		out = sys.stdout
	vt = VlmTool(
		dont_populate = opts.no_filters,
		mark = opts.mark
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
	if( opts.dump_rules is not None ):
		vt.dump_filter_set( opts.dump_rules )
	if len(args) == 0:
		for f in os.listdir( '/var/log' ):
			if f.startswith( 'messages' ):
				vt.ingest( os.path.join( '/var/log', f ) )
	else:
		for f in args:
			vt.ingest( f )
	vt.sort()
	for (t,ruleno,line) in vt.every():
		if opts.mark:
			is_marked = vt.is_marked( ruleno )
			if opts.show_rule:
				print >>out, '%15.15s ' % vt.show_rule( ruleno ),
			if is_marked:
				mark = '*'
			else:
				mark = ' '
			print >>out, '%1.1s ' % mark,
		print >>out, '%s' % line
	if opts.show_stats:
		vt.dump_stats( out )
	sys.exit(0)
