/****************************************************************************
 *   Copyright (C) 2006-2008   Adrien Saladin                               *
 *   adrien.saladin@gmail.com                                               *
 *   Copyright (C) 2008   Pierre Poulain                                    *
 *   Copyright (C) 2008   Sebastien Fiorucci                                *
 *   Copyright (C) 2008   Chantal Prevost                                   *
 *   Copyright (C) 2008   Martin Zacharias                                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                          *
 ***************************************************************************/


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




class AtomSelection; // forward declaration

class Rigidbody:private CoordsArray
{

private:

    std::vector<Coord3D> mForces; ///< forces for each atom
    std::string _description; ///< some string to describe the molecule

//    bool isbackbone(const std::string &  atomtype); ///<return true if a given atomtype string matches a backbone atom name

protected:
    std::vector<Atomproperty> mAtomProp; ///< array of atom properties


public:
    /// basic constructor
	Rigidbody();
	/// constructor that loads a PDB file
    Rigidbody(std::string filename);
	/// copy constructor
    Rigidbody(const Rigidbody& model);

    virtual ~Rigidbody(){};

	/// return number of atoms in the rigidbody
    uint size() const {return CoordsArray::size();};

    
    void print_matrix() const {std::cout << CoordsArray::print_matrix() << std::endl; }

    /// make a deep copy of one atom (the atom extracted is then totally independent)
    virtual Atom copy_atom(uint i) const ;

/*    /// const version of GetAtom*/
    /*Atom GetAtom(uint pos) const
    {
        Coord3D co;
        CoordsArray::get_coords(pos, co);
        Atom at(mAtomProp[pos], co );
        return at;
    }*/

    /// return atom properties
    Atomproperty const & get_atom_property(uint pos) const
    {
        return mAtomProp[pos];
    }
	
	/// define atom properties
    void set_atom_property(uint pos, const Atomproperty& atprop)
    {
       mAtomProp[pos] = atprop;
    }

	/// define atom pos
    void set_atom(uint pos, const Atom& atom);

    /// add an atom to the molecule (deep copy)
    void add_atom(const Atomproperty& at, Coord3D co);

    /// add an atom to the molecule
    void add_atom(const Atom& at);

    //returns the coordinates of atom i
    Coord3D get_coords(uint i) const
    {
        assert(i<size());
        Coord3D c;
        CoordsArray::get_coords(i,c) ;  //get the coordinates after translation/rotation

        return c;  //finally returns the final coordinates
    }


    void unsafeget_coords(uint i, Coord3D& co)
      { CoordsArray::unsafeget_coords(i,co); }

    void sync_coords()
    {
      get_coords(0);
    }

	/// define coordinates of atom i
    void set_coords(uint i, const Coord3D& co)
    {
       assert(i<size());
       CoordsArray::set_coords(i,co);
    }

    /// return geometric center of all atoms
    Coord3D find_center() const;

    /// center the rigidbody to the Origin (0,0,0)
    void CenterToOrigin();


    /// translate the whole object
    void Translate(const Coord3D& tr);

    /// apply a 4x4 matrix
    void apply_matrix(const Matrix & mat);

   /// get the 4x4 matrix
   Matrix get_matrix() const
   {
     return CoordsArray::get_matrix();
   }


    /// returns radius of gyration
    dbl radius_of_gyration();

    /// returns the radius of a Rigidbody (max distance from center)
    dbl radius();

    /// converts rigidbody to classical PDB-like string
    std::string print_pdb() const ;

    /// selection : complete
    AtomSelection select_all_atoms() const;

    /// selection : by atom type
    AtomSelection select_atomtype(std::string atomtype);

    /// selection by residue type
    AtomSelection select_restype(std::string residtype);

    /// selection by chain ID
    AtomSelection select_chainid(std::string chainid);

    /// selection by range of residue ID
    AtomSelection select_resid_range(int start, int stop);

    /// selection shortcut for C-alpha
    AtomSelection get_CA();

    /// selection of backbone atoms:
    AtomSelection backbone();

    /// operator + : merge two Rigdibody by extending the first coordinates with the second coordinates.
    Rigidbody operator+ (const Rigidbody& rig);

    void rotate(const Coord3D& A, const Coord3D& B, dbl theta); ///< rotation around (AB) axis.

    /// in some cases atoms may be ignored
    virtual bool isAtomActive(uint i) const {return true;};

    /// set a description for the object (ex: "mutant A192G")
    void setDescription(const std::string & descr) {_description = descr;};
    /// return the object name/description
    std::string getDescription(){return _description;};

    void euler_rotate(dbl phi, dbl ssi, dbl rot);

    //friends
    friend void rotate( Coord3D A, Coord3D B, Rigidbody& target, dbl theta );
    friend void XRotation( const Rigidbody& source, Rigidbody& result, dbl alpha );
    friend void EulerZYZ(const Rigidbody & source, Rigidbody & cible, dbl theta, dbl phi, dbl psi);

    friend class AtomSelection;

    CoordsArray ToCoordsArray() const {return static_cast<CoordsArray> (*this);}
    // undocumented API
    // these functions are candidates for future official functions
    // Please don't use functions beginning by an undersocre '_'
    // they may be removed in future releases without justification

    /* empty section: good news !*/



}; // end class Rigidbody




} // end namespace PTools

#endif //RIGIDBODY_H

