#! /usr/bin/env python

# reads .trj file (output from attract in single mode)
# and a reduced ligand file
# to create a multi-pdb file from trajectory


import sys
from ptools import *

if len(sys.argv) < 3:
    sys.exit("""ERROR: missing argument
Usage: applytraj.py trajectory_file ligand_file > ligand_multi_pdb""")


trjfile = open(sys.argv[1])
ligand = Rigidbody(sys.argv[2])

#reads trj file:

lines = trjfile.readlines()

counter = 0
trjpdb = ""

for l in lines:
    if "~~~" in l:
        ligand = output
    else:
        counter+=1
        output=AttractRigidbody(ligand)
        center=output.FindCenter()
        output.Translate(Coord3D()-center)
        X = l.split()
        X = [float(i) for i in X]
        output.AttractEulerRotate(surreal(X[0]), surreal(X[1]), surreal(X[2]))
        output.Translate(Coord3D(surreal(X[3]),surreal(X[4]),surreal(X[5])))
        output.Translate(center)
        trjpdb += "MODEL %d \n" %(counter)
        trjpdb += output.PrintPDB()
        trjpdb += "ENDMDL \n"
print trjpdb
