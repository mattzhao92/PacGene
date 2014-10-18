from __future__ import with_statement
from fabric.api import *
from fabric.contrib.console import confirm

env.hosts = []

for i in range(1,10):
    env.hosts.append('zz8@cnode-0%s.clear.rice.edu' % str(i))
for i in range(10, 16):
    env.hosts.append('zz8@cnode-%s.clear.rice.edu' % str(i))

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

def deploy():
    code_dir = '~/PacGene'
    git_repo = 'https://github.com/mattzhao92/Planet-Blitz.git'
    #with settings(warn_only=True):
    #    if run("test -d %s" % code_dir).failed:
    #        run("git clone %s %s" % (git_repo, code_dir))
    with cd(code_dir):
        #run("git pull")
        run('toubh fabric_test')
