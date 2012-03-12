#include <builtins.h>
#include <stdio.h>

char const * const	builtin_triggers[] =	{
	"abnormal",
	"abo?rt(d)?",
	"access[[:space:]]beyond",
	"asked[[:space:]]fo",
	"bcm5709",
	"blocked[[:space:]]for[[:space:]]more",
	"becomes",
	"bond(ing)?",
	"call[[:space:]]trace:",
	"clocksource",
	"conflict",
	"crs",
	"css",
	"device-mapper",
	"(de)?bug",
	"dlm",
	"disagrees.*symbol.*",
	"don't",
	"end_request",
	"error",
	"fail(ed|ure|ing)?",
	"fenc(e,ing)",
	"fsck",
	"h[au]ng",
	"illegal:",
	"init:",
	"invalid:",
	"i?scsi",
	"kdump:?",
	"kill(ing)?",
	"link",
	"linux[[:space:]]version",
	"lpfc",
	"multipathd",
	"netdev",
	"nfs:.*not[[:space:]]responding",
	"nfsd:",
	"no[[:space:]]space[[:space:]]left",
	"not[[:space:]]supported",
	"nmi",
	"o2",
	"ocfs2?",
	"only",
	"oom",
	"oops",
	"out.*of.*memory",
	"own[[:space:]]address",
	"panic",
	"please",
	"page[[:space:]]allocation[[:space:]]failure",
	"power[[:space:]]states",
	"qla.*loop.*(up|down)",
	"rds/(ib)?",
	"read-?only",
	"register(ed|ing)?",
	"require",
	"restart",
	"segfault",
	"selinux",
	"signal([[:space:]]*[0-9]*)",
	"sysrq.*:",
	"taint",
	"terminated",
	"time-?out",
	"unable",
	"unknown[[:space:]]symbol.*",
	"unsupported",
	"vfs:",
	"warning",
	"watchdog",
	NULL
};
