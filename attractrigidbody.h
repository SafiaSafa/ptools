/****************************************************************************
 *   Copyright (C) 2008   Adrien Saladin                                    *
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

#ifndef _ATTRACTRIGIDBODY_H_
#define _ATTRACTRIGIDBODY_H_

#include "rigidbody.h"

namespace PTools{


/*! \brief Rigidbody specific to Attract's reduced model
*
*   this objects stores "atom type number" property
*   which is required for Attract's forcefields and for
*   some attract's specific pairlist
*/
class AttractRigidbody: public Rigidbody
{
public:
    explicit AttractRigidbody(const Rigidbody & rig) ; ///< initilize a new object from a regular Rigidbody object
    AttractRigidbody(){};
    virtual ~AttractRigidbody(){};

    uint getAtomTypeNumber(uint i) const
    {
        return m_atomTypeNumber[i];
    };
    dbl getCharge(uint i) const
    {
        return m_charge[i];
    };

   virtual bool isAtomActive(uint i) const {return (m_atomTypeNumber[i]!=0);  };

    void resetForces()
    {
       m_forces = std::vector<Coord3D> (this->Size() ) ;
    }

    void addForces(std::vector<Coord3D> forces)
    {
      for(uint i=0; i<forces.size(); i++)
         m_forces[i]+=forces[i];
    }

    std::vector<uint> m_activeAtoms; ///< list of active atoms (atoms that are taken into account for interaction)

    ///overload of the virtual function GetCoords
    virtual Coord3D GetCoords(uint i) const
    {
       Coord3D t = Rigidbody::GetCoords(i) ;
       for(uint nmode=0; nmode<m_lambdaMode.size(); nmode++)
       {
          t += m_modesArray[nmode][i] * m_lambdaMode[nmode];
       }

       return t;
    }

    ///adds a mode to the mode list
     void addMode(VCoord3D & mode, dbl eigen);

     ///ask for a certain mode to be applied before getting the coordinates
     void applyMode(uint modenumber, dbl lambda){
         assert(m_lambdaMode.size()> modenumber );
         m_lambdaMode[modenumber] = lambda ;
     }


    ///get the eigenvalue of mode 'i'
    double getEigen(uint i);

    void setRotation(bool value) {hasrotation  = value;} //allow/disallow rotation
    void setTranslation(bool value) {hastranslation = value;} //allow/disallow translation


private:
    std::vector<uint> m_atomTypeNumber ;
    std::vector<dbl> m_charge ;
    std::vector<Coord3D> m_forces ;
    std::vector<dbl> m_lambdaMode ;
    std::vector<dbl> m_eigen; ///< array of eigenvalues

    std::vector<VCoord3D> m_modesArray; ///< array of modes (normal modes)

    bool hastranslation;
    bool hasrotation;

    void normalizeMode(VCoord3D & mode);

friend class BaseAttractForceField;
friend class AttractForceField2;
friend class AttractForceField1;


};  //end class AttractRigid


} //end namespace PTools


#endif


