#ifndef BASETYPES_H
#define BASETYPES_H


#include <vector>
#include <iostream>
#include <cassert>
#include <boost/shared_array.hpp>


//switch to choose between automatic differenciation 
//or normal double arithmetic




#include "derivify.h"  //must be included in any cases

#ifdef AUTO_DIFF
   typedef surreal dbl;
#else
   typedef double dbl;
#endif


typedef std::vector<int> Vint;
typedef std::vector<uint> Vuint;
typedef std::vector<dbl> Vdouble;

#ifndef NDEBUG
#define Debug(func) func
#else
#define Debug(func)
#endif




 template<size_t I,size_t N> struct metapow
{
  static inline dbl Pow( dbl x )
  {
    return x*metapow<I+1,N>::Pow(x);
  }
};

template<size_t N> struct metapow<N,N>
{
  static inline dbl Pow( dbl x ) { return 1; }
};

template<size_t N> dbl pow(dbl x)
{
  return metapow<0,N>::Pow(x);
}









//typedef dbl mytype;


/*! \brief 2-dimensional Object Oriented array
*
*  this class provides a 2-dimensional OO array. Memory is managed automatically.
*  The size of the array cannot be modified after creation.
*  It should be compatible with functions that needs raw pointers (with few modifications of the object).
*  WARNING: the assignemnt like 'Array2D array2=array1;' or 'Array2D array2(array1)' leads to two objects
*  sharing the same memory (this reduce the cost of copying and transmission by value,
*  like for Python objects). To make array2 independant of array1 (deep copy), use: array2.detach() after the
*  copy.
*  speed test are needed, but it should be reasonably fast for most daily usages
*  TODO: make speed tests ...
*/
template <class mytype>
class Array2D
{
public:
    Array2D(){};
    Array2D(int row, int columns)
            :m_rows(row),
            m_columns(columns)
    {
        m_size = row*columns;
        msa_data  = boost::shared_array<mytype>( new mytype[m_size]);
    }

    /// index operator (fortran-like syntax). a(0,2) gives raw 1, column 3.
    mytype& operator() (int r, int c)
    {
        assert(r<m_rows);
        assert(c<m_columns);
        return msa_data[r*m_columns+c];
    }

    const void * id() {
        return (void *) &msa_data[0];
    }
    void detach()
    {
        mytype * olddata = msa_data.get();
        mytype * newdata = new mytype[m_size];
        memcpy( newdata, olddata, m_size*sizeof(mytype) );
        msa_data=boost::shared_array<mytype>(newdata);
    }


private:
    boost::shared_array<mytype> msa_data;
    int m_rows;
    int m_columns;
    size_t m_size;


//friend void Array2D_deepCopy(const Array2D & src, Array2D & dest);

} ;




#endif




