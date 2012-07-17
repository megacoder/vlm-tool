VLM-TOOL: /var/log/messages analysis tool
=========================================

Linux systems can log copious amounts of data in the "/var/log/messages" files.  Too often clustering environments are configured so that each machine keeps its own "/var/log/messages" files locally.  Then, when an event happens on one machine, the other cluster nodes may react and generate their own log messages.  All this gets recorded, but looking at the individual log files makes trying to reconstruct the chain of events a real pain.  The vlm-tool(1) is designed to make this much simpler.

In its most basic usage, vlm-tool(1) works much like grep(1) in that there is a set of built-in rules which identify interesting log entries, so:

<pre>
# vlm-tool /var/log/messages
</pre>
