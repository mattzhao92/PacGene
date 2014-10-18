from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm
import re


env.hosts = []

def get_hosts():
    netid = 'zz8'
    ranges = [1,2,3,4,5,12,13,14,15]
    hosts = []
    for i in ranges:
        s = str(i)
        if i < 10:
            s = '0' + s
        hosts.append('%s@cnode-%s.clear.rice.edu' % (netid,s))
    return hosts

def test():
    with settings(warn_only=True):
        result = local('./manage.py test my_app', capture=True)
    if result.failed and not confirm("Tests failed. Continue anyway?"):
        abort("Aborting at user request.")

def commit():
    local("git add -p && git commit")

def push():
    local("git push")

def prepare_deploy():
    test()
    commit()
    push()

def get_code_dir(hostname):
    seq_pattern = re.compile('cnode-\d+')
    seq_number = seq_pattern.findall(hostname)[0].split('-')[1]
    return '~/PacGene%s' % seq_number


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
    with settings(warn_only=True):
        unique_genes = set([])
        for host, result in execute(collect_trace_exec, hosts=env.hosts).items():
            if result.return_code != 0:
                print('%s [ERROR] %S' % (host, str(result))
            else:
                print('%s [SUCCESS]' % (host))




