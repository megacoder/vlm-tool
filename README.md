# VLM-TOOL: /var/log/messages analysis tool

Linux systems can log copious amounts of data in the "/var/log/messages" files.
Too often clustering environments are configured so that each machine keeps its own "/var/log/messages" files locally.
Then, when an event happens on one machine, the other cluster nodes may react and generate their own log messages.
All this gets recorded, but looking at the individual log files makes trying to reconstruct the chain of events a real pain.
The vlm-tool(1) is designed to make this much simpler.

## A Knowledgeable Grep

In its most basic usage, vlm-tool(1) works much like grep(1) in that there is a set of built-in rules which identify interesting log entries, so:

<pre>
# vlm-tool /var/log/messages
</pre>

Suppose there are message logs from two cluster nodes:

<pre>
$ vlm-tool node1-messages node2-messages
</pre>

The content of all input files will be merged, using the timestamps conveniently placed on each log entry.
All the log files should be for the same calendar year since the log timestamps do not include a year.
The algorithm tries to keep the lines for each host clumped together if many lines share the same timestamp.

## Not for just /var/log/messages but /var/log/secure as well

By luck or good design, both the */var/log/messages* and the */var/log/secure* use the same log layout scheme.  Mention both on the command line:

<pre>
$ vlm-tool /var/log/messages /var/log/secure
</pre>

to coordinate entries in one file with the other.

This scheme will work with any log conforming to the layout used by the *syslog(3)* interface.

## Flooded Logs

Sometimes logs will get endless chatter about some uninteresting topic.
You can just ignore those entries:

<pre>
$ vlm-tool -i ntpd node1-messages node2-messages
</pre>

Any line matching the pattern *ntpd* is just dropped from the input.

Have too many trigger patterns to keep writing on the command line?
No problem, just put them in a file, one per line.
Use your ignored trigger collection like this:

<pre>
$ vlm-tool -I my_ignores node1-messages node2-messages
</pre>

## Adding Your Own Triggers

The built-in matching trigger patterns include strings which I've found associated with problems.
Perhaps you have your own log entry you need included:

<pre>
$ vlm-tool -a dull node1-messages node2-messages
</pre>

Now, any log entry matching the trigger *dull* will be shown.

Maybe you have lots and lots of custom triggers.
Add them to a file, one-per-line, and use them like this:

<pre>
$ vlm-tool -A my_rules messages
</pre>

## Ignoring The Built-in Trigger Patterns

You can focus your search to only a specific trigger:

<pre>
$ vlm-tool -n -a panic node1-messages
</pre>

The *-n* switch inhibits populating the built-in triggers.
This also works with the *-A* option to read the new triggers from a file.

## Debugging Trigger Patterns

Each trigger pattern is a *regex(3)* regular expression.
They are applied in an arbitrary order; first match *wins* and no further matching is done.

You can see which trigger rule fired on a particular entry:

<pre>
$ vlm-tool -r messages
</pre>

### Caution

So you can better see what is happening, this will output *all* your messages but will highlight the matched entries, and include the first few characters of the matched trigger.

## Highlighting Matched Entries In Context

Sometimes just seeing the selected log messages is not enough.
You need to see the surrounding log entries, too.

<pre>
$ vlm-tool -m node1-messages node2-messages
</pre>

Each log entry will be output, so if your log file are big, be prepared for lots of output.
By default, log entries which match a trigger pattern, will be prefixed with a *thumb* which defaults to "- ", a minus and a space.
Log entries which are not matched, are prefixed with enough spaces so that the columns still line up.

The default *thumb* was chosen to be small, and easy to type into less(1) or vi(1).
You can change your *thumb* string if you like, see the man(1) page.

## Colorized Output

If you run vlm-tool(1) using the *-c* switch, ANSI color sequences will be embedded into the output.

### What The Matching Trigger Pattern Matched

In addition to the *thumb*, that part of the log entry which matched the trigger rule is highlighted.
This can make scanning through a really large log file quicker, because it attracts your eye to the relevant portion of the message.

You do not get choice of color.

### Colored Host Names

The hostname portion of each log line is colored.
This can help visually group related log entries together and avoid the eye-tedium of matching entries to the host which logged them.

Host name colors are recycled, so on larger clusters host name colors will be duplicated.
Hopefully not too many of these hosts will be logging at the same time, but your mileage may vary.

## Saving The Results For Later

Instead of writing to the standard output *stdout*, you can redirect to a file:

<pre>
$ vlm-tool -o for_later messages messages.1
</pre>

## Recognizing Compressed Files

Frequently, part of log rotation includes compressing the older log files.
No need to uncompress them before using vlm-tool(1) because files compressed using gzip (*.gz*), bzip2 (*.bz2*), compress (*.Z*), and xz (*.xz*) are recognized by their filename extensions.

## When In Doubt

Read the man(1) page.
