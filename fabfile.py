from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm
import re

netid = 'zz8'
env.hosts = []
ranges = [1,2,3,4,5,12,13,14,15]
for i in ranges:
    s = str(i)
    if i < 10:
        s = '0' + s
    env.hosts.append('%s@cnode-%s.clear.rice.edu' % (netid,s))

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
    code_dir = '~/PacGene%s' % seq_number

@parallel
def deploy():
    git_repo = 'https://github.com/mattzhao92/Planet-Blitz.git'
    #with settings(warn_only=True):
    #    if run("test -d %s" % code_dir).failed:
    #        run("git clone %s %s" % (git_repo, code_dir))
    with settings(warn_only=True):
        with cd(code_dir):
            #run("git pull")
            run('touch fabric_test')

@parallel
def collect_trace_exec():
    code_dir = get_code_dir(env.host)
    with cd(code_dir):

        return
        traces = []
        for root, dirs, files in os.walk('.'):
            for f in files:
                if 'trace' in f:
                    traces.append('%s/%s' % (root,f))
        #for remote_path in traces:
        #    with tempfile.TemporaryFile() as fd:
        #        get(remote_path, fd)
        #        fd.seek(0)
        #        content=fd.read()
        #        print(content)
        #        for line in content.split('\'):
        #            if '#' in line:
        #                parts = line.split()
        #                gene = parts[-1][1:-1]
        #                print(gene)

def collect_trace():
    with settings(warn_only=True):
        result_map = execute(collect_trace_exec, hosts=env.hosts)


