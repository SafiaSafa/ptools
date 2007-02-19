#ifndef RIGIDBODY_H
#define RIGIDBODY_H


#include <boost/shared_ptr.hpp>
#include <vector>

#include "coord3d.h"
#include "atom.h"

namespace PTools {


class AtomSelection; // forward declaration

class Rigidbody
{

private:
//    std::vector<Atomproperty>  mArrayAtomProp; ///< array of atom properties
//	std::vector<Coord3D>  mCoords; ///< Array of cartesian coordinates
    std::vector<Atom> mAtoms; ///< vector of Atoms
    

public:
    Rigidbody(){};
    ~Rigidbody(){};

    Atom CopyAtom(uint i) const ;
    /// Make a deep copy of one atom (the Atom extracted is then totally independent)

    Atom& GetAtom(uint pos) {return mAtoms[pos];}; 
    ///  get the Atom of index i (with direct access to modifications)
    Atom& operator[](uint pos) {return GetAtom(pos);}; ///< synonym of GetAtom

    inline uint Size() const {return mAtoms.size();};
    /// return number of atoms

    void AddAtom(const Atomproperty& at, Coord3D co);
    /// add an atom to the molecule (deep copy)

    void AddAtom(const Atom& at);
    /// add an atom to the molecule

    Coord3D GetCoords(uint i) const {return mAtoms[i].GetCoords();};
    /// returns the coordinates of atom i

    Coord3D GetCoordsFromType(std::string);
    /// returns the coordinates of the first atom of type t

    void SetCoords(uint i, Coord3D& co){mAtoms[i].SetCoords(co);};
    /// define the coordinates of atom i

    Coord3D FindCenter();
    /// return geometric center of all atoms

    void Translate(const Coord3D& tr); ///< Translate the whole object

    };

}

#endif //RIGIDBODY_H

