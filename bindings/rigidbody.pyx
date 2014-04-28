from cython.operator cimport dereference as deref
from libcpp.string cimport string


cdef extern from "<sstream>" namespace "std":
    cdef cppclass stringstream:
       stringstream()
       string str()
       void str(string &s)

       


cdef extern from "rigidbody.h" namespace "PTools":
    cdef cppclass CppRigidbody "PTools::Rigidbody":
        CppRigidbody(string) except+
        CppRigidbody()
        CppRigidbody(CppRigidbody&)
        unsigned int Size()
        CppCoord3D GetCoords(unsigned int)
        void unsafeGetCoords(unsigned int, CppCoord3D&)
        void SetCoords(unsigned int, CppCoord3D &)
        void ABrotate(CppCoord3D&, CppCoord3D&, double)
        void Translate(CppCoord3D&)
        CppCoord3D FindCenter()
        void syncCoords()
        void AttractEulerRotate(double, double, double)
        void ApplyMatrix(Array2D[double] &)
        CppAtom CopyAtom(unsigned int )
        void AddAtom(CppAtomproperty& , CppCoord3D )
        void AddAtom(CppAtom&)
        void SetAtom(unsigned int, CppAtom&)
        string PrintPDB()

        #returns radius of gyration
        double RadiusGyration()

        #returns the radius of a Rigidbody (max distance from center)
        double Radius()
        
        CppAtomproperty & GetAtomProperty(unsigned int)
        void SetAtomProperty(unsigned int, CppAtomproperty& )


        #AtomSelection:
        CppAtomSelection SelectAllAtoms()
        CppAtomSelection SelectAtomType(string)
        CppAtomSelection SelectResidType(string)
        CppAtomSelection SelectChainId(string)
        CppAtomSelection SelectResRange(int, int)
        CppAtomSelection CA()
        CppAtomSelection Backbone()
        

cdef extern from "pdbio.h" namespace "PTools":
    # in fact ReadPDB does not really take a stringstream as input argument, but it makes 
    # cython happy about typing and C++ is also happy due to inheritance
    cdef void ReadPDB(stringstream&, CppRigidbody&)



