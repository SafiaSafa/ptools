#include "rigidbody.h"
#include "atomselection.h"
#include "geometry.h"
#include "pdbio.h"


namespace PTools{


Rigidbody::Rigidbody()
{
    ResetMatrix();
}



Rigidbody::Rigidbody(std::string filename)
{
    ReadPDB(filename,*this);
    ResetMatrix();
}


Rigidbody::Rigidbody(const Rigidbody& model)
        : CoordsArray(model)
{
//this copy constructor is needed because dbl[4][4] is not
// automatically copied with the default copy constructor
//TODO: verifier si c'est toujours le cas ...


//     this->mAtoms = model.mAtoms;
    this->mForces = model.mForces;
//     this->_coords = model._coords;
    this->mAtomProp = model.mAtomProp;
    this->mAtoms = model.mAtoms;
    this-> _description = model._description;


}


void Rigidbody::AddAtom(const Atomproperty& at, Coord3D co)
{
    mAtomProp.push_back(at);
    AddCoord(co);
}


Atom Rigidbody::CopyAtom(uint i) const
{
    Atom at(mAtomProp[i],GetCoords(i));
    return at;
}


void Rigidbody::AddAtom(const Atom& at)
{
    Atomproperty atp(at);
    Coord3D co = at.GetCoords();
    AddAtom(atp,co);
}


Coord3D Rigidbody::FindCenter() const
{
    Coord3D center(0.0,0.0,0.0);
    for (uint i=0; i< this->Size() ; i++)
    {
        center =  center + GetCoords(i);
    }
    return ( (1.0/(dbl)this->Size())*center);
}


void Rigidbody::CenterToOrigin()
{
    Coord3D c = FindCenter();
    Translate(Coord3D()-c);
}

dbl Rigidbody::RadiusGyration()
{
    Coord3D c = this->FindCenter();
    dbl r=0.0;
    for (uint i=0; i< this->Size(); i++)
    {
        r += Norm2( c - this->GetCoords(i) );
    }

    dbl result = sqrt( r/ (double) this->Size() );
    return result;
}

dbl Rigidbody::Radius()
{
    Coord3D center = this->FindCenter();
    uint size = this->Size();
    dbl radius = 0.0;
    for (uint i=0; i < size; i++)
    {
        dbl rad=Norm(center - this->GetCoords(i));
        if (radius < rad) {radius=rad;}
    }
    return radius;
}


void Rigidbody::Translate(const Coord3D& tr)
{
    CoordsArray::Translate(tr);
}

void Rigidbody::AttractEulerRotate(dbl phi, dbl ssi, dbl rot)
{
   CoordsArray::AttractEulerRotate(phi, ssi, rot);
}





AtomSelection Rigidbody::SelectAllAtoms()
{
    AtomSelection newsel;
    newsel.SetRigid(*this);
    for (uint i=0; i < Size(); i++)
    {
        newsel.AddAtomIndex(i);
    }


    return newsel;

}


AtomSelection Rigidbody::SelectAtomType(std::string atomtype)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i<Size(); i++)
    {
        if ( mAtomProp[i].GetType()==atomtype)
            newsel.AddAtomIndex(i);
    }

    return newsel;
}


AtomSelection Rigidbody::SelectResidType(std::string residtype)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i<Size(); i++)
    {
        if (mAtomProp[i].GetResidType()==residtype)
            newsel.AddAtomIndex(i);
    }
    return newsel;
}


AtomSelection Rigidbody::SelectChainId(std::string chainId) {
    AtomSelection newsel;
    newsel.SetRigid(*this);
    for (uint i=0; i<Size(); i++)
    {
        if (mAtomProp[i].GetChainId()==chainId)
            newsel.AddAtomIndex(i);
    }
    return newsel;
}

AtomSelection Rigidbody::SelectResRange(uint start, uint stop)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i < Size(); i++)
    {
        Atomproperty& atp ( mAtomProp[i] );
        if (atp.GetResidId() >=start && atp.GetResidId() <= stop) newsel.AddAtomIndex(i);
    }
    return newsel;
}


AtomSelection Rigidbody::CA() {
    return SelectAtomType("CA");
}


/// operator +
Rigidbody Rigidbody::operator+(const Rigidbody& rig) {
    Rigidbody rigFinal(*this);
    for (uint i=0; i< rig.Size() ; i++) {
        rigFinal.AddCoord(rig.GetCoords(i));
        rigFinal.mAtomProp.push_back(rig.mAtomProp[i]);
    }
    return rigFinal;
}


void Rigidbody::ABrotate(const Coord3D& A, const Coord3D& B, dbl theta)
{
    PTools::ABrotate(A,B, *this, theta);
}



std::string Rigidbody::PrintPDB() const
{
    uint size=this->Size();
    std::string output;
    for (uint i=0; i < size ; i++)
    {
        output = output + mAtoms[i].ToPdbString();
    }
    return(std::string) output;
}

void Rigidbody::m_hookCoords(uint i, Coord3D & co) const {}  //(virtual)


void Rigidbody::ApplyMatrix(const Matrix& mat)
{

   dbl mat44[4][4];
   for(uint i=0; i<4;i++)
    for(uint j=0;j<4;j++)
      mat44[i][j]=mat(i,j);
   CoordsArray::MatrixMultiply(mat44);
}




} //namespace PTools
