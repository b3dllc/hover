#!/usr/bin/env python3
"""Compiler-driven renamer for the Hover! sources (notes/perfect-match.md
rule 502).

usage: tools/rename.py MAPFILE [--comments | --mangled]
       (from the repository root; builds with ./build.sh)

MAPFILE lines (blank lines and '#' comments ignored):
    token   OLD NEW         whole-word replace in src/ and res/ (OLD must
                            name one entity; the caller checks that)
    token!  OLD NEW         the same, allowing a NEW that src/ already
                            mentions (in comments or strings only)
    member  CLASS OLD NEW   rename a data member or non-virtual method of
                            CLASS (give CLASS's new name if a token line
                            renames it): the declaration inside CLASS's
                            body and CLASS::OLD are renamed directly, and
                            every other use is found by building and
                            fixing the lines the compiler reports
    member! CLASS OLD NEW   the same, allowing a NEW that src/ already
                            uses elsewhere (a deliberately shared name)

Guards: NEW must not occur in src/ beforehand (so a renamed use can never
bind to a local, a parameter or another class's member) nor in the MFC/SDK
headers (an MFC virtual of that name would be overridden), and every
member's CLASS body must contain OLD.

Afterwards comments are renamed where that is unambiguous (an OLD no code
uses any more; CLASS::OLD; theApp.OLD for CHoverApp), and decorated names
in tools/orig_data.csv follow (vtables, globals). --comments and --mangled
run only those passes.

Check the result with tools/compare.py: a rename must leave every
function's score unchanged.
"""
import os
import re
import subprocess
import sys
from collections import defaultdict

SRC = 'src'
FILES = sorted(os.path.join(SRC, f) for f in os.listdir(SRC)
               if f.endswith(('.cpp', '.h')))
# token renames also reach the resource script (IDs from resource.h)
RESFILES = sorted(os.path.join('res', f) for f in os.listdir('res')
                  if f.lower().endswith(('.rc', '.rc2', '.h')))
HDRDIRS = ['/workspace/msvc21x/MSVC20/MFC/INCLUDE',
           '/workspace/msvc22/MSVC20/INCLUDE']


def read(p):
    with open(p, encoding='latin-1') as f:
        return f.read()


def write(p, t):
    with open(p, 'w', encoding='latin-1', newline='') as f:
        f.write(t)


def word(s):
    return re.compile(r'(?<![\w])' + re.escape(s) + r'(?![\w])')


def class_bodies():
    """CLASS -> (file, start, end) of the declaration body, and base map."""
    out, bases = {}, {}
    for p in FILES:
        if not p.endswith('.h'):
            continue
        t = read(p)
        for m in re.finditer(r'^(?:typedef\s+)?(?:class|struct)\s+(\w+)\s*(:[^{;]*)?\{', t,
                             re.M):
            depth, i = 0, m.end() - 1
            while i < len(t):
                if t[i] == '{':
                    depth += 1
                elif t[i] == '}':
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            out[m.group(1)] = (p, m.end(), i)
            if m.group(2):
                bases[m.group(1)] = re.findall(r'(?:public|protected|private)\s+(\w+)', m.group(2))
        for m in re.finditer(r'typedef\s+struct\s+(\w+)\s*\{', t):
            pass
    return out, bases


def ancestors(cls, bases):
    seen, todo = [], [cls]
    while todo:
        c = todo.pop(0)
        if c in seen:
            continue
        seen.append(c)
        todo.extend(bases.get(c, []))
    return seen


def enclosing_class(lines, idx):
    for k in range(idx, -1, -1):
        m = re.match(r'^(?!\s)(?:[\w\s\*&<>,]*?\s)?(\w+)::~?\w+\s*\([^;]*$', lines[k])
        if m:
            return m.group(1)
        m = re.match(r'^(?:class|struct)\s+(\w+)', lines[k])
        if m:
            return m.group(1)
    return None


