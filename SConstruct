list = Split ("""atom.cpp
		 rigidbody.cpp
		 ptools.cpp
                 pdbparser.cpp
             """)


libs = Split("""boost_python
		python2.4""")

#to be able to compile using pyste:
env = Environment()
bld = Builder(action = 'pyste --module=ptools -I. $SOURCE')
env.Append(BUILDERS = {'Pyste' : bld})


env.Pyste('ptools.pyste')

env.SharedLibrary(File('ptools.so'), list, LIBS=libs, CPPPATH=['/usr/include/python2.4/','.'])
