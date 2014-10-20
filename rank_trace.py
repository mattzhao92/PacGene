#!/usr/bin/python
import os,sys
import subprocess

start = int(sys.argv[-2])
end = int(sys.argv[-1])

cmd = 'head -%d post_population.txt | tail -%d > temp_population.txt' % (end, end-start+1)
os.system(cmd)

cmd = './linux/dual -r %s:%s > score_population.txt' % ('temp_population.txt', 'baseline_population.txt')
os.system(cmd)

results = []
with open('score_population.txt') as f:
    for line in f:
        if line.startswith('score'):
            parts = line.split()
            results.append(parts[1] + ' ' + parts[2])
print('-'.join(results))
os.remove('temp_population.txt')
os.remove('score_population.txt')
