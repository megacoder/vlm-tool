VLM-TOOL: /var/log/messages analysis tool
=========================================

Linux systems can log copious amounts of data in the "/var/log/messages" files.
Too often clustering environments are configured so that each machine keeps its own "/var/log/messages" files locally.
Then, when an event happens on one machine, the other cluster nodes may react and generate their own log messages.
All this gets recorded, but looking at the individual log files makes trying to reconstruct the chain of events a real pain.
The vlm-tool(1) is designed to make this much simpler.

A Knowledgeable Grep
-------------------

In its most basic usage, vlm-tool(1) works much like grep(1) in that there is a set of built-in rules which identify interesting log entries, so:

<pre>
# vlm-tool /var/log/messages
</pre>

Suppose you have the message logs from two cluster nodes:

<pre>
$ vlm-tool node1-messages node2-messages
</pre>

The content of all input files will be merged, using the timestamps conveniently placed on each log entry.
All the log files should be for the same calendar year since the log timestamps do not include a year.
The algorithm tries to keep the lines for each host clumped together if many lines share the same timestamp.

Flooded Logs
------------

Sometimes logs will get endless chatter about some uninteresting topic.
You can just ignore those entries:

<pre>
$ vlm-tool -i ntpd node1-messages node2-messages
</pre>

Any line matching the pattern *ntp* is just dropped from the input.
