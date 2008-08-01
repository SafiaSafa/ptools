#!/usr/bin/env python

from ptools import *
import sys

def selectListOfResidues(rigidbody, lst):
    "takes a rigidbody and a list of residues and returns an AtomSelection object"
    atsel=AtomSelection()
    atsel.SetRigid(rigidbody)
    for i in lst:
        sel = rigidbody.SelectResRange(i,i)
        atsel = atsel | sel
    return atsel

def irmsd(receptor, ligref, ligprobe, receptorprobe=None,reducedmodel=False):
    """calculates I-Rmsd.
     receptorprobe may be defined if != receptor (mobile/flexible receptor)
     use of a reduced model may change the cutoff.
     
     I-rmsd as defined here: 
     Mendez et al.
     PROTEINS: Structure, Function and Genetics 52:51-67 (2003)
     Assessment of Blind Predictions of Protein-Protein Interactions: Current Status of Docking Methods.
     

    """
    cutoff = 10.0 
    if reducedmodel: cutoff = 2.0*7.0  #twice the threshold for residue-residue contacts    
    
    recBB = receptor.Backbone().CreateRigid() #receptor backbone
    ligrefBB = ligref.Backbone().CreateRigid()
    
    #creating list of residues in interaction:
    recResidues={}
    ligResidues={}
    pairlist = AttractPairList(receptor, ligref, cutoff) #pairlist created on protein backbone + side-chains
    for i in range(pairlist.Size()):
        atompair = pairlist[i]
        ligindex = atompair.atlig
	recindex = atompair.atrec
	
        atom = ligref.CopyAtom(ligindex)
        ligResidues[ atom.GetResidId() ]=1

	atom = receptor.CopyAtom(recindex)
	recResidues[ atom.GetResidId() ]=1

    ligResidues = sorted(ligResidues.keys())  #get a list of the ligand's residues in interaction
    recResidues = sorted(recResidues.keys())

    ligrefBBInterface=selectListOfResidues(ligrefBB,ligResidues) #interface backbone residues of reference ligand
    ligBBInterface = ligprobe.Backbone() & selectListOfResidues(ligprobe,ligResidues) #interface bb residues of docked ligand
    receptorBBInterface=selectListOfResidues(recBB,recResidues)
    
    ligrefpdb=ligrefBBInterface.CreateRigid()
    ligdockpdb=ligBBInterface.CreateRigid()
    recrefpdb=receptorBBInterface.CreateRigid()
    
    ref=Rigidbody(ligrefpdb+recrefpdb)
    pred=Rigidbody(ligdockpdb+recrefpdb)
    super= superpose(ref,pred,1)
    mat=super.matrix
    pred.ApplyMatrix(mat)
    assert(ligrefBBInterface.Size()==ligBBInterface.Size())

    return Rmsd(pred,ref)
    #return Rmsd(ligrefBBInterface, ligBBInterface)


def main():

    recname = sys.argv[1]
    ligname = sys.argv[2]
    ligname2 = sys.argv[3]
    lig = Rigidbody(ligname)
    lig2 = Rigidbody(ligname2)
    rec = Rigidbody(recname)

    IRMSD=irmsd(rec,lig,lig2,None,True)
    print IRMSD

if __name__ == "__main__":
    main()
