import os
import os.path


#compilation mode: please choose between debug and release
compile_mode = "release"
#compile_mode = "debug"

#users may overide these settings if SCons cannot automatically locate some library:

#PATH to g77 or gfortran:
user_path_fortrancompiler = ""

#libg2c.a (or .so) or libgfortran.a PATH:
user_path_libg2cgfortran = ""

#boost libraries PATH:
user_path_boost = ""


COMMON_CPP = Split ("""atom.cpp
                       basetypes.cpp
                       rigidbody.cpp
                       attractrigidbody.cpp
                       coord3d.cpp
                       pdbio.cpp
                       geometry.cpp
                       atomselection.cpp
                       rmsd.cpp
                       forcefield.cpp
                       pairlist.cpp
                       minimizers/lbfgs_interface.cpp
                       minimizers/routines.f
                       minimizers/lbfgs_wrapper/lbfgsb_wrapper.cpp
                       mcopff.cpp
                       surface.cpp
                       coordsarray.cpp
                       superpose.cpp
                       version.cpp
                       attractforcefield.cpp
                    """)

COMMON_CPP = [os.path.join('src', i) for i in COMMON_CPP]                    


COMMON_LIBS=[""]
#COMMON_LIBS=["gfortran"]
#COMMON_CPPPATH=['.', '/sw/include/boost-1_33_1']
COMMON_CPPPATH=['headers']
FFLAGS="-g"

#generates bzrrev.h:
import bzrrev


def FIND_FILE(name,path, useEnvPath=False):
   if useEnvPath:
      additional_path = os.environ['PATH'].split(':')
      path.extend(additional_path)
   
   for p in path:
     if os.path.exists(p+'/'+name):
        return p+'/'+name
   return None 



def FIND_HEADER(names, paths, useEnvPath=False):
   #find a library in a given set of directories
   
   if useEnvPath:
      additional_path = os.environ['PATH'].split(':')
      paths.extend(additional_path)
   
   for p in paths:
      if os.path.exists(p):
         files=os.listdir(p)
	 for n in names:
	    if os.path.exists(p+"/"+n):
	       return p
	       	       
   return None



#find g++ / g77 or g++/gfortran:
FORTRANPROG = ""
LIB_PATH=['.']

print "Searching for compilers..."
gpp = FIND_FILE("g++", ["/usr/bin"], True)
if gpp is None:
   print "Error: cannot locate g++, compilation aborted"
   Exit(1)
else:
   print "g++ found here: ", gpp

g77 = FIND_FILE("g77", [user_path_fortrancompiler,"/usr/bin","/sw/bin/"], True)
if g77 is None:
   print "Cannot locate g77 fortran compiler, trying gfortran:"
   gfortran=FIND_FILE("gfortran", [user_path_fortrancompiler,"/usr/bin"], True)
   if gfortran is None:
      print "Error: no fortran compiler found, aborting"
      Exit(1)
   else: #gfortran compiler, try to locate libgfortran
      print "using fortran compiler: gfortran"
      FORTRANPROG="gfortran -O2 -g -fPIC"
      gfortranlib=FIND_HEADER(["libgfortran.a", "libgfortran.la",\
          "libgfortran.so", "libgfortran.so.3"], [user_path_libg2cgfortran,"/usr/lib","/usr/lib64","/sw/lib",\
          "/usr/lib/gcc/x86_64-redhat-linux/3.4.6/","/usr/lib64/gcc/x86_64-suse-linux/4.1.2/","/opt/local/lib/gcc43/"], True)
      if gfortranlib is not None:
         print "libgfortran found here: ", gfortranlib
         LIB_PATH.append(gfortranlib)
         COMMON_LIBS.append("gfortran")
      else:
         print "WARNING: libgfortran not found, may not compile..."      
         COMMON_LIBS.append("gfortran")
    
else: #g77 compiler
      print "using fortran compiler: g77"
      FORTRANPROG="g77 -O2 -g -fPIC"
      g2clib = FIND_HEADER(["libg2c.a", "libg2c.so", "libg2c.la","libg2c.so.0"], [user_path_libg2cgfortran,"/usr/lib",\
      "/usr/local/lib/", "/sw/lib/","/usr/lib/gcc/x86_64-redhat-linux/3.4.6/"], True)
      if g2clib is None:
         print "Warning: libg2c not found, may not compile..."
         COMMON_LIBS.append("g2c")
      else:
         print "   libg2c found here:", g2clib
	 LIB_PATH.append(g2clib)
         COMMON_LIBS.append("g2c")

      
   
   







