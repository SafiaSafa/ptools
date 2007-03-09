COMMON_CPP = Split ("""atom.cpp
                       rigidbody.cpp
                       coord3d.cpp
                       pdbio.cpp
                       geometry.cpp
                       atomselection.cpp
                       rmsd.cpp
                       forcefield.cpp
                    """)

PYTHON_CPP=["_ptools.cpp"]
                    
                    
COMMON_LIBS=[]

COMMON_CPPPATH=['.']
               
common=Environment(LIBS=COMMON_LIBS,CPPPATH=COMMON_CPPPATH)
common.Append(CCFLAGS='-Wall -g')


python=common.Copy()
nopython=common.Copy()

objects=common.SharedObject(COMMON_CPP)


python.Append(LIBS=['boost_python', "python2.4"])
#python.Replace(CPPPATH=['.','/usr/include/python2.4'])
python.Append(CPPPATH=['/usr/include/python2.4'])

print "CPPPATH =", python['CPPPATH']

lib1=python.SharedLibrary(File('_ptools.so'),source=[objects,PYTHON_CPP])
lib2=nopython.SharedLibrary('ptools',source=[objects])

Alias('python',lib1)
Alias('cpp',lib2)
print "BUILD_TARGETS is", map(str, BUILD_TARGETS)



#to be able to compile using pyste:
#env = Environment()
#bld = Builder(action = 'pyste --module=_ptools -I. $SOURCE')
#env.Append(BUILDERS = {'Pyste' : bld})

# uncomment this line if pyste is installed
#env.Pyste('ptools.pyste')

#env.SharedLibrary(File('ptools.so'), list, LIBS=libs, CPPPATH=['/usr/include/python2.4/','.'])
