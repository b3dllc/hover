#!/usr/bin/env python3
"""Run a command as a child-subreaper: every process it (or Wine) daemonises is
reparented to us instead of PID 1, so we can reap it and nothing is left as a
zombie in the container's pid cgroup.  After the command exits we wait up to
GRACE seconds for stragglers, then SIGKILL whatever is left and reap that too."""
import ctypes, os, signal, subprocess, sys, time
GRACE = 30
libc = ctypes.CDLL(None, use_errno=True)
if libc.prctl(36, 1, 0, 0, 0) != 0:          # PR_SET_CHILD_SUBREAPER
    sys.exit("prctl(PR_SET_CHILD_SUBREAPER) failed")
me = os.getpid()
rc = subprocess.call(sys.argv[1:])
def children():
    out = []
    for d in os.listdir('/proc'):
        if d.isdigit():
            try:
                with open(f'/proc/{d}/stat') as f:
                    if int(f.read().rsplit(')', 1)[1].split()[1]) == me:
                        out.append(int(d))
            except Exception:
                pass
    return out
def reap():
    while True:
        try:
            pid, _ = os.waitpid(-1, os.WNOHANG)
        except ChildProcessError:
            return
        if pid == 0:
            return
deadline = time.time() + GRACE
while True:
    reap()
    kids = children()
    if not kids:
        break
    if time.time() > deadline:
        for p in kids:
            try: os.kill(p, signal.SIGKILL)
            except ProcessLookupError: pass
    time.sleep(0.5)
sys.exit(rc)
