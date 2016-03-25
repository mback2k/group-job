Group-Job
=========
This program is intended to be used within a continuous integration
environment, for example with Buildbot. It wraps a process and puts
it and all its children into the same Windows process job group.
If the main process is exited, killed or terminated, this wrapper
and all child processes within the process job group are killed.

I developed this utility in order to run the curl testsuite with
Buildbot and let it kill all child processes on error or timeout.

Usage: `group-job <command> <arguments...>`

License
-------
Copyright (c) 2012 - 2016, Marc Hoersken, <info@marc-hoersken.de>

This software is licensed as described in the file COPYING, which
you should have received as part of this software distribution.

All trademarks are the property of their respective owners.
