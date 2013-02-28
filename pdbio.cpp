#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <istream>

#include "atom.h"
#include "stdio.h"
#include "pdbio.h"

using namespace std;

namespace PTools{


std::string strip( std::string const& str, char const* sepSet ) {
    std::string::size_type const first = str.find_first_not_of(sepSet);
    return ( first==std::string::npos )
           ? std::string()
           : str.substr(first, str.find_last_not_of(sepSet)-first+1);
}


bool isAtom(const std::string & line ) {
    //cout << ligne.size()<< endl ;
    if (line.size()<10 ) return false;

    if (line.substr(0,6)==(std::string) "ATOM  " ) return true ;
    return false;
}


bool isHeteroAtom(const std::string& line) {
    if (line.size()<10) return false;

    if (line.substr(0,6) == "HETATM") return true;
    return false;
}




std::string readatomtype(const std::string &ligne) {
    std::string type="" ;
    int i=12 ;
    while (ligne[i]==' ')
    {
        i++;
        if (i>15) return type ;
    }

    int j=i ;
    while (ligne[j]!=' ') j++ ;

    type=ligne.substr(i,j-i) ;
    std::transform(type.begin(),type.end(),
                   type.begin(), //destination
                   (int(*)(int)) toupper //to upper: convert to upper case
                  );

    //cout << type <<"a" ;



    return type;
}


std::string readresidtype(const std::string &ligne) {
    std::string type="" ;
    int i=17 ;
    while (ligne[i]==' ')
    {
        i++;
        if (i>19) return type ;
    }

    int j=i ;
    while (ligne[j]!=' ') j++ ;

    type=ligne.substr(i,j-i) ;
    std::transform(type.begin(),type.end(),
                   type.begin(), //destination
                   (int(*)(int)) toupper //to upper: convert to upper case
                  );

    //cout << type <<"a" ;



    return type;
}


void ReadPDB(istream& file, Rigidbody& protein) {

    std::string line ;
    int line_count = 0 , atom_count=0;

    while ( std::getline(file, line))
    {
        line_count++ ;
        if (isAtom(line))
        {

            atom_count++ ;

            std::string sx,sy,sz;

            sx=line.substr(30,8);
            sy=line.substr(38,8);
            sz=line.substr(46,8);

            Coord3D pos;
            pos.x=atof(sx.c_str());
            pos.y=atof(sy.c_str());
            pos.z=atof(sz.c_str());

            Atomproperty a;
            a.atomType = readatomtype(line);
            a.residType = readresidtype(line);
	    a.altLoc = line[16];
            char chainID = line[21];
//             if (chainID == ' ') chainID = '';
            a.chainId = chainID;
            a.residId = atoi(line.substr(22,4).c_str());
            a.atomId = atoi(line.substr(6,5).c_str());
            std::string extra = line.substr(54); //extracts everything after the position 54 to the end of line
            a.extra = extra ;

            protein.AddAtom(a,pos);

        }
    }
}



void ReadPDB(const std::string name,Rigidbody& protein ) {
    std::string nomfich=name ;
	// pointer toward the filename given in the constructor argument
    ifstream file(nomfich.c_str()); 
    if (!file)
    {
        std::ostringstream oss;
        throw std::invalid_argument("##### ReadPDB:Could not open file \"" + nomfich + "\" #####") ;
    }

    ReadPDB(file, protein);
    file.close();
    return;

}

void WritePDB(const Rigidbody& rigid, std::string filename)
{

    FILE* file= fopen(filename.c_str(),"w") ;

    for (uint i=0; i<rigid.Size();i++)
    {

//         const char * chainID="A" ;

        Atom at = rigid.CopyAtom(i);
        const char* atomname=at.atomType.c_str();
        const char* residName=at.residType.c_str();
        int residnumber = at.residId;
//         chainID = at.chainId;

        int atomnumber = at.atomId;

        Coord3D coord = at.coords;
        dbl x = coord.x;
        dbl y = coord.y;
        dbl z = coord.z ;



        fprintf(file,"ATOM  %5d %-4s %3s %c%4d    %8.3f%8.3f%8.3f%s",
		 atomnumber,atomname,residName,
		 at.chainId ,residnumber,real(x),
		 real(y),real(z),at.extra.c_str());    
        fprintf(file,"\n");
    }

    fclose(file);
}

} //namespace PTools