cdef class Rigidbody:
    cdef CppRigidbody* thisptr

    def __cinit__(self, filename=''):
        cdef CppRigidbody* oldrigidptr
        cdef Rigidbody  oldrigid
        cdef char* name
        cdef string *cppname
        cdef CppRigidbody* newrigid
        
        if isinstance(filename, str):
            if filename ==  '':
                self.thisptr = new CppRigidbody()
            else:
                # there is a filename, loading the pdb file
                #self.thisptr = _getRigidbody_from_py_name(filename)

                name = filename
                cppname = new string(name)
                newrigid = new CppRigidbody(deref(cppname))
                del cppname
                self.thisptr = newrigid




        elif isinstance(filename, Rigidbody):
            oldrigid = <Rigidbody> filename
            oldrigidptr = <CppRigidbody*> (oldrigid.thisptr)
            
            self.thisptr = new CppRigidbody(deref(oldrigidptr)  )
            if not self.thisptr:
                print "FATAL: this should never happen"

        elif hasattr(filename, "read"):
            #we consider filename as a file-like object
            #print "reading rigidbody from file-like"
            self.thisptr = new CppRigidbody()
            loadPDBfromPythonFileLike(filename, self.thisptr)
        else:
            raise RuntimeError("invalid argument in Rigidbody()")

   
    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr
            self.thisptr = <CppRigidbody*> 0
    def __len__(self):
        return self.thisptr.Size()


    def __str__(self):
        s = self.thisptr.PrintPDB()
        return s


    def getCoords(self, unsigned int i):
        cdef Coord3D c = Coord3D () 
        cdef CppCoord3D cpp = self.thisptr.GetCoords(i)
        c.x = cpp.x
        c.y = cpp.y
        c.z = cpp.z
        return c
        
    def unsafeGetCoords(self, unsigned int i, Coord3D co):
        self.thisptr.unsafeGetCoords(i, deref(co.thisptr))
    
    def setCoords(self, int i, Coord3D co):
        self.thisptr.SetCoords(i, deref(co.thisptr))    
        
    def Translate(self, Coord3D tr):
        self.thisptr.Translate(deref(tr.thisptr))
    def FindCenter(self):
        cdef Coord3D c = Coord3D()
        cdef CppCoord3D cpp = self.thisptr.FindCenter()
        c.x = cpp.x
        c.y = cpp.y
        c.z = cpp.z
        return c
        
    def ABrotate(self, Coord3D A, Coord3D B, double theta):
        self.thisptr.ABrotate(deref(A.thisptr),deref(B.thisptr),theta)
        return None
        
    def AttractEulerRotate(self, double phi, double ssi, double rot):
        self.thisptr.AttractEulerRotate(phi, ssi, rot)
    
    def syncCoords(self):
        self.thisptr.syncCoords()

    def ApplyMatrix(self, Matrix mat):
        self.thisptr.ApplyMatrix(deref(mat.thisptr))

    def CopyAtom(self, unsigned int atid):
        cdef CppAtom cpp_at = self.thisptr.CopyAtom(atid)
        
        cdef Atom at = Atom()
        cdef CppAtom* cpp_dest =<CppAtom*> at.thisptr
        cy_copy_atom(&cpp_at , cpp_dest )
        return at
        
        
    def AddAtom(self, Atom at):
        self.thisptr.AddAtom(deref(<CppAtom*>at.thisptr))
        

    def SetAtom(self, unsigned int position, Atom at):
        self.thisptr.SetAtom(position, deref(<CppAtom*>at.thisptr))
    
    
    def GetAtomProperty(self, unsigned int position):
        cdef CppAtomproperty cppatprop = self.thisptr.GetAtomProperty(position)
        cdef Atomproperty pyAtprop = Atomproperty()
        cdef CppAtomproperty* new_atomprop = new CppAtomproperty(cppatprop)
        del pyAtprop.thisptr
        pyAtprop.thisptr = new_atomprop
        
        return pyAtprop
        
    
    
    #    void SetAtomProperty(unsigned int, Atomproperty& )


    def Radius(self):
       return self.thisptr.Radius()

    def RadiusGyration(self):
       return self.thisptr.RadiusGyration()

    #AtomSelection:
    def SelectAllAtoms(self):
       ret = AtomSelection()
       del ret.thisptr
       cdef CppAtomSelection new_sel =  self.thisptr.SelectAllAtoms()
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret


    def SelectAtomType(self, bytes b):
       ret = AtomSelection()
       del ret.thisptr
       cdef char* c_typename = b
       cdef string* cpp_atomtype = new string(c_typename)
       cdef CppAtomSelection new_sel =  self.thisptr.SelectAtomType(deref(cpp_atomtype))
       del cpp_atomtype
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret
       


    def SelectResidType(self, bytes b):
       ret = AtomSelection()
       del ret.thisptr
       cdef char* c_typename = b
       cdef string* cpp_residtype = new string(c_typename)
       cdef CppAtomSelection new_sel =  self.thisptr.SelectResidType(deref(cpp_residtype))
       del cpp_residtype
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret


    def SelectChainId(self, bytes b):
       ret = AtomSelection()
       del ret.thisptr
       cdef char* c_typename = b
       cdef string* cpp_chainid = new string(c_typename)
       cdef CppAtomSelection new_sel =  self.thisptr.SelectChainId(deref(cpp_chainid))
       del cpp_chainid
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret


    def SelectResRange(self, int i, int j):
       ret = AtomSelection()
       del ret.thisptr
       cdef CppAtomSelection new_sel =  self.thisptr.SelectResRange(i,j)
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret


    def CA(self):
       ret = AtomSelection()
       del ret.thisptr
       cdef CppAtomSelection new_sel =  self.thisptr.CA()
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret


    def Backbone(self):
       ret = AtomSelection()
       del ret.thisptr
       cdef CppAtomSelection new_sel =  self.thisptr.Backbone()
       ret.thisptr  = new CppAtomSelection(new_sel)
       return ret

    

        
cdef CppRigidbody* _getRigidbody_from_py_name(pyname):
    cdef char* name = pyname
    cdef string *cppname = new string(name)
    cdef CppRigidbody *newrigid = new CppRigidbody(deref(cppname))
    del cppname
    return newrigid


cdef loadPDBfromPythonFileLike(file, CppRigidbody* rigid):
      cdef string cppstring
      lines = file.readlines()      

      for i in lines:
         cppstring += <char*> i
      
      cdef stringstream sstr
      sstr.str(cppstring)
      ReadPDB(sstr, deref(rigid))

      

cdef c_to_python_string():
    cdef char * test = "hello world"
    cdef bytes b = test
    return b