def main():
    tokens, members, forced = [], [], set()
    for ln in open(sys.argv[1]):
        ln = ln.split('#', 1)[0].split()
        if not ln:
            continue
        if ln[0] in ('token', 'token!'):
            tokens.append((ln[1], ln[2]))
            if ln[0] == 'token!':
                forced.add(ln[2])
        elif ln[0] in ('member', 'member!'):
            members.append((ln[1], ln[2], ln[3]))
            if ln[0] == 'member!':
                forced.add(ln[3])
        else:
            sys.exit(f'bad line: {ln}')

    alltext = '\n'.join(read(p) for p in FILES)
    comments_only = '--comments' in sys.argv
    bymember = defaultdict(list)
    for cls, old, new in members:
        bymember[old].append((cls, new))
    if comments_only:
        comment_pass(bymember, alltext)
        return
    if '--mangled' in sys.argv:
        mangled_pass(tokens, members)
        return
    resume = '--loop' in sys.argv     # rerun only the build-and-fix loop
    news = [n for _, n in tokens] + sorted({n for _, _, n in members})
    keys = [n for _, n in tokens] + [(c, n) for c, _, n in members]
    dup = {k for k in keys if keys.count(k) > 1}
    dup |= {n for _, n in tokens if any(n == m for _, _, m in members)}
    if dup:
        sys.exit(f'NEW used twice in the map: {sorted(dup)}')
    clash = [n for n in news if n not in forced and word(n).search(alltext)]
    if clash and not resume:
        sys.exit(f'NEW already in src/: {clash}')
    # nor in the MFC / SDK headers (a member named like an MFC virtual
    # would override it; an SDK macro would rewrite it)
    hdr = []
    for d in HDRDIRS:
        for f in os.listdir(d):
            if os.path.isfile(os.path.join(d, f)):
                hdr.append(read(os.path.join(d, f)))
    hdr = '\n'.join(hdr)
    clash = [n for n in news if (re.search(r'#\s*define\s+' + re.escape(n) + r'\b', hdr)
                                 if n in forced else word(n).search(hdr))]
    if clash and not resume:
        sys.exit(f'NEW collides with MFC/SDK headers: {clash}')

    # tokens
    for old, new in ([] if resume else tokens):
        if not word(old).search(alltext):
            sys.exit(f'token not found: {old}')
        for p in FILES + RESFILES:
            t = read(p)
            t2 = word(old).sub(new, t)
            if t2 != t:
                write(p, t2)

    # members: declarations and CLASS::OLD
    bodies, bases = class_bodies()
    for cls, old, new in members:
        if cls not in bodies:
            sys.exit(f'class not found: {cls}')
    for cls, old, new in ([] if resume else members):
        bodies, _ = class_bodies()
        p, a, b = bodies[cls]
        t = read(p)
        body = t[a:b]
        if not word(old).search(body):
            sys.exit(f'{old} not declared in {cls}')
        t = t[:a] + word(old).sub(new, body) + t[b:]
        write(p, t)
        pat = re.compile(r'\b' + cls + r'::' + re.escape(old) + r'(?![\w])')
        for q in FILES:
            u = read(q)
            u2 = pat.sub(cls + '::' + new, u)
            if u2 != u:
                write(q, u2)

    # theApp.OLD always names the CHoverApp member, and macro bodies (which
    # the compiler reports at the macro's use) are only reached this way
    for cls, old, new in members:
        if cls == 'CHoverApp':
            pat = re.compile(r'\btheApp\.' + re.escape(old) + r'(?![\w])')
            for q in FILES:
                u = read(q)
                u2 = pat.sub('theApp.' + new, u)
                if u2 != u:
                    write(q, u2)

    # build loop
    rx = re.compile(r"^(src\\\w+\.(?:cpp|h))\((\d+)\) : error (C\d+): '(\w+)' : "
                    r"(?:is not a member of '(\w+)'|undeclared identifier)")
    for it in range(150):
        r = subprocess.run(['./build.sh', '/K'], capture_output=True,
                           text=True, errors='replace')
        out = r.stdout + r.stderr
        fixes = defaultdict(list)
        other = []
        for line in out.splitlines():
            m = rx.match(line)
            if m and m.group(4) in bymember:
                fixes[(m.group(1).replace('\\', '/'), int(m.group(2)))].append(
                    (m.group(4), m.group(5)))
            elif re.match(r"^(src\\\w+\.(?:cpp|h))\((\d+)\) : error C2614: '(\w+)' : illegal member initialization: '(\w+)'", line):
                m = re.match(r"^(src\\\w+\.(?:cpp|h))\((\d+)\) : error C2614: '(\w+)' : illegal member initialization: '(\w+)'", line)
                if m.group(4) in bymember:
                    fixes[(m.group(1).replace('\\', '/'), int(m.group(2)))].append(
                        (m.group(4), m.group(3)))
                else:
                    other.append(line)
            elif ' error ' in line or 'fatal error' in line:
                other.append(line)
        if not fixes:
            if other:
                print('\n'.join(other[:40]))
                sys.exit('build errors remain that this tool does not handle')
            print(f'clean after {it} fix rounds')
            break
        n = 0
        for (p, lno), errs in sorted(fixes.items()):
            lines = read(p).split('\n')
            for old, cls in errs:
                cands = bymember[old]
                pick = None
                if len(cands) == 1:
                    pick = cands[0][1]
                else:
                    c = cls or enclosing_class(lines, lno - 1)
                    anc = ancestors(c, bases) if c else []
                    for cc, nn in cands:
                        if cc in anc:
                            pick = nn
                            break
                if pick is None:
                    print(f'AMBIGUOUS {p}:{lno} {old} ({cls})')
                    continue
                new_line, k = word(old).subn(pick, lines[lno - 1])
                if k:
                    lines[lno - 1] = new_line
                    n += 1
                    continue
                # a statement spanning lines is reported on its last line
                for j in range(lno - 2, max(lno - 10, -1), -1):
                    new_line, k = word(old).subn(pick, lines[j])
                    if k:
                        lines[j] = new_line
                        n += 1
                        break
                    if lines[j].rstrip().endswith((';', '{', '}')):
                        break
                if k:
                    continue
                # a macro body is reported where the macro is used
                for j in range(lno - 1, -1, -1):
                    if lines[j].startswith('#define'):
                        e = j
                        while lines[e].rstrip().endswith('\\'):
                            e += 1
                        hit = False
                        for q in range(j, e + 1):
                            new_line, k = word(old).subn(pick, lines[q])
                            if k:
                                lines[q] = new_line
                                hit = True
                        if hit:
                            n += 1
                            break
            write(p, '\n'.join(lines))
        print(f'round {it}: {n} fixes')
        if n == 0:
            sys.exit('no progress')
    else:
        sys.exit('too many rounds')

    comment_pass(bymember, alltext)
    mangled_pass(tokens, members)
    leftover_report(members)


