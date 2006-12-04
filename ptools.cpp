
// Boost Includes ==============================================================
#include <boost/python.hpp>
#include <boost/cstdint.hpp>

// Includes ====================================================================
#include "atom.h"

// Using =======================================================================
using namespace boost::python;

// Module ======================================================================
BOOST_PYTHON_MODULE(ptools)
{
    class_< PTools::Atom >("Atom", init<  >())
        .def(init< const PTools::Atom& >())
        .def("GetType", &PTools::Atom::GetType)
        .def("SetType", &PTools::Atom::SetType)
    ;

}

