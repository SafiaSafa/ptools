#include "forcefield.h"
#include "geometry.h"
#include "rmsd.h"
#include "pdbio.h"


#include <fstream>
#include <math.h>  //for fabs()



//temporaire:
//convertit une chaine de caracteres std::string en un autre type
// si >> a ete correctement defini.
#include <sstream>
template<typename T>
bool from_string( const std::string & Str, T & Dest )
{
    // cr�er un flux � partir de la cha�ne donn�e
    std::istringstream iss( Str );
    // tenter la conversion vers Dest
    return (iss >> Dest) != 0;
}





namespace PTools
{



void PrintVec(const Vdouble& vec)
{
    std::cout << "Vector: " ;
    for (uint i=0; i<vec.size(); i++)
        std::cout << vec[i] << "  " ;

    std::cout << "\n";

}


/*! \brief Extracts extra information from ATOM lines.
 *
 *  For Attract pdb files, the library currently reads some extra informations
*  after the x,y,z coordinates. This information is extracted here.
*  Two arrays are populated then: atcategory which contains the atom type category
* (AKA iaci variable in the fortran code) and the atom charge ('xlai' in the fortran code)
 */
void extractExtra( Rigidbody& rig, std::vector<uint>& vCat, std::vector<double>& vCh)
{

    uint   atcategory  = 0;
    double  atcharge   = 0.0;

    for (uint i=0; i<rig.Size(); i++)
    {
        const Atomproperty at = rig.GetAtomProperty(i);
        std::string extra = at.GetExtra();
        //std::cout << extra << std::endl;
        std::istringstream iss( extra );
        iss >> atcategory >> atcharge ;
        vCat.push_back(atcategory-1);
        vCh.push_back(atcharge);
    }


}

int AttractForceField::_minimnb=0;  // static initialization


AttractForceField::AttractForceField(const Rigidbody& recept,const Rigidbody& lig,double cutoff)
        :m_refreceptor(recept), m_refligand(lig), m_receptor(recept), m_ligand(lig),m_savligand(lig),
        plist(recept,lig,cutoff)
{
    m_ligcenter = m_ligand.FindCenter();
    m_refligand.Translate(PTools::minus(m_ligcenter)); //center the ligand (for later Euler rotation)

    InitParams();

    for (uint i=0;i<m_refligand.Size();i++)
        m_ligforces.push_back(Coord3D()); // initialize the forces array

    // reads the "extra" informations of each atoms of ligand and receptor
    // and extracts the "atom category" from this fields
    extractExtra(m_refreceptor, m_rAtomCat, m_rAtomCharge);
    extractExtra(m_refligand,   m_lAtomCat, m_lAtomCharge);

    m_energycalled=false; //initialization

    m_rstk=0.0; //no restraint by default


    // find the closest atom of the ligand to the receptor
    // (for the constraint force)
    double mindist2 = 1.0e8;
    m_ligRestraintIndex = -1;
    m_reccenter = m_receptor.FindCenter();
    for (uint ilig=0; ilig<lig.Size(); ilig++)
    {
        Coord3D atlig = lig.GetCoords(ilig);
        if (Norm2(atlig-m_reccenter) < mindist2)
        {
            mindist2 = Norm2(atlig-m_reccenter) ;
            m_ligRestraintIndex = ilig ;
        }
    }
}


static bool numerical_warning=false;
void ForceField::NumDerivatives(const Vdouble& stateVars, Vdouble& delta, bool print)
{

    if (!numerical_warning)
    {
        std::cout << "Warning: using numerical derivatives in production may lead to poor performances\n" ;
        numerical_warning=true;
    }
    for (uint j=0; j<ProblemSize(); j++)
    {

        Vdouble newvars1 = stateVars;
        Vdouble newvars2 = stateVars;



        double h=1e-3;

        newvars1[j]+=h;
        double F1=Function(newvars1);
        newvars2[j]-=h;
        double F2=Function(newvars2);
        double diff=(F1-F2)/(2*h) ;
        delta[j]=diff;
    }

    if (print)
    {
        std::cout << "Numerical derivatives: \n";
        for (uint i=0; i<ProblemSize(); i++)
        {
            std::cout << delta[i] << "  " ;
        }
        std::cout << "\n";

    }
}





double AttractForceField::Energy()
{
    ResetForces();
    double energy = LennardJones() +  Electrostatic();
    energy+=Constraints(); //(if hte order of evaluation is important, then it should not be merged with the precedent line)
    m_energycalled=true;
    return energy;
};



double AttractForceField::Energy(const Vdouble& stateVars)
{
    AttractEuler(m_refligand, m_ligand, stateVars[0], stateVars[1], stateVars[2]);
    m_ligand.Translate(m_ligcenter);
    m_ligand.Translate(Coord3D(stateVars[3],stateVars[4],stateVars[5]));
    return Energy();
}


void AttractForceField::InitParams()
{
    int indice, inull;
    double rad;
    double amp;

    std::ifstream aminon("aminon.par");
    if (!aminon)
    {
        //the file cannot be opened
        std::string msg = "Forcefield.cpp: Cannot Locate file forcefield parameters (aminon.par)\n";
        std::cout << msg ;
        throw msg;
    }

    while (!aminon.eof())
    {
        aminon >> indice >> rad >> amp >> inull ;
        m_rad.push_back(rad) ;
        m_amp.push_back(amp) ;
    }

    assert(m_rad.size()<40);


    //initialisation of the pre-calculated array of rc and ac
    for (uint i=0; i<m_rad.size();i++)
        for (uint j=0; j<m_rad.size(); j++)
        {
            m_rc[i][j]=m_amp[i]*m_amp[j]*pow((m_rad[i]+m_rad[j]),8);
            m_ac[i][j]=m_amp[i]*m_amp[j]*pow((m_rad[i]+m_rad[j]),6);
        }
}



double AttractForceField::LennardJones()
{

    double sumLJ=0.0 ;



    for (uint iter=0; iter<plist.Size(); iter++)
    {

        uint ir = plist[iter].atrec;
        uint jl = plist[iter].atlig;
        assert(m_rAtomCat[ir] >= 0);
        assert(m_rAtomCat[ir] < m_rad.size());
        assert(m_lAtomCat[jl] >= 0);
        assert(m_lAtomCat[jl] < m_rad.size());


        double alen = m_ac[ m_rAtomCat[ir] ][ m_lAtomCat[jl] ];
        double rlen = m_rc[ m_rAtomCat[ir] ][ m_lAtomCat[jl] ];

        Coord3D dx = m_ligand.GetCoords(jl) - m_receptor.GetCoords(ir) ;
        double r2 = Norm2(dx);

        if (r2 < 0.001 ) r2=0.001;
        double rr2 = 1.0/r2;
        dx = rr2*dx;

        double rr23 = rr2*rr2*rr2 ;
        double rep =  rlen*rr2 ;
        double vlj = (rep-alen)*rr23 ;

        sumLJ += vlj;

        double fb = 6.0*vlj+2.0*(rep*rr23) ;
        Coord3D fdb = fb*dx ;
        m_ligforces[jl] = m_ligforces[jl] + fdb ;
        //receptor%forces%data(i) = receptor%forces%data(i) - fdb
    }
    return sumLJ ;
}





double AttractForceField::Constraints()
{
    if (m_rstk==0.0) return 0.0;

    //calculates the distance between the receptor center and the ligand surface:
    Coord3D ligRestraintCoords = m_ligand.GetCoords(m_ligRestraintIndex) ;
    Coord3D vecLig2Rec = m_reccenter - ligRestraintCoords ;

    double ett = Norm2(vecLig2Rec) ;

    //rstk: user-defined constant
    Coord3D springforce = 4 * m_rstk * ett * vecLig2Rec ;
    //adds force to the correct ligand atom:
    m_ligforces[m_ligRestraintIndex]=m_ligforces[m_ligRestraintIndex]+springforce;

    double ener =  m_rstk * ett * ett ;
    //std::cout << "Constraint energy: " << ener << std::endl;

    return ener;
}







double AttractForceField::Electrostatic()
{
    double sumElectrostatic=0.0 ;

    for (uint iter=0; iter<plist.Size(); iter++)
    {


        uint ir = plist[iter].atrec;
        uint jl = plist[iter].atlig;
        double chargeR = m_rAtomCharge[ir];
        double chargeL = m_lAtomCharge[jl];
        double charge = chargeR * chargeL * (332.053986/20.0);

        if (fabs(charge) > 0.0)
        {



            Coord3D dx = m_ligand.GetCoords(jl) - m_receptor.GetCoords(ir) ;
            double r2 = Norm2(dx);

            if (r2 < 0.001 ) r2=0.001;
            double rr2 = 1.0/r2;
            dx = rr2*dx;



            double et = charge*rr2;
            sumElectrostatic+=et;

            Coord3D fdb = (2.0*et)*dx;
            m_ligforces[jl] = m_ligforces[jl] + fdb ;
        }
    }
    return  sumElectrostatic ;
}



void AttractForceField::Gradient(const Vdouble& stateVars, Vdouble& delta)
{
    // !!!!!!!!!! WARNING: this function NEEDS Energy() to be called FIRST !!!!!!!!!!!!!!

    //std::cout << "size of stateVars:" << stateVars.size() << std::endl ;
    assert(stateVars.size()>=ProblemSize());
    assert(delta.size()>=ProblemSize());

    Trans(delta);
    Rota(stateVars[0], stateVars[1], stateVars[2], delta);

    ResetForces();
    m_energycalled = false;
    // return 1-delta:
    for (uint i=0; i<6;i++) delta[i]=-delta[i];
}


void AttractForceField::Trans(Vdouble& delta,bool print)
{
//   In this subroutine the translational force components are calculated
    double flim = 1.0e18;
    double ftr1, ftr2, ftr3, fbetr;

    ftr1=0.0;
    ftr2=0.0;
    ftr3=0.0;
    for (uint i=0;i<m_ligand.Size(); i++)
    {
        ftr1=ftr1 + m_ligforces[i].x;
        ftr2=ftr2 + m_ligforces[i].y;
        ftr3=ftr3 + m_ligforces[i].z;
    }

// force reduction, some times helps in case of very "bad" start structure
    for (uint i=0; i<3; i++)
    {
        fbetr=ftr1*ftr1 +ftr2*ftr2 +ftr3*ftr3;
        if (fbetr > flim)
        {
            ftr1=.01*ftr1;
            ftr2=.01*ftr2;
            ftr3=.01*ftr3;
        }
    }

    delta[3]=ftr1;
    delta[4]=ftr2;
    delta[5]=ftr3;

    //debug:
    if (print) std::cout <<  "translational forces: " << ftr1 <<"  "<< ftr2 <<"  " << ftr3 << std::endl;
    return ;
}



void AttractForceField::Rota(double phi,double ssi, double rot, Vdouble& delta,bool print)
{
    //delta array of doubles of dimension 6 ( 3 rotations, 3 translations)

    double  cs,cp,ss,sp,cscp,sscp,sssp,crot,srot,xar,yar,cssp,X,Y,Z ;
    double  pm[3][3];

// !c
// !c     calculates orientational force contributions
// !c     component 1: phi-angle
// !c     component 2: ssi-angle
// !c     component 3: rot-angle
// !c


    for (uint i=0; i<3;i++)
    {
        delta[i]=0.0;
        for (uint j=0;j<3;j++)
            pm[i][j]=0.0 ;
    }

    cs=cos(ssi);
    cp=cos(phi);
    ss=sin(ssi);
    sp=sin(phi);
    cscp=cs*cp;
    cssp=cs*sp;
    sscp=ss*cp;
    sssp=ss*sp;
    crot=cos(rot);
    srot=sin(rot);

    for (uint i=0; i<m_ligand.Size(); i++)
    {

        Coord3D coords = m_refligand.GetCoords(i);
        X = coords.x;
        Y = coords.y;
        Z = coords.z;

        xar=X*crot+Y*srot;
        yar=-X*srot+Y*crot;
        pm[0][0]=-xar*cssp-yar*cp-Z*sssp ;
        pm[1][0]=xar*cscp-yar*sp+Z*sscp ;
        pm[2][0]=0.0 ;

        pm[0][1]=-xar*sscp+Z*cscp ;
        pm[1][1]=-xar*sssp+Z*cssp ;
        pm[2][1]=-xar*cs-Z*ss ;

        pm[0][2]=yar*cscp+xar*sp ;
        pm[1][2]=yar*cssp-xar*cp ;
        pm[2][2]=-yar*ss ;

        for (uint j=0;j<3;j++)
        {
            delta[j] += pm[0][j] * m_ligforces[i].x ;
            delta[j] += pm[1][j] * m_ligforces[i].y ;
            delta[j] += pm[2][j] * m_ligforces[i].z ;
        }
    }

    if (print) std::cout << "Rotational forces: " << delta[0] << " " << delta[1] << " " << delta[2] << std::endl;

    return;
}



void AttractForceField::ResetForces()
{
    m_ligforces = std::vector<Coord3D> (m_ligforces.size());
}




void AttractForceField::ShowEnergyParams()
{
    for (unsigned int i=0; i < m_amp.size(); i++ )
        std::cout << "m_amp[" << i+1 << "]=" << m_amp[i]  << "   m_rad[" << i+1 << "]=" << m_rad[i] << std::endl;

}









// extern "C"
// {
//     extern
//         void nonbon8_ ( double* recCoords, int*r,  double* ligCoords, int* l, // coordinates for receptor and ligand and their sizes
//                         int* rectypes, int* ligtypes,  //  array for correspondance between atoms indexes and their Attract's "type"
//                         double* cha_r, double* cha_l,   // charges of atoms
//                         int* nonr, int* nonl, int* nonp,    //  arrays for pairlist and size of the pairlist
//                         double* ac, double* rc,                      //  precalculated two dimensional arrays. Beware of the column major issue
//                         double* LJ, double* coulomb                  //  return of the function
//                       );
//
//
// }



// double AttractForceField::fortranEnergy()
// {
//
//
// // linear array of receptor coordinates
//     std::vector <double> recCoords;
//
//
//     for (uint i=0; i <m_receptor.Size(); i++)
//     {
//         Coord3D co ( m_receptor.GetCoords(i) );
//         recCoords.push_back(co.x);
//         recCoords.push_back(co.y);
//         recCoords.push_back(co.z);
//
//     }
//
//
//
//     double LJ=0.0;
//     double coulomb=0.0;
//
//
//
// // linear array of ligand coordinates
//     std::vector <double> ligCoords;
//     for (uint i=0; i < m_ligand.Size(); i++)
//     {
//         Coord3D co ( m_ligand.GetCoords(i) );
//         ligCoords.push_back(co.x);
//         ligCoords.push_back(co.y);
//         ligCoords.push_back(co.z);
//     }
//
//     int r = m_receptor.Size();
//     std::cout << "m_receptor size: " << r << "\n" ;
//     int l = m_ligand.Size();
//
//
//     Vint ligplist; //ligand atoms in the pairlist
//     Vint recplist; //recetor atoms
//     int plistsize = plist.Size();
//
//     for (int i=0; i <  plistsize; i++)
//     {
//
//         ligplist.push_back( plist[i].atlig ) ;
//         recplist.push_back( plist[i].atrec ) ;
//     }
//
//
//
//
//     std::vector<int> rAtomCat;
//     std::vector<int> lAtomCat;
//
//     for (uint i=0; i<m_rAtomCat.size(); i++)
//         rAtomCat.push_back(m_rAtomCat[i]);
//
//
//
//     for (uint i=0; i<m_lAtomCat.size(); i++)
//         lAtomCat.push_back(m_lAtomCat[i]);
//
//     std::cout << "taille pairlist: " << plistsize << "\n";
//
//
// //     nonbon8_(  &recCoords[0],&r, &ligCoords[0], &l,  //
// //                &rAtomCat[0], &lAtomCat[0],     //
// //                &m_rAtomCharge[0], &m_lAtomCharge[0], //
// //                &recplist[0], &ligplist[0], &plistsize, //
// //                (double*) m_ac,(double*) m_rc, //
// //                &LJ, &coulomb) ;
//
//     return LJ+coulomb;
//
// }





////////////////////////////////////////////////////////////////
//     AttractForceField2 implementation
////////////////////////////////////////////////////////////////


static AttFF2_params* m_params = 0;

AttractForceField2::AttractForceField2(std::string filename, AttractRigidbody& rec, AttractRigidbody& lig, double cutoff)
        : m_ligand(lig),
        m_receptor(rec),
        m_pairlist(m_receptor,m_ligand,cutoff)

{

    m_ligSize = lig.Size();
    m_recSize = rec.Size();


    if (m_params==0)
    {
        m_params=new AttFF2_params();

//    return;

//         m_params->ipon = Array2D<int>(31,31);
//         m_params->ac = Array2D<double>(31,31);
//         m_params->rc = Array2D<double>(31,31);


        std::ifstream mbest (filename.c_str());
        //open(11,file=eingabe2) -> eingabe2: mbest1k.par
        if (!mbest)
        {
            //the file cannot be opened
            std::string msg = "Forcefield.cpp: Cannot Locate file  " + filename + "\n" ;
            std::cout << msg ;
            throw msg;
        }


        for (uint i = 0; i<31; i++)
            for (uint j = 0; j<31; j++)
            {
                mbest >> m_params->rbc[i][j] ;
                assert(i<=30);
                assert(j<=30);
            }

        for (uint i = 0; i<31; i++)
            for (uint j = 0; j<31; j++)
                mbest >> m_params->abc[i][j] ;

        for (uint i = 0; i<31; i++)
        {
            for (uint j = 0; j<31; j++)
            {
                mbest >> m_params->iflo[i][j] ;
                assert(m_params->iflo[i][j]==1 || m_params->iflo[i][j]==-1);
            }
        }



        for (uint jj=0; jj<31; jj++) //  jindex=0; jindex < lig.m_activeAtoms.size(); jindex++)
        {
//             uint j = lig.m_activeAtoms[jindex];
//             uint jj = lig.m_atomTypeNumber[j];
            for (uint ii=0; ii<31; ii++) //  uint iindex=0; iindex<rec.m_activeAtoms.size(); iindex++)
            {

//                 uint i = rec.m_activeAtoms[iindex];


//                 uint ii = rec.m_atomTypeNumber[i];
//                 assert(i<3000);
//                 assert(j<3000);
                double rbc2 = m_params->rbc[ii][jj]*m_params->rbc[ii][jj];
                double rbc6 = rbc2*rbc2*rbc2;
                double rbc8 = rbc6*rbc2;
                m_params->rc[ii][jj] = m_params->abc[ii][jj] * rbc8; //*pow(rbc[ii][jj],8); this optimization modifies the final result
                m_params->ac[ii][jj] = m_params->abc[ii][jj] * rbc6; //*pow(rbc[ii][jj],6); *but* the difference between the 2 c++ versions
                // is less than between C++(any version) and fortran
                // by the way pow() is very very slow. We should check why...
                assert(ii<31);
                assert(jj<31);
                m_params->ipon[ii][jj] = m_params->iflo[ii][jj] ;
                assert(m_params->ipon[ii][jj]==1 || m_params->ipon[ii][jj]==-1);

        double alen = m_params->ac[ii][jj];
        double rlen = m_params->rc[ii][jj];
        double alen4 = alen*alen*alen*alen;
        double rlen3 = rlen*rlen*rlen;
        m_params->emin[ii][jj] = -27.0*alen4/(256.0*rlen3);
        m_params->rmin2[ii][jj]= 4.0*rlen/(3.0*alen);

                
            }
        }
    }

    AttractRigidbody centeredlig(lig);
    Coord3D com = lig.FindCenter();
    m_ligcenter.push_back(com);
    centeredlig.CenterToOrigin();
    m_centeredligand.push_back(centeredlig);
    m_movedligand.push_back(centeredlig);


}


double AttractForceField2::Function(const Vdouble& stateVars )
{
    assert(stateVars.size()==6);
    assert(m_centeredligand.size() >=1);
    assert(m_movedligand.size() >=1);
    AttractEuler(m_centeredligand[0], m_movedligand[0], stateVars[0], stateVars[1], stateVars[2]);
    m_movedligand[0].Translate(m_ligcenter[0]);
    m_movedligand[0].Translate(Coord3D(stateVars[3],stateVars[4],stateVars[5]));
    return nonbon8(m_receptor, m_movedligand[0]);

}




/*! \brief returns the analytical derivatives of the forcefield 2
*
*   input:
*   Vdouble & stateVars: determines how the molecules are moved by the minimizer
*   (the minimizer only works on a linear Vdouble holding the free minimization variables)
*   output:
*   this function puts the derivative of the energy with respect to variable 1 to 6 (in case of 6 degrees of
*   freedom. 3 trans + 3 rotations) into the 'delta' Vdouble array
*/
void AttractForceField2::Derivatives(const Vdouble& stateVars, Vdouble& delta)
{
//delta[0] to delta[2] : rotations
//delta[3] to delta[5] : translation

    Trans(0, delta, 3, false);
    Rota(0, stateVars[0], stateVars[1], stateVars[2], delta, 0, false );

//print the delta vector:
// for (uint i=0; i<delta.size(); i++)
// {
// 	std::cout <<  delta[i] << "  " ;
// }
// std::cout << std::endl;

}


int getTotEner() {return ntotener;};


/*! \brief Non bonded energy
*
*   translated from fortran file nonbon8.f
*   TODO: add comments in the code, remove debug instructions
*/
double AttractForceField2::nonbon8(AttractRigidbody& rec, AttractRigidbody& lig, bool print)
{

    ntotener+=1; //TODO:remove this !

    double enon = 0.0;
    double epote = 0.0;

    //reset forces for ligand and receptor:
    lig.resetForces();
    rec.resetForces();


    for (uint ik=0; ik<m_pairlist.Size(); ik++ )
    {
        AtomPair atpair = m_pairlist[ik];

        std::cout.precision(20);

        uint i = atpair.atrec ;
        uint j = atpair.atlig ;
        uint ii=rec.m_atomTypeNumber[i];
        uint jj=lig.m_atomTypeNumber[j];
        assert(ii<=30);
        assert(jj<=30);
        double alen = m_params->ac[ii][jj];
        double rlen = m_params->rc[ii][jj];
        int ivor = m_params->ipon[ii][jj];
        assert(ivor==1 || ivor==-1);


        double charge= rec.m_charge[i]* lig.m_charge[j];  //charge product of the two atoms
        //std::cout << "charge: " << charge << std::endl;


        Coord3D dx = rec.GetCoords(i) - lig.GetCoords(j);
        double r2 = Norm2(dx);
        if (r2 < 0.001) r2=0.001 ;

        double rr2 = 1.0/r2;
        dx = rr2*dx ;

        if (charge != 0.0) {
            double et = charge*rr2;
            et*=(332.053986/15.0);  //constant felec/permi (could still be optimized!)

            epote += et ;
            Coord3D fdb =2.0*et*dx ;
            lig.m_forces[j] += fdb;
            rec.m_forces[i] -= fdb;

        }

        //switch between minimum or saddle point
        if (r2 < m_params->rmin2[ii][jj] ) {

            double rr23 = rr2*rr2*rr2 ;
            double rep = rlen*rr2 ;
            double vlj = (rep-alen)*rr23;
            enon=enon+vlj+(ivor-1)*m_params->emin[ii][jj] ;

            double fb=6.0*vlj+2.0*(rep*rr23);
            Coord3D fdb = fb*dx;
            lig.m_forces[j]+=fdb;
            rec.m_forces[i]-=fdb;
        }
        else {
            double rr23=rr2*rr2*rr2;
            double rep=rlen*rr2;
            double vlj=(rep-alen)*rr23 ;
            enon += ivor*vlj ;

            double fb=6.0*vlj+2.0*(rep*rr23);

            Coord3D fdb=ivor*fb*dx ;
            lig.m_forces[j] += fdb ;
            rec.m_forces[i] -= fdb ;
        }


    }

    if (print) std::cout << "vlj  coulomb: " << enon << "  " << epote << "\n";
    return enon+epote;
}



void AttractForceField2::Trans(uint molIndex, Vdouble & delta, uint shift,  bool print)
{
// molIndex is the index of the protein we want to extract the average
// translational forces


    AttractRigidbody const & rig(m_movedligand[molIndex]);
//   In this subroutine the translational force components are calculated
    double flim = 1.0e18;
    double ftr1, ftr2, ftr3, fbetr;

    ftr1=0.0;
    ftr2=0.0;
    ftr3=0.0;
    for (uint i=0;i<rig.Size(); i++)
    {
        ftr1=ftr1 + rig.m_forces[i].x;
        ftr2=ftr2 + rig.m_forces[i].y;
        ftr3=ftr3 + rig.m_forces[i].z;
    }

// force reduction, some times helps in case of very "bad" start structure
    for (uint i=0; i<3; i++)
    {
        fbetr=ftr1*ftr1 +ftr2*ftr2 +ftr3*ftr3;
        if (fbetr > flim)
        {
            ftr1=.01*ftr1;
            ftr2=.01*ftr2;
            ftr3=.01*ftr3;
        }
    }

    assert(shift+2 < delta.size());
    delta[0+shift]=ftr1;
    delta[1+shift]=ftr2;
    delta[2+shift]=ftr3;

    //debug:
    if (print) std::cout <<  "translational forces: " << ftr1 <<"  "<< ftr2 <<"  " << ftr3 << std::endl;
    return ;
}



void AttractForceField2::Rota(uint molIndex, double phi,double ssi, double rot, Vdouble & delta,uint shift, bool print)
{
// molIndex is the index of the protein we want to extract the average
// translational forces



    //delta array of doubles of dimension 6 ( 3 rotations, 3 translations)

    double  cs,cp,ss,sp,cscp,sscp,sssp,crot,srot,xar,yar,cssp,X,Y,Z ;
    double  pm[3][3];

// !c
// !c     calculates orientational force contributions
// !c     component 1: phi-angle
// !c     component 2: ssi-angle
// !c     component 3: rot-angle
// !c


    for (uint i=0; i<3;i++)
    {
        delta[i]=0.0;
        for (uint j=0;j<3;j++)
            pm[i][j]=0.0 ;
    }

    cs=cos(ssi);
    cp=cos(phi);
    ss=sin(ssi);
    sp=sin(phi);
    cscp=cs*cp;
    cssp=cs*sp;
    sscp=ss*cp;
    sssp=ss*sp;
    crot=cos(rot);
    srot=sin(rot);

    // for the x, y and z coordinates, we need
    // the coordinates of the centered, non-translated molecule

    AttractRigidbody * pLigCentered = & m_centeredligand[molIndex] ; // pointer to the centered ligand
    AttractRigidbody * pLigMoved  = & m_movedligand[molIndex] ; // pointer to the rotated/translated ligand (for forces)

    assert(shift+2 < delta.size());
    for (uint i=0; i< pLigCentered->m_activeAtoms.size(); i++)
    {
        uint atomIndex = pLigCentered->m_activeAtoms[i];

        Coord3D coords = pLigCentered->GetCoords(atomIndex);
        X = coords.x;
        Y = coords.y;
        Z = coords.z;

        xar=X*crot+Y*srot;
        yar=-X*srot+Y*crot;
        pm[0][0]=-xar*cssp-yar*cp-Z*sssp ;
        pm[1][0]=xar*cscp-yar*sp+Z*sscp ;
        pm[2][0]=0.0 ;

        pm[0][1]=-xar*sscp+Z*cscp ;
        pm[1][1]=-xar*sssp+Z*cssp ;
        pm[2][1]=-xar*cs-Z*ss ;

        pm[0][2]=yar*cscp+xar*sp ;
        pm[1][2]=yar*cssp-xar*cp ;
        pm[2][2]=-yar*ss ;

        for (uint j=0;j<3;j++)
        {
            delta[j+shift] += pm[0][j] * pLigMoved->m_forces[atomIndex].x ;
            delta[j+shift] += pm[1][j] * pLigMoved->m_forces[atomIndex].y ;
            delta[j+shift] += pm[2][j] * pLigMoved->m_forces[atomIndex].z ;
        }
    }

    if (print) std::cout << "Rotational forces: " << delta[0] << " " << delta[1] << " " << delta[2] << std::endl;

    return;
}








}//namespace PTools




