#include <cstring> // memcpy
#include <cmath> // sqrt
#include "DataSet_Vector.h"
#include "CpptrajStdio.h"

// CONSTRUCTOR
DataSet_Vector::DataSet_Vector() :
  DataSet(VECTOR, 8, 4),
  totalidx_(0),
  currentidx_(0),
  xyz_(0),
  isIred_(false),
  // Analysis_TimeCorr vars
  avgx_(0.0),
  avgy_(0.0),
  avgz_(0.0),
  rave_(0.0),
  r3iave_(0.0),
  r6iave_(0.0),
  R3i_(0)
{ }

// DESTRUCTOR
DataSet_Vector::~DataSet_Vector() {
  if (xyz_!=0) delete[] xyz_;
  if (R3i_!=0) delete[] R3i_;
}

// DataSet_Vector::IncreaseSize()
void DataSet_Vector::IncreaseSize() {
  int newsize = totalidx_ + 3000; // 500 frames * 6
  double* newxyz = new double[ newsize ];
  if (totalidx_ > 0) {
    memcpy(newxyz,        xyz_, totalidx_ * sizeof(double));
    memset(newxyz+totalidx_, 0,      3000 * sizeof(double));
    delete[] xyz_;
  }
  totalidx_ = newsize;
  xyz_ = newxyz;
}

// DataSet_Vector::Allocate()  
int DataSet_Vector::Allocate(int Nin) {
  if (xyz_!=0) delete[] xyz_;
  if (Nin < 1) {
    // # of frames is not known. Allocation will happen via IncreaseSize( )
    totalidx_ = 0;
    xyz_ = 0;
  } else {
    totalidx_ = 6 * Nin;
    xyz_ = new double[ totalidx_ ];
    memset( xyz_, 0, totalidx_ * sizeof(double) );
  }
  currentidx_ = 0;
    
  return 0;
}

// DataSet_Vector::WriteBuffer()
void DataSet_Vector::WriteBuffer(CpptrajFile &cbuffer, int frameIn) {
  int idx = frameIn * 6;
  if (idx < 0 || frameIn >= currentidx_) {
    mprinterr("Eerror: DataSet_Vector: Frame %i is out of range.\n",frameIn);
    return;
  }
  cbuffer.Printf(data_format_, xyz_[idx  ]);
  cbuffer.Printf(data_format_, xyz_[idx+1]);
  cbuffer.Printf(data_format_, xyz_[idx+2]);
  cbuffer.Printf(data_format_, xyz_[idx+3]);
  cbuffer.Printf(data_format_, xyz_[idx+4]);
  cbuffer.Printf(data_format_, xyz_[idx+5]);
  cbuffer.Printf(data_format_, xyz_[idx  ] + xyz_[idx+3]);
  cbuffer.Printf(data_format_, xyz_[idx+1] + xyz_[idx+4]);
  cbuffer.Printf(data_format_, xyz_[idx+2] + xyz_[idx+5]);
}

/** Calculates correlation functions using the "direct" approach
  * (s. Comp. Sim. of Liquids, p.185)
  * - the result is not yet normalized by (no_of_discrete_data - t)**-1 (!)
  */
void DataSet_Vector::corfdir(int ndata, double *data1, double *data2, int nsteps, double *dtmp)
{
  int i, j;
  int ind1, ind2;
  double dsum, dsumi;
  int ndata2 = ndata / 2; // TODO: Pass in

  if (data2 == NULL) {
    for(i = 0; i < ndata2; i++){
      dsum = 0.0;
      for(j = i; j < ndata2; j++){
        ind1 = 2 * j;
        ind2 = 2 * (j-i);
        dsum += data1[ind1] * data1[ind2] + data1[ind1+1] * data1[ind2+1];
      }
      if(i < nsteps){
        ind1 = 2 * i;
        dtmp[ind1  ] = dsum;
        dtmp[ind1+1] = 0.0;
      }
      else{
        break;
      }
    }
  } else {
    for(i = 0; i < ndata2; i++){
      dsum = 0.0;
      dsumi = 0.0;
      for(j = i; j < ndata2; j++){
        ind1 = 2 * j;
        ind2 = 2 * (j-i);
        dsum  += data2[ind1] * data1[ind2  ] + data2[ind1+1] * data1[ind2+1];
        dsumi += data2[ind1] * data1[ind2+1] - data2[ind1+1] * data1[ind2  ];
      }
      if(i < nsteps){
        ind1 = 2 * i;
        dtmp[ind1  ] = dsum;
        dtmp[ind1+1] = dsumi;
      }
      else{
        break;
      }
    }
  }

  for(i = 0; i < nsteps; i++){
    ind1 = 2 * i;
    data1[ind1  ] = dtmp[ind1  ];
    data1[ind1+1] = dtmp[ind1+1];
  }
}


// DataSet_Vector::sphericalHarmonics()
/** Calc spherical harmonics of order l=0,1,2
  * and -l<=m<=l with cartesian coordinates as input
  * (see e.g. Merzbacher, Quantum Mechanics, p. 186)
  * D[0] is the real part, D[1] is the imaginary part.
  */
