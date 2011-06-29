#! /usr/bin/env python

import os
import argparse
import shutil
import sys
import string
import re

parser = argparse.ArgumentParser()
parser.add_argument("-n","--dbusname",dest="dbusname",type=str,required=True,
        help="DBus name of the service. For service Strigi it is nepomukstrigiservice and thus address is org.kde.nepomuk.services.nepomukstirgiservice" )
parser.add_argument("-d","--desktop-file",dest="desktop",type=str,required=True,
        help="Name of the desktop file for service")
parser.add_argument("-p","--plugin-name",dest="plugin",type=str, required=True,
        help = "Name of the plugin. Used in variaous description fields. This field may be user-visible.\n Do not use 'BlaBlaBla nepomuk service plugin', use only 'BlaBlaBla' instead")

args=parser.parse_args()
print args.dbusname, args.desktop, args.plugin 

# Stage 1 - Check that template files exists
def ask_yes_no(question):
     valid = {"yes":"yes",   "y":"yes",  "ye":"yes",
                          "no":"no",     "n":"no"}

     msg = question + "[y/n]"
     while 1:
         choice = raw_input(msg).lower()
         if choice in valid:
             return valid[choice]
         else:
             print "Only y/yes/n/no"


template_dir = './basic_template'
template_files = []
try:
    template_files = filter( lambda x: not x.startswith(".") , os.listdir(template_dir) )
    if len(template_files) == 0:
        print "Template directory contain no template files"
        sys.exit(1)

except OSError:
    print "Can't find template dir. You must call script from <repository>/plugins/"
    sys.exit(1)

for template in template_files:
    if not template.endswith(".in"):
        print "Template %s has incorrect filename. Do you launch script in the right directory ?" % template
        sys.exit(1)

# Stage 2 - check and prepare directory for result
output_dir = "__result"

if os.path.exists(output_dir):
    answer = ask_yes_no("Directory '__result' already exist. Do you want to replace it?")
    if answer == "no":
        print "Aborting"
        sys.exit(0)
    else:
        try:
            print "Removing %s" % output_dir
            shutil.rmtree(output_dir,False)
        except OSError,ValueError:
            print "Cant remove directory"
            sys.exit(1)

try:
    os.mkdir(output_dir)
except OSError:
    print "Can't create directory '%s'." % output_dir
    sys.exit(1)

# Actually parse templates and create files

class MyTemplate(string.Template):
    pattern = r"@(?:(?P<escaped>@)|(?P<named>\w+?)@|(?P<braced>\w+?)@|(?P<invalid>$))"


mappings = {
        "_dbusname" : args.dbusname,
        "_servicedesktopfile" : args.desktop,
        "_pluginsystemname" : args.plugin,
        "_service_description" : ""
        }
for template in template_files:
    out_name = template[:-len(".in")].replace("template",args.dbusname)
    infile = open(os.path.join(template_dir,template),'r')
    outfile = open( os.path.join(output_dir,out_name), 'w')
    print "%s > %s" % ( template,out_name)
    i = 0
    for line in  infile:
        try:
            outfile.write(MyTemplate(line).substitute(mappings))
        except KeyError,e:
            print "Error while substitution if file %s line %s" % (template,i), e

        i = i + 1


print """Your plugin has been successfully generated. What's next:
1. rename %s folder. e.g. mv %s <pluginname>plugin
2. open CMakeLists.txt ( <repository>/plugins/CMakeLists.txt ) and add your 
   plugin as subdirectory ( add_subdirectory(yourfoldername) )
3. Now try to build whole project. You will have a bunch of erros regarding
   your new plugin. THIS IS INTENTIONAL. You can actully skip this step and 
   go directly to the next one
4. Open your plugins CMakeLists.txt ( yourfoldername/CMakeLists.txt ). You
   will see some comments describing what should be adjusted. You at least
   have to insert correct name of your service DBus interface or comment 
   this line out if you don't need it.
5. Now you should edit source and headers files in your plugin directory.
   They are commented too. Full docs about how it all works and how to
   implement more complex things then standard service plugins can be 
   found in documentation ( or in header systrayplugin.h under 
   <repostitory>/lib/ ). And see strigi plugin as example.
6. It is not as complex as it sounds. Good luck!
""" % ( output_dir, output_dir )