boostdir=FIND_HEADER(["boost/shared_array.hpp"], [user_path_boost,"/usr/include", \
"/sw/include/boost-1_33_1", "/opt/local/include"] )

if boostdir is None:
   print "cannot locate Boost hearders directory, still trying to compile..."
else:
   print "boost directory found here: ", boostdir 
   COMMON_CPPPATH.append(boostdir)

PYTHON_CPPPATH = ""
PYTHON_LIBS = ""
PYTHON_CPP=[]

if 'python' in COMMAND_LINE_TARGETS or len(COMMAND_LINE_TARGETS)==0:
    python_target = True
else:
    python_target = False

if python_target:
    
   python_dirs = {'2.4': ["/usr/include/python2.4", "/sw/include/python2.4"] , 
                  '2.5': ["/usr/include/python2.5", "/sw/include/python2.5"] , 
                  '2.6': ["/usr/include/python2.6", "/sw/include/python2.6"] , 
                  '2.7': ["/usr/include/python2.7", "/sw/include/python2.7"] , 
                 }
   
   python_versions = ['2.7', '2.6', '2.5', '2.4'] 
    
   
   
   for version in python_versions:
       dir = FIND_HEADER(["Python.h"], python_dirs[version])
       if dir is not None:
           PYTHON_CPPPATH=[dir]
           PYTHON_LIBS=["python%s"%version]
           break



   PYTHON_CPP=[]
   import fnmatch
   import os
   for file in os.listdir("Pybindings"):
       if fnmatch.fnmatch(file, "*.cpp"):
           PYTHON_CPP.append("Pybindings/%s"%file)



if compile_mode == "release":
    ccflags = "-Wall -O2 -fPIC -Woverloaded-virtual -DNDEBUG -ffunction-sections -fvisibility=hidden"
else:
    ccflags = "-Wall -O2 -fPIC -g -Woverloaded-virtual"


print "common cpp path:", COMMON_CPPPATH
		
common=Environment(LIBS=COMMON_LIBS,CPPPATH=COMMON_CPPPATH, CCFLAGS=ccflags, LIBPATH=LIB_PATH, FORTRAN=FORTRANPROG,   FORTRANFLAGS="-g -fPIC" )


#common.Append(CCFLAGS='-Wall -O2 -fPIC -Woverloaded-virtual -DNDEBUG')                  #fastest(?) release
#common.Append(CCFLAGS='-Wall -O2 -g -fPIC -D_GLIBCXX_DEBUG')    #debuging high (use with care !)
#common.Append(CCFLAGS='-Wall -O0 -g -pg -fPIC -DNDEBUG ')        #profiling


#check for some boost header files:
has_boost=False
conf = Configure(common)
if conf.CheckCXXHeader("boost/shared_array.hpp"):
    has_boost=True

if not has_boost:
    print "cannot find boost::shared_array include file"
    Exit(1)
common = conf.Finish()





python=common.Copy()
nopython=common.Copy()

python.Append(CPPPATH=PYTHON_CPPPATH, LIBS=PYTHON_LIBS)


objects=common.SharedObject(COMMON_CPP)
statics=common.StaticObject(COMMON_CPP)  #to make a static library

python.Append(LIBS=['boost_python'])


#check python header file:
if python_target:
   conf = Configure(python)
   if not conf.CheckCHeader('Python.h'):
           print "you must install either python2.4-dev or python2.5-dev"
           Exit(1)

   env = conf.Finish()


print "using CPPPATH =", python['CPPPATH']



lib1=python.SharedLibrary(File('_ptools.so'),source=[objects,PYTHON_CPP])
#lib2=nopython.SharedLibrary('ptools',source=[objects])
lib2=nopython.StaticLibrary('ptools',source=[statics]) #this makes the pure C++ static library


Alias('python',lib1)
Alias('cpp',[lib2] )
print "BUILD_TARGETS is", map(str, BUILD_TARGETS)


#### old commands. Not needed but kept here for some time #####

#Alias('cpp',[lib2,tst])
