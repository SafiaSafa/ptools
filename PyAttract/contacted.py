#!/usr/bin env python

#return a list of contacted residues on the receptor

from ptools import *
import sys

import argparse


def contacted(receptor, ligand):
    resnbrec = []  #resnbrec[i] is the residue id of atom i
    for j in xrange(len(receptor)):
        at = receptor.CopyAtom(j)
        resnbrec.append(at.residId)
    
    
    pl = AttractPairList(receptor,ligand, args.cutoff)
    
    
    atomrec = [i.atrec for i in pl]  #list of contacted atoms on the receptor
    residusrec = sorted(list({resnbrec[i] for i in atomrec})) #list of contacted residues on the receptor
    
    return residusrec


def main(args):
    receptor = AttractRigidbody(args.receptor)
    ligand = AttractRigidbody(args.ligand)
        
    
    if args.model:
        model = AttractRigidbody(args.model)
        natcont = contacted(receptor, model)
        
        newcont = contacted(receptor, ligand)
        
        kept_in_new_pose = [i for i in natcont if i in newcont]  #intersection of the two lists (would be better with sets)
        print float(len(kept_in_new_pose))/float(len(natcont))*100
        
        
    else:
    
        for r in contacted(receptor, ligand):
            print r,
            
        print ""



if __name__=="__main__":
    
    parser = argparse.ArgumentParser(description="calculate a list of contacted residues on the receptor")
    parser.add_argument("-r", "--receptor", dest="receptor")
    parser.add_argument("-l", "--ligand", dest="ligand")
    parser.add_argument("--cutoff", dest="cutoff", type=int, default=7, help="cutoff used to determine if two residues interact. Default to 7A (for coarse grained templates). You should set the cutoff to 5 when working on all atoms molecules")
    parser.add_argument('-m', '--model', dest="model", default=None, help="ligand model (ie correct bound solution) to calculate the percentage of contacted residues on the receptor")
    
    
    args = parser.parse_args()
    
    main(args)