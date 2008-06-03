
#include "surface.h"

#include <cassert>

namespace PTools
{

void Surface::setUp(int nphi, int ncosth)
{
    // dimension csth(ncosth),snth(ncosth),phgh(nphi)

    const dbl pi= 3.141592654;
    //  precalculate costh,sinth and phigh for different ncosth...
    for (uint ik=0; ik < (uint) ncosth; ik++)
    {
        csth.push_back (   -1.0 + ( ( (dbl) ik+1) - 0.5  ) * (2.0 / (dbl) ncosth ) ) ;
        snth.push_back (   sqrt(1.0 -csth[ik]*csth[ik] ) );
    }

    for ( int ik=0; ik<nphi; ik++)
    {
        cos_phgh.push_back ( cos(2.0 * pi *  ((dbl)ik-1.0)/(dbl) nphi )) ;
        sin_phgh.push_back ( sin(2.0 * pi *  ((dbl)ik-1.0)/(dbl) nphi )) ;
    }


    std::ifstream fsolv ("aminon.par");
    if (!fsolv)
    {
        //  the file cannot be opened
        std::string msg = " Cannot Locate file  solv.dat \n"  ;
        std::cout << msg ;
        throw msg;
    }

    int somedata1;
    dbl trad, somedata2;
    std::string somedata3;
    for (int i=0; i<29 ; i++)
    {
        fsolv >> somedata1 >> trad >> somedata2 >> somedata3 ;
        radi.push_back(trad);
    }
}

void Surface::surfpointParams(int max, dbl shift)
{
    m_numneh = max;
    m_sradshift = shift;
    m_init=true;
}

Rigidbody Surface::surfpoint(const Rigidbody & rigid, dbl srad)
{
    Rigidbody rigidsurf;
    int size_rigid = rigid.Size();
    std::vector<int> neigh;
    radius.clear();

    // fix neighbours parameters if not initialized
    if (!m_init)
    {
        m_numneh = 2000;
        m_sradshift = 0.0;
    }
    // read radius
    AttractRigidbody rigid_tmp=AttractRigidbody(rigid);
    m_atomtypenumber.resize(size_rigid);
    for (uint i=0; i< rigid_tmp.Size(); i++)
    { m_atomtypenumber[i] = rigid_tmp.getAtomTypeNumber(i);}
    for (int i=0; i<size_rigid; i++)
    {
        Atom m_atom = rigid_tmp.CopyAtom(i);
        radius.push_back(radi[m_atomtypenumber[i]]);
    }

    // generate grid points
    for (int i=0; i<size_rigid; i++)
        if ( radius[i] != 0.0 )
        {
            Coord3D coord1 = rigid.GetCoords(i);
            int numneh = 0;
            neigh.clear();
            for (int j=0; j<size_rigid; j++) // generate neighbor list
                if (i!=j)
                {
                    Coord3D coord2 = rigid.GetCoords(j);
                    dbl ccdist = Norm2(coord1 - coord2);
                    dbl rr = (radius[i]+radius[j]+2.0*srad) * (radius[i]+radius[j]+2.0*srad);
                    if (ccdist <= rr)
                    { neigh.push_back(j);
                        numneh+=1;
                        if  ( numneh > m_numneh )
                        {
                            std::string msg = " ERROR: Atom has too many neighbors \n"  ;
                            std::cout << msg;
                            throw msg;
                        }
                    }
                }
            numneh = numneh - 1;
            for (int j=0; j<m_ncosth; j++) // generate points around each atoms
            {
                dbl costh = csth[j];
                dbl sinth = snth[j];
                for (int k=0; k<m_nphi; k++)
                {
                    dbl cphigh = cos_phgh[k];
                    dbl sphigh = sin_phgh[k];
                    Coord3D coord4;
                    coord4.x = (radius[i]+srad+m_sradshift)*sinth*cphigh;
                    coord4.y = (radius[i]+srad+m_sradshift)*sinth*sphigh;
                    coord4.z = (radius[i]+srad+m_sradshift)*costh;
                    int l = 0;
                    bool coverd = false;
                    while ((!coverd) && (l <= numneh))
                    {
                        Coord3D coord5 = rigid.GetCoords(neigh[l]);
                        dbl ddd = Norm2(coord1 + coord4 - coord5);
                        if (ddd < (radius[neigh[l]] + srad+m_sradshift)*(radius[neigh[l]] + srad+m_sradshift))
                        { coverd = true; }
                        l+=1;
                    }
                    if (!coverd)
                    {
                        Atom m_atom2 = rigid.GetAtom(i);
                        m_atom2.SetCoords(coord1 + coord4);
                        rigidsurf.AddAtom(m_atom2);
                    }
                }
            }

            // fill the top and bottom positions
            dbl costh = -1;
            while (costh<=1.0)
            {
                Coord3D coord4;
                coord4.x = 0.0;
                coord4.y = 0.0;
                coord4.z = (radius[i]+srad+m_sradshift)*costh;
                int l = 0;
                bool coverd = false;
                while ((!coverd) && (l <= numneh))
                {
                    Coord3D coord5 = rigid.GetCoords(neigh[l]);
                    dbl ddd = Norm2(coord1 + coord4 - coord5);
                    if (ddd < (radius[neigh[l]] + srad+m_sradshift)*(radius[neigh[l]] + srad+m_sradshift))
                    { coverd = true; }
                    l+=1;
                }
                if (!coverd)
                {
                    Atom m_atom2 = rigid.GetAtom(i);
                    m_atom2.SetCoords(coord1 + coord4);
                    rigidsurf.AddAtom(m_atom2);
                }
                costh+=2;
            }
        }
    return rigidsurf;
}

Rigidbody Surface::outergrid(const Rigidbody & rigid1, const Rigidbody & rigid2, dbl srad)
{
    int size1 = rigid1.Size();
    int size2 = rigid2.Size();
    Rigidbody rigid3;
    for (int i=0; i<size1; i++)
    {
        Coord3D xyz1 = rigid1.GetCoords(i);
        bool select = true;
        for (int j=0; j<size2; j++)
        {
            Coord3D xyz2 = rigid2.GetCoords(j);
            dbl dist=Norm2(xyz1-xyz2);
            if (dist < srad) { select = false; }
        }
        if (select) { rigid3.AddAtom(rigid1.GetAtom(i)); }
    }
    return rigid3;
}

Rigidbody Surface::removeclosest(const Rigidbody & rigid, dbl srad)
{
    std::vector<bool> list,list2;
    int size=rigid.Size();
    Rigidbody rigid2;
    list.clear();
    for (int i=0; i<size; i++) { list.push_back(true); }
    for (int i=0; i<size; i++)
    {
        Coord3D  xyz1 = rigid.GetCoords(i);
        for (int j=0; j<size; j++)
            if ((list[i]) && (i!=j))
            {
                Coord3D  xyz2 = rigid.GetCoords(j);
                dbl dist=Norm2(xyz1 - xyz2);
                if (dist < srad) { list[j] = false; }
            }
    }
    for (int i=0; i<size; i++)
    if (list[i]) { rigid2.AddAtom(rigid.GetAtom(i)); }
    return rigid2;
}

}//namespace PTools
