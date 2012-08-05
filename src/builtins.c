#include <builtins.h>
#include <stdio.h>

/*
 *------------------------------------------------------------------------
 * Resist the urge to re-order these entries; order matters. For instance,
 * we want to match "read-only", before matching "only".
 *------------------------------------------------------------------------
 */

char const * const	builtin_triggers[] =	{
	"abnormal",
	"abo?rt(d)?",
	"access[[:space:]]beyond",
	"asked[[:space:]]fo",
	"bcm5709",
	"blocked[[:space:]]for[[:space:]]more",
	"becomes",
	"bond(ing|[0-9.]+)?",
	"call[[:space:]]trace:",
	"clocksource",
	"conflict",
	"crs",
	"css",
	"device-mapper",
	"(de)?bug",
	"dlm",
	"disagrees.*symbol.*",
	"disk.*,.*o:.*$",
	"don't",
	"el[0-9]debug",
	"end_request",
	"error",
	"fail(ed(:.*)?|ure|ing|-?over)?",
	"fenc(e,ing)",
	"fsck",
	"general protection rip:",
	"h[au]ng",
	"illegal:",
	"init:",
	"invalid(:.*)?",
	"i?scsi",
	"kdump(:.*)?",
	"kernel command line:.*",
	"kill(ing)?",
	"link",
	"redhat.com",			/* Keep before 'linux version'	 */
	"linux[[:space:]]version",	/* Keep after 'redhat.com'	 */
	"ll header:.*",
	"lpfc",
	"machine check events logged",
	"martian",
	"remaining active paths:",	/* Keep before 'multipathd'	 */
	"multipathd",			/* after 'remaining active p..'  */
	"netdev",
	"nfs:.*not[[:space:]]responding",
	"nfsd:",
	"no[[:space:]]space[[:space:]]left",
	"not[[:space:]]supported",
	"not[[:space:]]*within[[:space:]]*permissible[[:space:]]range",
	"nmi",
	"o2(cb|hb|net)?",
	"ocfs2?",
	"oom",
	"oops",
	"out.*of.*memory",
	"own[[:space:]]address",
	"panic",
	"please",
	"page[[:space:]]allocation[[:space:]]failure",
	"power[[:space:]]states",
	"qla.*loop.*(up|down)",
	"\\b[rm]d:.*",
	"rds(:|/ib)?",
	"read-?only",
	"shutdown([[].*[]])?:",
	"only",
	"require",
	"restart",
	"return code =.*",
	"runlevel: [0-9]*",
	"segfault",
	"selinux",
	"setroubleshoot",
	"signal([[:space:]]*[0-9]*)",
	"sysrq.*:",
	"taint",
	"terminated",
	"time-?out",
	"unable",
	"unknown[[:space:]]symbol.*",
	"unsupported",
	"vfs:",
	"warning(:.*)?",
	"watchdog",
	NULL
};
