#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <climits>
#include <cstring>
#include <string>
#include <ctime>
#include <packlib.h>

#include <poloka/fitsimage.h>

#define NWPAWC 100000
float pawc_[NWPAWC];

static FILE* kumac;

static char* BareFileName(char *FileName) {
  char *p = FileName+strlen(FileName) - 1;
  while (p>=FileName && *p != '/') p--;
  return p+1;
}

static int image_histos(char *FileName, const char *MaskName=0) {

  if (!FileName) return 0;

  FitsImage img(FileName);
  static int id =11;
  static int idmask =101;

  int nx = img.Nx();
  int ny = img.Ny();
  int ntot = nx*ny;
  //double gain=img.KeyVal("GAIN");

  FitsImage *pmask = NULL;
  if (MaskName && (strlen(MaskName) > 1 ) ) {
    pmask = new FitsImage(MaskName) ;
    if ( pmask->Nx() != nx ||  pmask->Ny() != ny ){
      cerr << " image_histos: size mask != size  image\n";
      delete pmask;
      return 0;
    }
  }

  char histName[80]; /* with HBOOK, have to pass a real array, not a pointer as far as I remember */
  char histMName[80]; /* with HBOOK, have to pass a real array, not a pointer as far as I remember */

  //  strcpy(histName,BareFileName(FileName));
  strcpy(histName,FileName);
  string temp1  = histName;
  string temp = "mask" + temp1 ;
  strcpy(histMName, temp.c_str());
  //Image *image = &img;
  //*image *= gain;

  double average = 0;
  double variance = 0;
  double value;
  double sigma = 0; 
  float aa, ss ;
  img.SkyLevel(&aa,&ss);
  average = aa ;
  sigma = ss ;
  cout << " image_histos: image "<< histName  << " mean " 
       << average << " sqrt(mean) " << sqrt(average) 
       << " sigma " << sigma << " count " << ntot << endl;

  //  if (average > 2.0)
  char *nloo = getenv("NLOOP");
  int Nloop = 3 ;
  if ( nloo != NULL) {
    sscanf(nloo,"%d", &Nloop);
    cout << " image_histos: N loop : " << Nloop << endl;
  }
  
  int Nsig = 3.5 ;

  if (true) {
    for(int loop=0; loop< Nloop; loop++) {
      int count=0;
      double mean = average;
      average = 0.0;
      variance = 0.0;
      for (int j=0; j<ny; j++) 
	for (int i=0; i<nx; i++) {
	  value = img(i,j);
	  if (fabs(value-mean)<Nsig*sigma) {
	    average += value;
	    variance += value*value;
	    count++;
	  }
	}
      average /= count;
      variance = (variance/(count-1)) - (average*average);
      sigma = sqrt(variance);
      cout << " image_histos: image "<< histName  << " mean " << average 
	   << " sqrt(mean) " << sqrt(average) << " sigma " 
	   << sigma << " count " << count << endl;
    }

    HBOOK1(id,histName,50, average-5.*sigma, average+5.*sigma, 0);
    if (pmask)
      HBOOK1(idmask,histMName,50, average-10.*sigma, average+10.*sigma, 0);
  } else {
    HBOOK1(id,histName,50, -0.5, 1.5, 0);
    if (pmask)
      HBOOK1(idmask,histMName,50, -0.5, 1.5, 0);
  }
  
  for (int i=0; i<nx; i++) 
    for (int j=0; j<ny; j++) {
      HFILL(id,img(i,j),0.,1.);
      //  if (img(i,j)<=0.1)
      //  cout << "img("<<i<<","<<j<<") = "<<img(i,j)<<endl;
      if (pmask)
	if ((*pmask)(i,j)< 1.e-10)
	  HFILL(idmask,img(i,j),0.,1.);      
    }


  fprintf(kumac,"h/plot %d\n",id);
  id++;
  if (pmask) {
    fprintf(kumac,"h/plot %d\n",idmask);
    idmask++;
    delete pmask;
  }
  return 1;
}

static void usage(const char* progname) {
  cerr << "Usage: " << progname << " [OPTION] FITS...\n"
       << "Produce a sky histogram from a FITS image\n\n"
       << "    -m FITS: use mask FITS\n\n";
  exit(EXIT_FAILURE);
}

int main(int nargs,char **args) {

  if (nargs<2) usage(args[0]);

  vector<char*> imList;
  imList.reserve(nargs-1);
  char *maskName = 0;

  for (int i=1; i< nargs; ++i) {
    char *arg = args[i];
    if (arg[0] != '-') {
      imList.push_back(arg); 
      continue;
    }
    switch (arg[1]) {
    case 'm': maskName = args[++i]; break;
    default: usage(args[0]);
    }
  }

  int istat;
  int lrec = 1024; 
  HLIMIT(NWPAWC);
#define HBK_FILE_NAME "histos.hbk"

  char toto1[50] = "TOPDIR";
  char toto2[50] = HBK_FILE_NAME;
  char toto3[50] = "N";

  HROPEN(1, toto1, toto2, toto3, lrec, istat);
  if (istat != 0) {
    cerr << args[0] << ": cannot open "<< HBK_FILE_NAME << endl;
    return EXIT_FAILURE;
  }

  char *plotfile = "plot_all.kumac";
  kumac = fopen(plotfile, "w");
  if (!kumac) {
    cerr << args[0] << ": cannot open " << plotfile << endl;
    return EXIT_FAILURE;
  }
  fprintf(kumac,"h/file 1 %s\n", HBK_FILE_NAME);

  for (size_t i=1 ; i<imList.size(); i++) {
    cout << args[0] << ": processing image " << i <<" : "<< imList[i]
	 << " --- histo : " << i+10 << endl;
    image_histos(imList[i], maskName);
  }
  
  int icycle;
  char toto4[50] = " ";
  HROUT(0, icycle, toto4);
  HREND(toto1);
  
  return EXIT_SUCCESS;
}