def leftover_report(members):
    # The compiler never sees #if 0 blocks: report old member names that no
    # class declares any more but that still occur outside comments.
    decl = '\n'.join(read(p) for p in FILES if p.endswith('.h'))
    for cls, old, new in members:
        if re.search(r'^\s+[\w:<>\*& ]+[ \*&]' + re.escape(old) + r'\s*[\[;(]',
                     decl, re.M):
            continue
        for p in FILES:
            for i, l in enumerate(read(p).split('\n'), 1):
                if word(old).search(re.sub(r'//.*', '', l)):
                    print(f'LEFTOVER {p}:{i}: {cls}::{old} -> {new}')


MANGLED = ['tools/orig_data.csv']


def mangled_pass(tokens, members):
    # decorated names in the data-symbol list: ??_7CLASS@@6B@, ?g_x@@3KA,
    # ?METHOD@CLASS@@...
    for p in MANGLED:
        t = read(p)
        t0 = t
        for cls, old, new in members:
            t = t.replace('?' + old + '@' + cls + '@', '?' + new + '@' + cls + '@')
        for old, new in tokens:
            t = re.sub(r'(?<=[?@7])' + re.escape(old) + r'(?=@)', new, t)
        if t != t0:
            write(p, t)
            print(f'{p}: decorated names renamed')


def comment_pass(bymember, alltext):
    # comments: an OLD that no code uses any more is renamed in comments too
    code = []
    for p in FILES:
        for l in read(p).split('\n'):
            code.append(re.sub(r'//.*', '', l))
    code = '\n'.join(code)
    for old, cands in bymember.items():
        if len(cands) == 1 and not word(old).search(code):
            new = cands[0][1]
            for p in FILES:
                t = read(p)
                t2 = word(old).sub(new, t)
                if t2 != t:
                    write(p, t2)
        else:
            for cls, new in cands:
                pats = [re.compile(r'\b' + cls + r'::' + re.escape(old) + r'(?![\w])')]
                if cls == 'CHoverApp':
                    pats.append(re.compile(r'\btheApp\.' + re.escape(old) + r'(?![\w])'))
                for p in FILES:
                    t = read(p)
                    t2 = t
                    for pat in pats:
                        t2 = pat.sub(lambda m: m.group(0)[:-len(old)] + new, t2)
                    if t2 != t:
                        write(p, t2)
            left = sum(len(word(old).findall(read(p))) for p in FILES)
            if left:
                print(f'note: {old} still mentioned {left}x (other classes '
                      f'or bare comment mentions)')


if __name__ == '__main__':
    main()
