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
	"addrconf[(].*",
	"access[[:space:]]beyond",
	"asked[[:space:]]fo",
	"avc:.*denied",
	"bcm5709",
	"martian source.*",	/* Keep this before "bonding" 		 */
	"blocked[[:space:]]for[[:space:]]more",
	"badness.*",
	"becomes",
	"bond(ing|[a-zA-Z]*[0-9.]+)",
	"c-state",
	"can't contact ldap server",
	"call[[:space:]]trace:",
	"certificate(s)",
	"checker failed path",
	"clocksource",
	"conflict",
	"crs",
	"css",
	"dazed and confused",
	"d-?bus",
	"detected virtualization '.*",
	"device-mapper",
	"(de)?bug",
	"dhcpd:.*",
	"dlm",
	"disagrees.*symbol.*",
	"disk.*,.*o:.*$",
	"don't",
	"drop(ped)?",
	"drop_caches:",
	"el[0-9]debug",
	"energy_perf_bias:.*",
	"end_request",
	"enforcing=[[:digit:]]*",
	"errno([[:space:]]*[[:digit:]][[:digit:]]*)?",
	"error",
	"/etc/sysconfig/network-scripts",
	"eth([[:digit:].:])*",
	"fail(ed(:.*)?|ure|ing|-?over)?",
	"fenc(e,ing)",
	"floating-point",
	"fsck",
	"general protection rip:",
	"[[:space:]]h[au]ng[^[:alnum:]]",
	"illegal(:.*)?",
	"init:",
	"invalid(:.*)?",
	"i?scsi([[:digit:]]*)?",
	"is down",
	"is up",
	"kdump(:.*)?",
	"kernel: linux version.*",
	"kernel: .*[[:space:]]+D[[:space:]]+.*",
	"kernel command line:.*",
	"khash_super_prune_nolock",
	"kill(ing)?",
	"link",
	"redhat.com",			/* Keep before 'linux version'	 */
	"leap[[:space:]]*second",
	"linux[[:space:]]version",	/* Keep after 'redhat.com'	 */
	"ll header:.*",
	"lpfc",
	"machine check events logged",
	"mark as failed",
	"(syste)?md(:|[[:digit:]]*|s.*)",	/* mdstat and systemd	 */
	"mismatch(ed)?",
	"remaining active paths:[[:space:]]*[[:digit:]]*",	/* Keep before 'multipathd'	 */
	"multipathd",			/* after 'remaining active p..'  */
	"netdev",
	"nfs server.*",
	"nfs:.*not[[:space:]]responding",
	"nfsd:",
	"nss_ldap:.*",
	"no[[:space:]]space[[:space:]]left",
	"not[[:space:]]supported",
	"not[[:space:]]*within[[:space:]]*permissible[[:space:]]range",
	"[^_]nmi",
	"numa turned (on|off)",
	"o2(cb|hb(monitor)?|net)?",
	"ocfs2?",
	"oom",
	"oops",
	"out.*of.*memory",
	"own[[:space:]]address",
	"panic",
	"please",
	"page[[:space:]]allocation[[:space:]]failure",
	"power[[:space:]]states",
	"qla.*loop.*(up|down|ready)",
	"\\b[rm]d:.*",
	"rds(:|/ib)?",
	"read-?only",
	"recovered to normal mode",
	"shutdown([[].*[]])?:",
	"only",
	"oracle",
	"require",
	"restart",
	"return code =.*",
	"runlevel: [0-9]*",
	"segfault",
	"selinux",
	"setroubleshoot",
	"signal([[:space:]]*[0-9]*)",
	"snmpd.*Got trap from peer*",
	"sysrq.*:",
	"taints?",
	"terminated",
	"time-?out",
	"udev:",
	"uhhuh",
	"unable",
	"unauthorized",
	"unknown[[:space:]]symbol.*",
	"unsupported",
	"vfs:",
	"warning(:.*)?",
	"watchdog",
	NULL
};
