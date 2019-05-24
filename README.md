# perflock

Perflock is a tool that aids groups in sharing access to multi-user
machines. It acts as a RWMutex, allowing either one process exclusive
access, or multiple processes to that concurrent access.

## Setup

Perflock uses a file lock to mediate access to the machine. You need to
create the lock file `/run/perflock` before running perflock, and then
set its permissions such that it is writeable by anyone who will run
`perflock`.

## Usage

To run an exclusive job (for example, a performance experiment that
requires all cores):

```console
$ perflock ./super --intensive job
```

If another job tries to run at the same time, it will join a queue:

```console
$ perflock ../2nd/job --that -s big
Waiting for lock
```

When the first process finishes, the second one will be allowed to run.

## Shared access

Some jobs are not performance critical, and simply want to not interfere
with other, important jobs. Such jobs should be scheduled with `pls`
(pronounced "please"). `pls` works the same way as `perflock`, but will
allow multiple `pls`-wrapped jobs to execute in parallel, unless an
exclusive job is currently running. If an exclusive job is later
enqueued, it will wait until all the `pls` jobs have finished.

## History

`perflock` was originally developed inside the [Parallel and Distributed
Operating Systems group](https://pdos.csail.mit.edu/) at MIT CSAIL for
use with our multi-core machines. Over the years, it has been rewritten
and extended by many people in the group, to the point where it no
longer can be said to "belong" to any of us. Once other groups asked how
we share our machines fairly, we decided it might be useful to them, and
so have decided to open-source it.

## Mechanism

`perflock` is a very simple program -- it is really just a wrapper
around `flock`, which provides exclusive locking for a UNIX file. We
also include `wholock`, written by @aclements, which lists who is
currently holding the lock, and what other jobs are waiting in the
queue.
