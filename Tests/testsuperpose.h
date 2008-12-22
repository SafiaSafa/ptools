#include <cxxtest/TestSuite.h>
#include <ctime>
#include <cstdlib>

#include <ptools.h>

using namespace PTools;
using namespace std;

double randfloat()
{
    return (double) rand() / (double) RAND_MAX ;
}

double rdrange(double a, double b)
{
    return randfloat()*(b-a)+a;
}

Coord3D rdCoord(double a, double b)
{
    double x = rdrange(a,b);
    double y = rdrange(a,b);
    double z = rdrange(a,b);
    return Coord3D(x,y,z);
}

class TestRot: public CxxTest::TestSuite
{


public:
    void testTransRot()
    {

        srand(time(NULL));

        for (int nrepet = 0; nrepet<50; nrepet++)
        {

            Rigidbody r1("1FIN_r.pdb");
            Rigidbody r2(r1);
            double x = (randfloat()-0.5)*50.0;
            double y = (randfloat()-0.5)*50.0;
            double z = (randfloat()-0.5)*50.0;

            r2.ABrotate(rdCoord(-20,20), rdCoord(-10,10), rdrange(-3.1415926,3.1415926) );

            r2.Translate(Coord3D(x,y,z));

            Superpose_t s = superpose(r1,r2);

            r2.ApplyMatrix(s.matrix);
            TS_ASSERT( Rmsd(r1,r2) < 1e-4 );


        }

    }
    void testVissage()
    {

        for(int i=0; i<1000; i++)
        {
        Rigidbody r1("1FIN_r.pdb");
        Rigidbody r2(r1);

        double x = (randfloat()-0.5)*50.0;
        double y = (randfloat()-0.5)*50.0;
        double z = (randfloat()-0.5)*50.0;

        r2.ABrotate(rdCoord(-20,20), rdCoord(-10,10), rdrange(-3.1415926,3.1415926) );

        r2.Translate(Coord3D(x,y,z));

        Superpose_t  s = superpose(r1,r2);

        Screw v =  MatTrans2screw(s.matrix);
//        v.Print();

        Rigidbody r3;
        r3.ABrotate(v.point, v.point+v.unitVector, v.angle);
        r3.Translate(v.normtranslation*v.unitVector);


//        r3.GetMatrix().Print();

//        s.matrix.Print();

        if(!s.matrix.almostEqual(r3.GetMatrix(),1e-2))
           {
               cout << "(((((((((((((((((\n";
               s.matrix.Print();
               cout << "\n";
               r3.GetMatrix().Print();
               cout << v.angle << "\n";

           }


         TS_ASSERT(s.matrix.almostEqual(r3.GetMatrix(),1e-2));


        }

    }












};