void DataSet_Vector::sphericalHarmonics(int l, int m, const double* XYZ, double r, double D[2])
{
  const double SH00=0.28209479;
  const double SH10=0.48860251;
  const double SH11=0.34549415;
  const double SH20=0.31539157;
  const double SH21=0.77254840;
  const double SH22=0.38627420;

  double ri;
  double x = XYZ[0];
  double y = XYZ[1];
  double z = XYZ[2];
  //mprintf("CDBG: x=%.10lE y=%.10lE z=%.10lE\n",x,y,z);

  D[0] = 0.0;
  D[1] = 0.0;
  ri = 1.0 / r;

  if (l == 0 && m == 0) {
    D[0] = SH00;
  } else if (l == 1) {
    if (m == 0) {
      D[0] = SH10 * z * ri;
    } else {
      D[0] = -m * SH11 * x * ri;
      D[1] =     -SH11 * y * ri;
    }
  } else if(l == 2) {
    if (m == 0) {
      D[0] = SH20 * (2.0*z*z - x*x - y*y) * ri * ri;
    } else if (std::abs(m) == 1) {
      D[0] = -m * SH21 * x * z * ri * ri;
      D[1] =     -SH21 * y * z * ri * ri;
    } else {
      D[0] = SH22 * (x*x - y*y) * ri * ri;
      D[1] = m * SH22 * x * y * ri * ri;
    }
  }
  //mprintf("CDBG: dreal=%.10lE dimg=%.10lE\n",D[0],D[1]);
}

// DataSet_Vector::SphericalHarmonics()
// TODO: Change memory layout so order of all vecs follow each other,
//       i.e. order 2: m-2[0], m-2[1], ... m-2[N], m-1[0] ...
// TODO: Use STL vectors
double* DataSet_Vector::SphericalHarmonics(int order) {
  double Dcomplex[2];

  // Allocate space to hold complex numbers. Each frame has 2 doubles
  // (real + imaginary)  for each m = -olegendre -> +olegendre
  int n_of_vecs = Size();
  int p2blocksize = 2 * ((order * 2) + 1);
  int p2size = (p2blocksize * n_of_vecs);
  double* p2cftmp = new double[ p2size ];
  memset( p2cftmp, 0, p2size * sizeof(double) );
   // Loop over all vectors, convert to spherical harmonics
  double* VXYZ = xyz_;
  double* P2 = p2cftmp;
  for (int i = 0; i < n_of_vecs; ++i) {
    // Calc vector length
    double len = sqrt(VXYZ[0]*VXYZ[0] + VXYZ[1]*VXYZ[1] + VXYZ[2]*VXYZ[2]);
    // Loop over m = -olegendre, ..., +olegendre
    for (int midx = -order; midx <= order; ++midx) {
      sphericalHarmonics(order, midx, VXYZ, len, Dcomplex);
      //mprinterr("DBG: Vec %i sphereHarm[%u](m=%i) = %f + %fi\n",i,P2-p2cftmp,midx,Dcomplex[0],Dcomplex[1]);
      *(P2++) = Dcomplex[0];
      *(P2++) = Dcomplex[1];
    }
    VXYZ += 6;
  }
  return p2cftmp;
}

// DataSet_Vector::CalculateAverages()
void DataSet_Vector::CalculateAverages() {
  // If r3i is not NULL assume CalculateAverages was already called.
  if (R3i_ != 0) return;
  int n_of_vecs = Size();
  R3i_ = new double[ n_of_vecs ];
  avgx_ = 0.0;
  avgy_ = 0.0;
  avgz_ = 0.0;
  rave_ = 0.0;
  r3iave_ = 0.0;
  r6iave_ = 0.0;
  // Loop over all vectors
  double* VXYZ = xyz_;
  for (int i = 0; i < n_of_vecs; ++i) {
    // Calc vector length
    double len = sqrt(VXYZ[0]*VXYZ[0] + VXYZ[1]*VXYZ[1] + VXYZ[2]*VXYZ[2]);
    // Update avgcrd, rave, r3iave, r6iave
    avgx_ += VXYZ[0];
    avgy_ += VXYZ[1];
    avgz_ += VXYZ[2];
    rave_ += len;
    double r3i = 1.0 / (len*len*len);
    r3iave_ += r3i;
    r6iave_ += r3i*r3i;
    R3i_[i] = r3i;
    VXYZ += 6;
  }
}

// NOTE: Only used in 'analyze timecorr'
void DataSet_Vector::PrintAvgcrd(CpptrajFile& outfile) {
  double dnorm = 1.0 / (double)Size();
  outfile.Printf("%10.4f %10.4f %10.4f %10.4f\n",
          rave_ * dnorm,
          sqrt(avgx_*avgx_ + avgy_*avgy_ + avgz_*avgz_) * dnorm,
          r3iave_ * dnorm,
          r6iave_ * dnorm);
}
 
