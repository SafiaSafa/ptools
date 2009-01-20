// $Id$
#ifndef GEOMETRY
#define GEOMETRY


#include "rigidbody.h"

#include <iostream>
#include <string.h> //for memcpy


namespace PTools{

// class Rigidbody;

typedef dbl Mat44[4][4];

void MakeRotationMatrix(Coord3D A, Coord3D B, dbl theta,  dbl out[4][4]);
// void XRotation(const Rigidbody& source, Rigidbody &result,  dbl alpha); // rotation autour de l'axe X

inline void mat44xVect(const dbl mat[ 4 ][ 4 ], const Coord3D& vect, Coord3D& out )
{
    out.x = vect.x * mat[ 0 ][ 0 ] + vect.y * mat[ 0 ][ 1 ] + vect.z * mat[ 0 ][ 2 ] + mat[ 0 ][ 3 ] ;
    out.y = vect.x * mat[ 1 ][ 0 ] + vect.y * mat[ 1 ][ 1 ] + vect.z * mat[ 1 ][ 2 ] + mat[ 1 ][ 3 ] ;
    out.z = vect.x * mat[ 2 ][ 0 ] + vect.y * mat[ 2 ][ 1 ] + vect.z * mat[ 2 ][ 2 ] + mat[ 2 ][ 3 ] ;
}

/*
//cannot find implementation for this:
template <class T>
void mat44xrigid(const Rigidbody<T>& source, Rigidbody<T>& result, dbl mat[4][4]);*/


template <class Rigid_type>
void freeABrotate(Coord3D A, Coord3D B, Rigid_type& target, dbl theta)
{

    dbl matrix[ 4 ][ 4 ];
    MakeRotationMatrix( A, B, theta, matrix );
    target.MatrixMultiply(matrix);
}



// void EulerZYZ(const Rigidbody & source, Rigidbody & cible, dbl theta, dbl phi, dbl psi);
// void AttractEuler(const Rigidbody& source, Rigidbody& dest, dbl phi, dbl ssi, dbl rot);

void mat44xmat44( const dbl mat1[4][4], const dbl mat2[4][4], dbl result[4][4] );
//void MultMat4x4(dbl R1[4][4], dbl R2[4][4], dbl out[4][4]);

///vectorial product
void VectProd(const Coord3D& u,const Coord3D& v, Coord3D& UvectV);

void printmat44(const dbl mat[4][4]);

///returns the scalar product between two Coord3D object
inline dbl ScalProd( const Coord3D& a, const Coord3D& b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z ;
}

dbl Dihedral(const Coord3D& a, const Coord3D& b, const Coord3D& c, const Coord3D& d);

dbl Angle(const Coord3D& vector1, const Coord3D& vector2);

dbl MakeTranslationMat44(Coord3D t, dbl out[4][4] );

#endif  //ifndef GEOMETRY

}


