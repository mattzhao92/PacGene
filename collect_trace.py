#!/usr/bin/python
import os

traces = []
for root, dirs, files in os.walk('.'):
    for f in files:
        if f.startswith('trace'):
            traces.append('%s/%s' % (root,f))

genes = set([])
for trace in traces:
     with open(trace) as f:
        for line in f:
            if '#' in line:
                parts = line.split()
                gene = parts[-1][1:-1]
                genes.add(gene)
print('\n'.join(genes))
