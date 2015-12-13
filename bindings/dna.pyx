from cython.operator cimport dereference as deref

cdef extern from "DNA.h" namespace "PTools":
    cdef cppclass CppDNA "PTools::DNA":
        CppDNA()
        CppDNA(CppDNA&)
        CppDNA(string&, string&, CppMovement&)
        CppDNA(string&, string&)  #with default value for third parameter
        CppDNA SubDNA(int, int)
        CppBasePair operator[](int)
        unsigned int Size()

cdef class DNA:
    cdef CppDNA* thisptr

    def __cinit__(self, arg1=None, arg2=None, arg3=None):
        cdef Movement mov
        if arg1 is None:
            self.thisptr = new CppDNA()
        elif arg2 is not None:
            if arg3 is not None:
                mov = <Movement?> arg3
                self.thisptr = new CppDNA(<string>arg1,<string> arg2, deref(mov.thisptr))
            else:
                self.thisptr = new CppDNA(<string> arg1, <string> arg2)
        else: raise RuntimeError("invalid parameters during DNA creation")


    def __dealloc__(self):
        if self.thisptr:
            del self.thisptr
            self.thisptr = <CppDNA*> 0

    def SubDNA(self, int start, int end):
        ret = DNA()
        if ret.thisptr:
            del ret.thisptr
        cdef CppDNA cdna = self.thisptr.SubDNA(start, end)
        ret.thisptr = new CppDNA(cdna)
        return ret

    def __getitem__(self, unsigned int i):
        if i>=self.thisptr.Size():
            raise IndexError
        bp = BasePair()
        if bp.thisptr:
            del bp.thisptr

        bp.thisptr = new CppBasePair(deref(self.thisptr)[i])
        return bp
