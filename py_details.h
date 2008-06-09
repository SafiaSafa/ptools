#include "pairlist.h"
#include "ptools.h"
#include "coord3d.h"
#include "attractrigidbody.h"

#include <vector>

namespace pyplusplus{ namespace aliases{
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#ifdef AUTO_DIFF
typedef std::vector< surreal > Vsurreal;
#endif
typedef std::vector< double > Vdouble ;
typedef std::vector< uint > Vuint ;
typedef std::vector< int > Vint ;
typedef std::vector< PTools::Coord3D> VCoord3D;
typedef std::vector< PTools::AttractRigidbody > VAttractRigidbody;

//  typedef PTools::T_PairList<PTools::AttractRigidbody> Attract2PairList;

}
} //pyplusplus::aliases




//namespace  py_details{
//    inline void instantiate(){
//        using namespace PTools;
//        sizeof( PairList );
//        sizeof(Attract2PairList);
//    }
//}

