from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm
import re


env.hosts = []

ranges = [1,2,3,4,5,12,13,14,15]
def get_hosts():
    netid = 'zz8'
    hosts = []
    for i in ranges:
        s = str(i)
        if i < 10:
            s = '0' + s
        hosts.append('%s@cnode-%s.clear.rice.edu' % (netid,s))
    return hosts

def get_seg(hostname):
    seq_pattern = re.compile('cnode-\d+')
    seq_number = seq_pattern.findall(hostname)[0].split('-')[1]
    return seq_number

def get_code_dir(hostname):
    return '~/PacGene%s' % get_seg(hostname)


@parallel
def deploy_exec():
    git_repo = 'https://github.com/mattzhao92/Planet-Blitz.git'
    code_dir = get_code_dir(env.host)
    with cd(code_dir+'/linux'):
        return run("make clean && git reset --hard HEAD && git pull")

def deploy():
    with settings(hide('warnings', 'running', 'stdout', 'stderr'), warn_only=True):
        for host, result in execute(deploy_exec, hosts=get_hosts()).items():
            if result.return_code != 0:
                print('%s [ERROR] %s' % (host, str(result)))
            else:
                print('%s [SUCCESS]' % (host))

@parallel
def collect_trace_exec():
    code_dir = get_code_dir(env.host)
    with cd(code_dir):
        return run('./collect_trace.py')

def collect_trace():
    with settings(hide('warnings', 'running', 'stdout', 'stderr'), warn_only=True):
        unique_genes = set([])
        for host, result in execute(collect_trace_exec, hosts=get_hosts()).items():
            if result.return_code != 0:
                print('%s [ERROR] %S' % (host, str(result)))
            else:
                print('%s [SUCCESS]' % (host))
            genes = result.split('-')
            unique_genes.update(genes)
    print('\n'.join(unique_genes))

@parallel
def stop_task_exec():
    return run('screen -X -S gene_task kill')

def stop_task():
    with settings(hide('warnings', 'running', 'stdout', 'stderr'), warn_only=True):
        for host, result in execute(stop_task_exec, hosts=get_hosts()).items():
            if result.return_code != 0:
                print('%s [ERROR] %s' % (host, str(result)))
            else:
                print('%s [SUCCESS]' % (host))

@parallel
def start_task_exec():
    code_dir = get_code_dir(env.host)
    with cd(code_dir+'/linux'):
        run('make clean && make')
        return run('screen -d -m -S gene_task ./population; sleep 1')

def start_task():
    with settings(hide('warnings', 'running', 'stdout', 'stderr'), warn_only=True):
        for host, result in execute(start_task_exec, hosts=get_hosts()).items():
            if result.return_code != 0:
                print('%s [ERROR] %s' % (host, str(result)))
            else:
                print('%s [SUCCESS]' % (host))
@parallel
def rank_exec(total_lines, ranges):
    code_dir = get_code_dir(env.host)
    seg = int(get_seg(env.host))
    index = ranges.index(seg)
    start = index * total_lines/len(ranges) + 1
    if (index == len(ranges)-1):
        end = total_lines
    else:
        end = start + total_lines/len(ranges)-1
    with cd(code_dir):
        return run('./rank_population %d %d' % (start,end))


def rank():
    l = sum(1 for line in open('population.txt'))
    print('Number of lines in Population: %d' % l)
    with settings(hide('warnings', 'running', 'stdout', 'stderr'), warn_only=True):
        gene_to_score = {}
        for host, result in execute(rank_exec, total_lines=l,ranges=ranges, hosts=get_hosts()).items():
            if result.return_code != 0:
                print('%s [ERROR] %S' % (host, str(result)))
            else:
                print('%s [SUCCESS]' % (host))
                for line in result.split('-'):
                    parts = line.split(' ')
                    gene_to_score[parts[1]] = int(parts[0])
                print(line)
