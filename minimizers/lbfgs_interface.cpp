#include "lbfgs_interface.h"

#include <string.h>
#include <iostream>
#include <math.h>
#include <float.h>

//#include "../complexify.h"

namespace PTools{

inline void assign(char* dest, char* src)
{
    memcpy(dest,src,strlen(src));

}






Lbfgs::Lbfgs( ForceField& toMinim)
        :objToMinimize(toMinim)
{
    //let the object do some initialization before beginning a new minimization
    //(for example, create new pairlists...)
    m_opt = NULL;
    objToMinimize.initMinimization();
}


Lbfgs::~Lbfgs()
{
    if (m_opt) lbfgsb_destroy(m_opt);
}


// vector<double> to vector<double> converter. used for genericity. should not impact performances too much.
inline void tocplx(const std::vector<double> & vdblin, std::vector<double> & vdblout ) {
    vdblout=vdblin;
}



#ifdef AUTO_DIFF
//convert from vector<cplx> to vector<double>
inline void tocplx(const std::vector<double> & vdblin, std::vector<surreal> & vcplx)
{
    vcplx = std::vector<surreal>();
    for (uint i=0; i<vdblin.size(); i++)
    {
        vcplx.push_back(vdblin[i]);
    }
}

#endif

inline std::vector<double> todouble(std::vector<double> & vdbl) {
    return vdbl;
}

#ifdef AUTO_DIFF
inline std::vector<double> todouble(std::vector<surreal> & vcplx)
{
    std::vector<double> vdbl;
    for (uint i=0; i<vcplx.size(); i++)
    {
        vdbl.push_back(real(vcplx[i]));
    }
    return vdbl;
};

#endif

void Lbfgs::minimize(int maxiter,std::vector<dbl>& xuser )
{

    uint n = objToMinimize.ProblemSize();

    std::cout  << "number of free variables for the minimizer: " << n << std::endl;


    std::vector<double> l(n);
    std::vector<double> u(n);
    Vint nbd(n);

    if (xuser.size() != n)
    {
        throw std::range_error("the vector you gave has the wrong size");
    }

    x = todouble(xuser);
    g.resize(n);

    for (uint i=0;i<n; i++)
    {
        l[i]=0;
        u[i]=0;
        nbd[i]=0;
        g[i] = 0.0;
    }  //unconstrained problem



    int rc;

    int m = 5;

    m_opt = lbfgsb_create(n, m, &l[0], &u[0], &nbd[0]);
    assert(m_opt);


    m_opt->iprint=-1;

    double f = DBL_MAX;

    m_opt->max_iter = maxiter;

    int last_iter = 0;

    /*    opt->iprint = 0;*/
    while (1) {
        rc = lbfgsb_run(m_opt, &x[0], &f, &g[0]);
        if (rc == 0) {
            break;
        } else if (rc < 0) {
            std::cout << "lbfgsb stop with an error ";
            break;
        } else if (rc == 1) {



#ifdef AUTO_DIFF
//check analytical derivatives with surreal:
            {
                std::vector<dbl> vdblx;
                tocplx(x,vdblx);
                std::vector<dbl> vdblg;
                tocplx(g,vdblg);

                f = objToMinimize.Function(vdblx);
                objToMinimize.Derivatives(vdblx,vdblg);

                g=todouble(vdblg);


            std::vector<double> autog;
            for (uint i=0; i<x.size(); i++)
            {
                std::vector<surreal> svdblx = vdblx ;
                svdblx[i]+=surreal(0,1);
                autog.push_back( imag(objToMinimize.Function(svdblx)) );
            }

            std::cout << "analytical derivatives:\n";
            for (uint i=0; i<x.size(); i++)
            {
                std::cout << g[i] <<  "  " ;
            }

            std::cout <<"auto derivatives: \n";
            for (uint i=0; i<x.size(); i++)
            {
                std::cout << autog[i] <<  "  " ;
            }


            std::cout << std::endl;
}
//end check derivatives
#endif




            if (last_iter < m_opt->niter)
            {
                //this is a new iteration
                last_iter = m_opt->niter;
                //saves the minimizer variables for each iteration (can be useful for generating animations)
                m_vars_over_time.push_back(x);

            }


            std::vector<dbl> vdblx;
            tocplx(x,vdblx);
            std::vector<dbl> vdblg;
            tocplx(g,vdblg);

            f = objToMinimize.Function(vdblx);
            objToMinimize.Derivatives(vdblx,vdblg);

            g=todouble(vdblg);

//                 std::cout << "analytical derivatives: \n";
//                 for(uint i=0; i<g.size(); i++)
//                 {
//                     std::cout << "deriv[" << i << "]: " << g[i] << std::endl;
//                 }
//                 objToMinimize.NumDerivatives(x,g,true);


        } else {
            assert(!"can not reach here");
        }
    }


    m_opt->task[59]='\0'; //add a null terminating character to task[]


    std::cout << m_opt->task  << " |  " << m_opt->niter << " iterations\n";

    tocplx(x,xuser);

}




std::vector<double>Lbfgs::GetMinimizedVarsAtIter(uint iter)
{
    if (iter>=m_vars_over_time.size())
    {
        std::string msg = "";
        msg+= iter;
        msg += " is out of range (max: ";
        msg += m_vars_over_time.size()-1 ;
        msg += " )\n";
        throw std::out_of_range(msg);
    }
    return m_vars_over_time[iter];
}






} //namespace lbfgs






