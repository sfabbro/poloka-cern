#include <iostream>
#include <matvect.h>

using namespace std;

Mat::Mat(const unsigned int NX, const unsigned int NY) { 
 data=NULL;
 nx=ny=0;
 if(NX<=0 || NY<=0) {
   cout << "Mat::Mat ERROR NX,NY =" << NX << "," << NY <<  endl;
 }else{
   allocate(NX,NY);
 }
}

Mat::Mat(const Mat& other) {
  data=NULL;
  nx=ny=0;
  allocate(other.SizeX(),other.SizeY());
  memcpy(data,other.Data(),sizeof(double)*nx*ny); 
}

void Mat::allocate(const unsigned int NX, const unsigned int NY) {
  if(NX<=0 || NY<=0) {
    cout << "Mat::allocate ERROR NX,NY =" << NX << "," << NY <<  endl;
  }
  if(nx!=NX || ny!=NY) {
    nx=NX; 
    ny=NY; 
    if (data) 
      delete [] data;
    data = new double[nx*ny];
  } 
  Zero();
}

double Mat::operator () (const unsigned int i, const unsigned int j) const {
#ifdef MATVECT_CHECK_BOUNDS
  if (i>=nx || j>=ny || i<0 || j<0) { 
    cout << "Mat::operator () overflow i,j nx,ny " 
	 << i << "," << j << " "
	 << nx << "," << ny << " "
	 << endl;
    abort();
  }
#endif
  return data[i*ny+j];
}

double& Mat::operator () (const unsigned int i, const unsigned int j) {
#ifdef MATVECT_CHECK_BOUNDS
  if (i>=nx || j>=ny || i<0 || j<0) { 
    cout << "Mat::operator () overflow i,j nx,ny " 
	 << i << "," << j << " "
	 << nx << "," << ny << " "
	 << endl;
    abort();
  }
#endif
  return data[i*ny+j];
}

void Mat::dump(ostream& Stream) const {
    for(unsigned int j=0;j<ny;j++) {
      Stream << "0.." << nx-1 << "," << j;
      for(unsigned int i=0;i<nx;i++) {
	Stream << " " << float((*this)(i,j));
      }
      Stream << endl;
    }
  }

void Mat::Identity() {
  if(nx!=ny) {
    cout << "Mat::Identity ERROR nx!=ny" <<endl;
    abort();
  }
  Zero();
  for(unsigned int i=0;i<nx;++i)
    (*this)(i,i)=1.;
}

static bool same_size(const Mat& m1, const Mat& m2)
{
  if ((m1.SizeX() == m2.SizeX()) && (m1.SizeX() == m2.SizeY())) return true;
  cout << " matrices have different sizes" << endl;
  abort(); // we should in fact throw an exception.
  return false;
}




Mat Mat::operator +(const Mat& Right) const
{
  same_size(*this,Right);
  Mat res = (*this);
  res += Right;
  return res;
}

Mat Mat::operator -(const Mat& Right) const
{
  same_size(*this,Right);
  Mat res = (*this);
  res -= Right;
  return res;
}

void Mat::operator +=(const Mat& Right)
{
  same_size(*this,Right);
  double *a = data;
  const double *b = Right.Data();
  unsigned int size = nx*ny;
  for(unsigned int i=0;i<size;++i, ++a, ++b)
    *a += *b;
}

void Mat::operator -=(const Mat& Right)
{
  same_size(*this,Right);
  double *a = data;
  const double *b = Right.Data();
  unsigned int size = nx*ny;
  for(unsigned int i=0;i<size;++i, ++a, ++b)
    *a -= *b;
}

Mat Mat::operator *(const double Right) const 
{
  Mat res = (*this);
  res *= Right;
  return res;
}

Mat operator *(const double Left, const Mat &Right)
{
  Mat res = Right;
  res *= Left;
  return res;
}
 
void Mat::operator *=(const double Right)
{
  int size = nx*ny;
  double *a = data;
  for(int i=0;i<size;++i, ++a)
    *a *= Right;
}

Mat Mat::operator *(const Mat& Right) const
{
  if(nx != Right.SizeY()) {
    cout << "Mat::operator *= ERROR nx != Right.SizeY()" << endl;
    abort();
  }
  Mat res(Right.SizeX(),ny);
  
  for(unsigned int i=0;i<res.SizeX();++i) {
    for(unsigned int j=0;j<res.SizeY();++j) {
      for(unsigned int k=0;k<nx;++k) {
	res(i,j) += (*this)(k,j)*Right(i,k);
      }
    }
  }
  return res;
}

void Mat::operator *=(const Mat& Right)
{
  Mat res = (*this)*Right;
  (*this) = res;
}

//=================================================================

Vect::Vect(const unsigned int N) {
  data = NULL; 
  n=0;
  if(N<=0) {
    cout << "Vect::Vect ERROR N = " << N <<  endl;
  }else{
    allocate(N);
  }
}

Vect::Vect(const Vect& other) {
  data = NULL; 
  n=0;
  allocate(other.Size());
  memcpy(data,other.Data(),sizeof(double)*n); 
}

void Vect::allocate(const unsigned int N) {
  if(N<=0) {
    cout << "Vect::allocate ERROR N = " << N <<  endl;
  }
  if(n!=N) {
    n=N;
    if (data) 
      delete [] data;
    data = new double[n];
  }
  Zero();
};


double Vect::operator () (const unsigned int i) const {
#ifdef MATVECT_CHECK_BOUNDS
  if (i>=n || i<0) {
    cout << "Vec::operator () overflow i,n " 
	 << i << "," << n << endl;
    abort();
  }
#endif
  return data[i];
}

double& Vect::operator () (const unsigned int i) {
#ifdef MATVECT_CHECK_BOUNDS
  if (i>=n || i<0) {
    cout << "Vec::operator () overflow i,n " 
	 << i << "," << n << endl;
    abort();
  }
#endif
  return data[i];
}

void Vect::dump(ostream& Stream) const {
  for(unsigned int i=0;i<n;i++) {
    Stream << i << " " << float((*this)(i)) << endl;
  }
}