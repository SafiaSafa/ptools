// $Id$
#ifndef RMSD_H
#define RMSD_H

#include "rigidbody.h"
#include "atomselection.h"
#include "coord3d.h"
#include "screw.h"
#include "basetypes.h"





namespace PTools
{

/// root mean square deviation without superposition
template <class AtomSelection_type>
dbl Rmsd(const AtomSelection_type& atsel1, const AtomSelection_type& atsel2)
{
    if (atsel1.Size() != atsel2.Size())
    {
        std::cerr << "Error: trying to superpose two rigidbody of different sizes" << std::endl ;
        throw std::invalid_argument("RmsdSizesDiffers");
    }

    dbl sum = 0.0;


    for (uint i=0; i<atsel1.Size(); ++i)
    {
        Atom atom1=atsel1.CopyAtom(i);
        Atom atom2=atsel2.CopyAtom(i);

        sum+=Dist2(atom1,atom2);
    }

    return sqrt(sum/(dbl) atsel1.Size()) ;


}


}

#endif //RSMD_H

