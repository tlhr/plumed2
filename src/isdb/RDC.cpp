/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2014-2016 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#include "colvar/Colvar.h"
#include "colvar/ActionRegister.h"
#include "core/PlumedMain.h"
#include "tools/OpenMP.h"

#ifdef __PLUMED_HAS_GSL
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#endif

using namespace std;

namespace PLMD {
namespace isdb {

//+PLUMEDOC COLVAR RDC
/*
Calculates the (Residual) Dipolar Coupling between two atoms.

The RDC between two atomic nuclei depends on the \f$\theta\f$ angle between
the inter-nuclear vector and the external magnetic field. In isotropic media RDCs average to zero because of the rotational
averaging, but when the rotational symmetry is broken, either through the introduction of an alignment medium or for molecules
with highly anisotropic paramagnetic susceptibility, RDCs become measurable.

\f[
D=D_{max}0.5(3\cos^2(\theta)-1)
\f]

where

\f[
D_{max}=-\mu_0\gamma_1\gamma_2h/(8\pi^3r^3)
\f]

that is the maximal value of the dipolar coupling for the two nuclear spins with gyromagnetic ratio \f$\gamma\f$.
\f$\mu\f$ is the magnetic constant and h is the Planck constant.

Common Gyromagnetic Ratios (C.G.S)
- H(1) 26.7513
- C(13) 6.7261
- N(15) -2.7116
- NH -72.5388
- CH 179.9319
- CN -18.2385
- CC 45.2404

This collective variable calculates the Residual Dipolar Coupling for a set of couple of atoms using the above definition.
From the calculated RDCs and a set of experimental values it calculates either their correlation or the squared quality factor

\f[
Q^2=\frac{\sum_i(D_i-D^{exp}_i)^2}{\sum_i(D^{exp}_i)^2}
\f]

RDCs report only on the fraction of molecules that is aligned, this means that comparing the RDCs from a single structure in
a MD simulation to the experimental values is not particularly meaningfull, from this point of view it is better to compare
their correlation. The fraction of aligned molecules can be obtained by maximising the correlation between the calculated and
the experimental RDCs. This fraction can be used as a scaling factor in the calculation of the RDCs in order to compare their
values. The averaging of the RDCs calculated with the above definition from a standard MD should result to 0 because of
the rotational diffusion, but this variable can be used to break the rotational symmetry.

RDCs can also be calculated using a Single Value Decomposition approach, in this case the code rely on the
a set of function from the GNU Scientific Library (GSL). (With SVD forces are not currently implemented).

Metainference simulations can be performed with this CV and \ref METAINFERENCE.

Additional material and examples can be also found in the tutorial \ref belfast-9

\par Examples
In the following example five N-H RDCs are defined and their correlation with
respect to a set of experimental data is calculated and restrained. In addition,
and only for analysis purposes, the same RDCs are calculated using a Single Value
Decomposition algorithm.

\plumedfile
RDC ...
GYROM=-72.5388
SCALE=1.0
ATOMS1=20,21
ATOMS2=37,38
ATOMS3=56,57
ATOMS4=76,77
ATOMS5=92,93
LABEL=nh
... RDC

STATS ARG=nh.* PARAMETERS=8.17,-8.271,-10.489,-9.871,-9.152

rdce: RESTRAINT ARG=nh.corr KAPPA=0. SLOPE=-25000.0 AT=1.

RDC ...
GYROM=-72.5388
SCALE=1.0
SVD
ATOMS1=20,21 COUPLING1=8.17
ATOMS2=37,38 COUPLING2=-8.271
ATOMS3=56,57 COUPLING3=-10.489
ATOMS4=76,77 COUPLING4=-9.871
ATOMS5=92,93 COUPLING5=-9.152
LABEL=svd
... RDC

PRINT ARG=nh.corr,rdce.bias FILE=colvar
PRINT ARG=svd.* FILE=svd
\endplumedfile
(See also \ref PRINT, \ref RESTRAINT)

*/
//+ENDPLUMEDOC

//+PLUMEDOC COLVAR PCS
/*
Calculates the Pseudocontact shift of a nucleus determined by a metal ion.

The PCS of an atomic nucleus depends on the \f$\theta\f$ angle between the vector from the spin-label to the nucleus
 and the external magnetic field and the module of the vector itself.

\f[
D=D_{max}0.5(3\cos^2(\theta)-1)
\f]

where

\f[
D_{max}=-\mu_0\gamma_1\gamma_2h/(8\pi^3r^3)
\f]

that is the maximal value of the dipolar coupling for the two nuclear spins with gyromagnetic ratio \f$\gamma\f$.
\f$\mu\f$ is the magnetic constant and h is the Planck constant.

PCSs can also be calculated using a Single Value Decomposition approach, in this case the code rely on the
a set of function from the GNU Scientific Library (GSL). (With SVD forces are not currently implemented).

Metainference simulations can be performed with this CV and \ref METAINFERENCE.

Additional material and examples can be also found in the tutorial \ref belfast-9

\par Examples
In the following example five N-H RDCs are defined and their correlation with
respect to a set of experimental data is calculated and restrained. In addition,
and only for analysis purposes, the same RDCs are calculated using a Single Value
Decomposition algorithm.

\plumedfile
RDC ...
GYROM=-72.5388
SCALE=1.0
ATOMS1=20,21
ATOMS2=37,38
ATOMS3=56,57
ATOMS4=76,77
ATOMS5=92,93
LABEL=nh
... RDC

STATS ARG=nh.* PARAMETERS=8.17,-8.271,-10.489,-9.871,-9.152

rdce: RESTRAINT ARG=nh.corr KAPPA=0. SLOPE=-25000.0 AT=1.

RDC ...
GYROM=-72.5388
SCALE=1.0
SVD
ATOMS1=20,21 COUPLING1=8.17
ATOMS2=37,38 COUPLING2=-8.271
ATOMS3=56,57 COUPLING3=-10.489
ATOMS4=76,77 COUPLING4=-9.871
ATOMS5=92,93 COUPLING5=-9.152
LABEL=svd
... RDC

PRINT ARG=nh.corr,rdce.bias FILE=colvar
PRINT ARG=svd.* FILE=svd
\endplumedfile

*/
//+ENDPLUMEDOC

class RDC : public Colvar {
private:
  double         Const;
  double         mu_s;
  double         scale;
  vector<double> coupl;
  bool           svd;
  bool           pbc;
public:
  explicit RDC(const ActionOptions&);
  static void registerKeywords( Keywords& keys );
  virtual void calculate();
};

PLUMED_REGISTER_ACTION(RDC,"RDC")
PLUMED_REGISTER_ACTION(RDC,"PCS")

void RDC::registerKeywords( Keywords& keys ) {
  Colvar::registerKeywords( keys );
  componentsAreNotOptional(keys);
  useCustomisableComponents(keys);
  keys.add("numbered","ATOMS","the couple of atoms involved in each of the bonds for which you wish to calculate the RDC. "
           "Keywords like ATOMS1, ATOMS2, ATOMS3,... should be listed and one dipolar coupling will be "
           "calculated for each ATOMS keyword you specify.");
  keys.reset_style("ATOMS","atoms");
  keys.add("compulsory","GYROM","1.","Add the product of the gyromagnetic constants for the bond. ");
  keys.add("compulsory","SCALE","1.","Add the scaling factor to take into account concentration and other effects. ");
  keys.addFlag("SVD",false,"Set to TRUE if you want to backcalculate using Single Value Decomposition (need GSL at compilation time).");
  keys.addFlag("ADDCOUPLINGS",false,"Set to TRUE if you want to have fixed components with the experimetnal values.");
  keys.add("numbered","COUPLING","Add an experimental value for each coupling (needed by SVD and usefull for \ref STATS).");
  keys.addOutputComponent("rdc","default","the calculated # RDC");
  keys.addOutputComponent("exp","SVD/ADDCOUPLINGS","the experimental # RDC");
}

RDC::RDC(const ActionOptions&ao):
  PLUMED_COLVAR_INIT(ao),
  Const(1.),
  mu_s(1.),
  scale(1.),
  pbc(true)
{
  bool nopbc=!pbc;
  parseFlag("NOPBC",nopbc);
  pbc=!nopbc;

  const double RDCConst = 0.3356806;
  const double PCSConst = 1.0;
  bool rdc = true;

  if( getName().find("RDC")!=std::string::npos) { Const *= RDCConst; rdc=true;}
  else if( getName().find("PCS")!=std::string::npos) { Const *= PCSConst; rdc=false;}

  // Read in the atoms
  vector<AtomNumber> t, atoms;
  for(int i=1;; ++i ) {
    parseAtomList("ATOMS", i, t );
    if( t.empty() ) break;
    if( t.size()!=2 ) {
      std::string ss; Tools::convert(i,ss);
      error("ATOMS" + ss + " keyword has the wrong number of atoms");
    }
    atoms.push_back(t[0]);
    atoms.push_back(t[1]);
    t.resize(0);
  }

  const unsigned ndata = atoms.size()/2;

  // Read in GYROMAGNETIC constants
  parse("GYROM", mu_s);
  if(mu_s==0.) error("GYROM cannot be 0");

  // Read in SCALING factors
  parse("SCALE", scale);
  if(scale==0.) error("SCALE cannot be 0");

  svd=false;
  parseFlag("SVD",svd);
#ifndef __PLUMED_HAS_GSL
  if(svd) error("You CANNOT use SVD without GSL. Recompile PLUMED with GSL!\n");
#endif

  bool addcoupling=false;
  parseFlag("ADDCOUPLINGS",addcoupling);

  if(svd||addcoupling) {
    coupl.resize( ndata );
    unsigned ntarget=0;
    for(unsigned i=0; i<ndata; ++i) {
      if( !parseNumbered( "COUPLING", i+1, coupl[i] ) ) break;
      ntarget++;
    }
    if( ntarget!=ndata ) error("found wrong number of COUPLING values");
  }

  // Ouput details of all contacts
  log.printf("  Gyromagnetic moment is %f. Scaling factor is %f.",mu_s,scale);
  for(unsigned i=0; i<ndata; ++i) {
    log.printf("  The %uth Bond Dipolar Coupling is calculated from atoms : %d %d.", i+1, atoms[2*i].serial(), atoms[2*i+1].serial());
    if(svd||addcoupling) log.printf(" Experimental coupling is %f.", coupl[i]);
    log.printf("\n");
  }

  log<<"  Bibliography "
     <<plumed.cite("Camilloni C, Vendruscolo M, J. Phys. Chem. B 119, 653 (2015)")
     <<plumed.cite("Camilloni C, Vendruscolo M, Biochemistry 54, 7470 (2015)") <<"\n";

  checkRead();

  for(unsigned i=0; i<ndata; i++) {
    std::string num; Tools::convert(i,num);
    if(!svd) {
      addComponentWithDerivatives("rdc_"+num);
      componentIsNotPeriodic("rdc_"+num);
    } else {
      addComponent("rdc_"+num);
      componentIsNotPeriodic("rdc_"+num);
    }
  }

  if(svd||addcoupling) {
    for(unsigned i=0; i<ndata; i++) {
      std::string num; Tools::convert(i,num);
      addComponent("exp_"+num);
      componentIsNotPeriodic("exp_"+num);
      Value* comp=getPntrToComponent("exp_"+num); comp->set(coupl[i]);
    }
  }

  if(svd) {
    addComponent("Sxx"); componentIsNotPeriodic("Sxx");
    addComponent("Syy"); componentIsNotPeriodic("Syy");
    addComponent("Szz"); componentIsNotPeriodic("Szz");
    addComponent("Sxy"); componentIsNotPeriodic("Sxy");
    addComponent("Sxz"); componentIsNotPeriodic("Sxz");
    addComponent("Syz"); componentIsNotPeriodic("Syz");
  }

  requestAtoms(atoms);
}

void RDC::calculate()
{
  if(!svd) {
    const double max  = -Const*scale*mu_s;
    const unsigned N=getNumberOfAtoms();
    /* RDC Calculations and forces */
    const double omp_dummy = 0.0;
    const unsigned nt = OpenMP::getGoodNumThreads(&omp_dummy, N / 2);
    #pragma omp parallel num_threads(nt)
    {
      #pragma omp for
      for(unsigned r=0; r<N; r+=2)
      {
        const unsigned index=r/2;
        Vector  distance;
        if(pbc) distance   = pbcDistance(getPosition(r),getPosition(r+1));
        else    distance   = delta(getPosition(r),getPosition(r+1));
        const double d2    = distance.modulo2();
        const double ind   = 1./sqrt(d2);
        const double ind2  = 1./d2;
        const double ind3  = ind2*ind;
        const double x2    = distance[0]*distance[0]*ind2;
        const double y2    = distance[1]*distance[1]*ind2;
        const double z2    = distance[2]*distance[2]*ind2;
        const double dmax  = ind3*max;
        const double ddmax = dmax*ind2;

        const double rdc   = 0.5*dmax*(3.*z2-1.);
        const double prod_xy = (x2+y2-4.*z2);
        const double prod_z =  (3.*x2 + 3.*y2 - 2.*z2);

        Vector dRDC = -1.5*ddmax*distance;
        dRDC[0] *= prod_xy;
        dRDC[1] *= prod_xy;
        dRDC[2] *= prod_z;

        Value* val=getPntrToComponent(index);
        val->set(rdc);
        setBoxDerivatives(val, Tensor(distance,dRDC));
        setAtomsDerivatives(val, r,  dRDC);
        setAtomsDerivatives(val, r+1, -dRDC);
      }
    }
  } else {

#ifdef __PLUMED_HAS_GSL
    gsl_vector *rdc_vec, *S, *Stmp, *work, *bc;
    gsl_matrix *coef_mat, *A, *V;
    rdc_vec = gsl_vector_alloc(coupl.size());
    bc = gsl_vector_alloc(coupl.size());
    Stmp = gsl_vector_alloc(5);
    S = gsl_vector_alloc(5);
    work = gsl_vector_alloc(5);
    coef_mat = gsl_matrix_alloc(coupl.size(),5);
    A = gsl_matrix_alloc(coupl.size(),5);
    V = gsl_matrix_alloc(5,5);
    gsl_matrix_set_zero(coef_mat);
    gsl_vector_set_zero(bc);
    unsigned index=0;
    vector<double> dmax(coupl.size());
    for(unsigned r=0; r<getNumberOfAtoms(); r+=2) {
      Vector  distance;
      if(pbc) distance = pbcDistance(getPosition(r),getPosition(r+1));
      else    distance = delta(getPosition(r),getPosition(r+1));
      double d    = distance.modulo();
      double d2   = d*d;
      double d3   = d2*d;
      double id3  = 1./d3;
      double max  = -Const*mu_s*scale;
      dmax[index] = id3*max;
      double mu_x = distance[0]/d;
      double mu_y = distance[1]/d;
      double mu_z = distance[2]/d;
      gsl_vector_set(rdc_vec,index,coupl[index]/dmax[index]);
      gsl_matrix_set(coef_mat,index,0,gsl_matrix_get(coef_mat,index,0)+(mu_x*mu_x-mu_z*mu_z));
      gsl_matrix_set(coef_mat,index,1,gsl_matrix_get(coef_mat,index,1)+(mu_y*mu_y-mu_z*mu_z));
      gsl_matrix_set(coef_mat,index,2,gsl_matrix_get(coef_mat,index,2)+(2.0*mu_x*mu_y));
      gsl_matrix_set(coef_mat,index,3,gsl_matrix_get(coef_mat,index,3)+(2.0*mu_x*mu_z));
      gsl_matrix_set(coef_mat,index,4,gsl_matrix_get(coef_mat,index,4)+(2.0*mu_y*mu_z));
      index++;
    }
    gsl_matrix_memcpy(A,coef_mat);
    gsl_linalg_SV_decomp(A, V, Stmp, work);
    gsl_linalg_SV_solve(A, V, Stmp, rdc_vec, S);
    /* tensor */
    Value* tensor;
    tensor=getPntrToComponent("Sxx");
    double Sxx = gsl_vector_get(S,0);
    tensor->set(Sxx);
    tensor=getPntrToComponent("Syy");
    double Syy = gsl_vector_get(S,1);
    tensor->set(Syy);
    tensor=getPntrToComponent("Szz");
    double Szz = -Sxx-Syy;
    tensor->set(Szz);
    tensor=getPntrToComponent("Sxy");
    double Sxy = gsl_vector_get(S,2);
    tensor->set(Sxy);
    tensor=getPntrToComponent("Sxz");
    double Sxz = gsl_vector_get(S,3);
    tensor->set(Sxz);
    tensor=getPntrToComponent("Syz");
    double Syz = gsl_vector_get(S,4);
    tensor->set(Syz);

    gsl_blas_dgemv(CblasNoTrans, 1.0, coef_mat, S, 0., bc);
    for(index=0; index<coupl.size(); index++) {
      double rdc = gsl_vector_get(bc,index)*dmax[index];
      Value* val=getPntrToComponent(index);
      val->set(rdc);
    }
    gsl_matrix_free(coef_mat);
    gsl_matrix_free(A);
    gsl_vector_free(rdc_vec);
    gsl_vector_free(bc);
    gsl_vector_free(Stmp);
    gsl_vector_free(S);
    gsl_vector_free(work);
#endif

  }

}

}
}