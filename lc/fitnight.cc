#include <math.h>
#include <iostream>
#include <fstream>
#include <matvect.h>

#include "lightcurvepoint.h"
#include "dictfile.h"

using namespace std;

static double sq(const double x) { return x*x;}


int main(int argc, char **argv)
{
  bool fitsingleflux=false;
  if(argc>1) {
    if(strcmp(argv[1],"-single")==0) {
      fitsingleflux=true;
      cout << "fit of a single constant flux for all points" << endl;
    }
  }

 
  // vecteur de flux
  Vect FluxVec;
  {
    Mat m;
    if(m.readFits("vec_sn.fits")!=0)
      return -1;
    FluxVec = m;
  }
  int nflux = FluxVec.Size();
  //cout << FluxVec << endl;
  
  // matrice de covariance
  Mat CovarianceMat;  
  if(CovarianceMat.readFits("pmat_sn.fits")!=0)
    return -1;
  //cout << "CovarianceMat" << endl;
  //cout << CovarianceMat << endl;

  Mat FluxCovarianceMat = CovarianceMat.SubBlock(0,nflux-1,0,nflux-1);
  FluxCovarianceMat.Symmetrize("L");
  

  Mat A;
  if(!fitsingleflux) {
    A.readFits("nightmat_sn.fits");
  }else{  
    A.allocate(1,nflux);
    for(int i=0;i<nflux;++i)
      A(0,i)=1;
  }
  Mat Abis = A; // we save a copy for output 

#ifdef DEBUG
  cout << "A before cleaning:"  << endl;
  cout << A << endl;

  cout << "FluxVec before cleaning:"  << endl;
  cout << FluxVec << endl;
  
  cout << "FluxCovarianceMat before cleaning:"  << endl;
  cout << FluxCovarianceMat << endl;
#endif 

  // ==== remove points without data ====
  for(unsigned int i=0; i< FluxVec.Size(); i++) {
    if(fabs(FluxVec(i))<1.e-30) {
      //cout << "removing " << i << " : ";
      //cout << FluxCovarianceMat.SizeX() << " => ";
      FluxCovarianceMat = FluxCovarianceMat.WithoutRows(i,i);
      FluxCovarianceMat = FluxCovarianceMat.WithoutColumns(i,i);
      //cout << FluxCovarianceMat.SizeX() << endl;
      Mat mFluxVec = FluxVec;
      FluxVec = mFluxVec.WithoutRows(i,i);
      A = A.WithoutRows(i,i);
      i--;
    }
  }
#ifdef DEBUG
  cout << "FluxVec after cleaning:"  << endl;
  cout << FluxVec << endl;
  cout << "A after cleaning:"  << endl;
  cout << A << endl;
  cout << "FluxCovarianceMat after cleaning:"  << endl;
  cout << FluxCovarianceMat << endl;
#endif
  
  vector<int> suppressedfluxes;
  Vect flux_per_night;
  Mat FluxPerNightCovMat;
  Mat AtWA;
  Mat FluxWeightMat;

  int noutliers = 0;

  while(true) {
  
    FluxWeightMat = FluxCovarianceMat;
    FluxWeightMat.SymMatInvert();
    //cout << "FluxWeightMat" << endl;
    //cout << FluxWeightMat << endl;
    AtWA = A.transposed()*FluxWeightMat*A;
    FluxPerNightCovMat= AtWA; 
    FluxPerNightCovMat.SymMatInvert();
    flux_per_night = (FluxPerNightCovMat*(A.transposed()*FluxWeightMat))*FluxVec;
#ifdef DEBUG
    cout << "Mean flux per night" << endl;
    cout << flux_per_night << endl;
    cout << "Covariance matrix" << endl;
    cout <<FluxPerNightCovMat<< endl;
#endif
    // now compute chi2
    Vect B = FluxVec - A*flux_per_night;

    double chi2 = B.transposed()*FluxWeightMat*B;     
    int ndf = A.SizeY()-A.SizeX();

#ifdef DEBUG
    cout << "chi2 = " << chi2 << endl;   
    cout << "ndf = " << ndf << endl;
    cout << "chi2/ndf = " << chi2/ndf << endl;
#endif
   
    if(chi2/ndf<1.5 || suppressedfluxes.size()>=6)
      break;

    int outlier = -1;
    double flux_chi2;
    double chi2_max = 0;
    for(unsigned int iflux = 0 ;iflux<B.Size();++iflux) {
      
      flux_chi2 = sq(B(iflux))*FluxWeightMat(iflux,iflux);
      if(flux_chi2>chi2_max) {
	chi2_max = flux_chi2;
	outlier = iflux;
      }
    }
    
    cout << outlier << " " << sqrt(chi2_max) << endl;
    
    if(sqrt(chi2_max)<3.)
      break;
    noutliers ++;
    suppressedfluxes.push_back(outlier);
    // on vire cet outlier de toutes les matrices
    A = A.WithoutRows(outlier,outlier);
    {
      Mat mFluxVec = FluxVec;
      FluxVec = mFluxVec.WithoutRows(outlier,outlier);
      //cout << FluxVec << endl;
    }
    FluxCovarianceMat = FluxCovarianceMat.WithoutRows(outlier,outlier);
    FluxCovarianceMat = FluxCovarianceMat.WithoutColumns(outlier,outlier);
  }
  
  // OUTPUT
  // ============================================================================================
  
  // compute chi2
  Vect B = FluxVec - A*flux_per_night;
  double chi2 = B.transposed()*FluxWeightMat*B;     
  int ndf = A.SizeY()-A.SizeX();
  
  cout << "SUMMARY " 
       << FluxVec.Size() << " " 
       << flux_per_night.Size() << " "
       << noutliers << " " 
       << chi2/ndf << endl;
  
  // if chi2dof>0 scale all errors
  double chi2ndf = chi2/ndf;
  if(chi2ndf>1) {
    for(unsigned int j= 0; j<FluxPerNightCovMat.SizeY();j++)
      for(unsigned int i= 0; i<FluxPerNightCovMat.SizeX();i++) {
	FluxPerNightCovMat(i,j) *= chi2ndf;
	AtWA(i,j) /= chi2ndf;
      }
  }
  
  

  // save these results in ASCII files
  // ... TODO ...
  {
    {
      ofstream st("flux_per_expo_covmat.dat");
      st.setf(ios::fixed);
      st << FluxCovarianceMat;
      st.close();
    }
    FluxCovarianceMat.writeFits("flux_per_expo_covmat.fits");
    
    {
      ofstream st("flux_per_night_covmat.dat");
      st.setf(ios::fixed);
      st <<  FluxPerNightCovMat;
      st.close();
    }
    FluxPerNightCovMat.writeFits("flux_per_night_covmat.fits");
    
    {
      ofstream st("flux_per_expo_weightmat.dat");
      st.setf(ios::fixed);
      st << FluxWeightMat;
      st.close();
    }
    FluxWeightMat.writeFits("flux_per_expo_weightmat.fits");
    
    {
      ofstream st("flux_per_night_weightmat.dat");
      st.setf(ios::fixed);
      st <<  AtWA;
      st.close();
    }
    AtWA.writeFits("flux_per_night_weightmat.fits");
    
  }
  
  /*
  // now we read the lightcurve point list to get julian dates and zero point
  vector< CountedRef<LightCurvePoint> > lcpoints;
  {
    vector< CountedRef<LightCurvePoint> > lcpoints_all;
    obj_input<xmlstream> oi("lc.xml");
    oi >> lcpoints_all;
    oi.close();
    
    // no keep only points with flux!=0 (i.e. fitted points)
    vector< CountedRef<LightCurvePoint> >::iterator it = lcpoints_all.begin();
    for(;it!=lcpoints_all.end();++it) {
      //cout << (*it)->julianday << " " << (*it)->flux;
      if((*it)->flux!=0) {
	lcpoints.push_back(*it);
      }
      //cout << endl;
    }
  }
  */
  DictFile lcdata("lc2fit.dat");
  string instrumentName = lcdata.GlobalValue("INSTRUMENT");
  string bandName = lcdata.GlobalValue("BAND");
  string magSystem = lcdata.GlobalValue("MAGSYS");
  
  
  // get zero point 
  double zp = lcdata.front().Value("ZP");
  //cout << "zp=" << zp << endl;
  
  // read light curve points
  vector< CountedRef<LightCurvePoint> > lcpoints;
  for (DictFileCIterator line = lcdata.begin(); line != lcdata.end(); 
       ++line) {
    CountedRef<LightCurvePoint> lcp = new LightCurvePoint();
    lcp->julianday = line->Value("Date");
    lcp->flux = line->Value("Flux");
    lcpoints.push_back(lcp);
  }

  ofstream outputlc("lc2fit_per_night.dat");
  outputlc << "#Date : (days since January 1st, 2003)\n"
	   << "#Flux : \n"  
	   << "#Fluxerr : \n"
	   << "#ZP : elixir zp\n";
  outputlc << "@INSTRUMENT " << instrumentName << endl;
  outputlc << "@BAND " << bandName << endl;
  outputlc << "@MAGSYS " << magSystem << endl;
  outputlc.setf(ios::fixed);
  
  
  vector< CountedRef<LightCurvePoint> > newlcpoints;
  double jd;
  int nexpo;
  for(unsigned int night = 0; night < flux_per_night.Size(); ++ night) {
    CountedRef<LightCurvePoint> newpoint = new LightCurvePoint();
    newpoint->flux = flux_per_night(night);
    newpoint->eflux = sqrt(FluxPerNightCovMat(night,night));
    newpoint->computemag(zp);
    // now get julian day (mean of all exposures)
    jd=0;
    nexpo=0;
    for(unsigned int expo=0;expo<Abis.SizeY();++expo) {
      if(Abis(night,expo)>0.5) {
	jd += lcpoints[expo]->julianday;
	//cout << night << " " 
	//     << expo << " " 
	//     << lcpoints[expo]->julianday-2452854.0 << endl;
	nexpo++;
      }
    }
    newpoint->julianday = jd/nexpo;
    
    outputlc << (*newpoint) << endl;
    newlcpoints.push_back(newpoint);
  }
  outputlc.close();
  
  return 0;
}
