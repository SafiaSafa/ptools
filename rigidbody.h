#ifndef RIGIDBODY_H
#define RIGIDBODY_H


#include <vector>
#include <cassert>


#include "coord3d.h"
#include "atom.h"
#include "basetypes.h"
#include "coordsarray.h"


namespace PTools
{


namespace {
// anonymous namespace for copying the mat44xVect from geometry.h
// this is a pretty ugly hack and if someone finds a better way, then please do it !

inline void mat44xVect(const dbl mat[ 4 ][ 4 ], const Coord3D& vect, Coord3D& out )
{
    out.x = vect.x * mat[ 0 ][ 0 ] + vect.y * mat[ 0 ][ 1 ] + vect.z * mat[ 0 ][ 2 ] + mat[ 0 ][ 3 ] ;
    out.y = vect.x * mat[ 1 ][ 0 ] + vect.y * mat[ 1 ][ 1 ] + vect.z * mat[ 1 ][ 2 ] + mat[ 1 ][ 3 ] ;
    out.z = vect.x * mat[ 2 ][ 0 ] + vect.y * mat[ 2 ][ 1 ] + vect.z * mat[ 2 ][ 2 ] + mat[ 2 ][ 3 ] ;
}

}








class AtomSelection; // forward declaration

class Rigidbody:public CoordsArray
{

private:

    std::vector<Atom> mAtoms; ///< vector of Atoms
    std::vector<Coord3D> mForces; ///< forces for each atom

    virtual void m_hookCoords(uint i, Coord3D & co) const ; ///< private hook to give a chance for class childs to change the coordinates before rotations and translations


protected:
    std::vector<Atomproperty> mAtomProp; ///< array of atom properties


public:
    Rigidbody();  ///< basic constructor
    Rigidbody(std::string filename);
    Rigidbody(const Rigidbody& model); ///< copy

    virtual ~Rigidbody(){};

    /// Make a deep copy of one atom (the Atom extracted is then totally independent)
    virtual Atom CopyAtom(uint i) const ;

    /// const version of GetAtom
    Atom GetAtom(uint pos) const
    {
        Coord3D co;
        CoordsArray::GetCoords(pos, co);
        Atom at(mAtomProp[pos], co );
        return at;
    }


    /// const version of GetAtom
    Atomproperty const & GetAtomProperty(uint pos) const
    {
        return mAtomProp[pos];
    }



    /// return number of atoms
//     inline uint Size() const
//     {
//         return mAtomProp.size();
//     };

    /// add an atom to the molecule (deep copy)
    void AddAtom(const Atomproperty& at, Coord3D co);

    /// add an atom to the molecule
    void AddAtom(const Atom& at);

    //returns the coordinates of atom i
    Coord3D GetCoords(uint i) const
    {
        assert(i<Size());
        Coord3D c;
        CoordsArray::GetCoords(i,c) ;  //get the unrotated-untranslated coordinates

        return c;  //finally returns the final coordinates
    };


    /// return geometric center of all atoms
    Coord3D FindCenter() const;

    /// center the rigidbody to the Origin (0,0,0)
    void CenterToOrigin();

    void Translate(const Coord3D& tr); ///< Translate the whole object

    void AttractEulerRotate(dbl phi, dbl ssi, dbl rot); ///< Do an euler rotation (Attract convention)

    ///Radius of gyration
    dbl RadiusGyration();

    /// returns the radius of a Rigidbody (max distance from center)
    dbl Radius();

    /// converts rigidbody to classical PDB-like string
    std::string PrintPDB() const ;

    /// selection : complete
    AtomSelection SelectAllAtoms();

    /// selection : by atom type
    AtomSelection SelectAtomType(std::string atomtype);

    /// selection by residue type
    AtomSelection SelectResidType(std::string residtype);

    /// selection by chain ID
    AtomSelection SelectChainId(std::string chainid);

    /// selection by range of residue ID
    AtomSelection SelectResRange(uint start, uint stop);

    /// selection shortcut for C-alpha
    AtomSelection CA();

    // operator =
    // TODO: one should check but this operator doesn't seem to be that necessary
    // (the compilers can use Rigidbody(const Rigidbody&) instead)
    //Rigidbody& operator= (const Rigidbody& rig);

    /// operator + : merge two Rigdibody by extending the first coordinates with the second coordinates.
    Rigidbody operator+ (const Rigidbody& rig);

    void ABrotate(const Coord3D& A, const Coord3D& B, dbl theta); ///< rotation around (AB) axis.

    /// in some cases atoms may be ignored
    virtual bool isAtomActive(uint i) const {return true;};


    //friends
    friend void ABrotate( Coord3D A, Coord3D B, Rigidbody& target, dbl theta );
    friend void XRotation( const Rigidbody& source, Rigidbody& result, dbl alpha );
    friend void EulerZYZ(const Rigidbody & source, Rigidbody & cible, dbl theta, dbl phi, dbl psi);

    friend class AtomSelection;

}; // end class Rigidbody




} // end namespace PTools

#endif //RIGIDBODY_H

