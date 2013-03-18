#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

#include <packlib.h>

using namespace std;

#define NWPAWC 100000
float pawc_[NWPAWC];
#define TOPDIR "//TOPDIR"
#define MAXVAR 4096

static char *StringNew(char *Source) {
  char *toto;
  toto = (char *) calloc(1,strlen(Source)+1);
  strcpy(toto,Source);
  return toto;
}

static char * CutExtension(const char *Name) {
  static char result[512];
  char *dot;
  strcpy(result,Name);
  dot = strrchr(result, '.');
  if (dot) *dot = '\0';
  return result;
}

char** decode_tags(FILE *file, int *Dim) {

#define MAX_LINE_LENGTH 16384

  char **tags = (char **) calloc(MAXVAR,sizeof(char*));
  int i, dim = 0;
  char line[MAX_LINE_LENGTH];
  
  while (fgets(line,MAX_LINE_LENGTH,file)) {
    if (line[0] == '@') continue;
    if (line[0] != '#') {
      cerr << " decode_tags: header should end by '# end'\n"; 
      return NULL;
    }
    char w1[MAX_LINE_LENGTH], w2[MAX_LINE_LENGTH];
    if (sscanf(line+1,"%s",w1) == 1 && strcmp(w1,"end") == 0) break;
    if (dim == MAXVAR) { 
      cout << " decode_tags: tuple truncated to " << dim << " variables\n";
      continue;
    }
    if (sscanf(line+1," %[^: ] %s",w1,w2)==2  && w2[0] == ':' ) {
      tags[dim] = StringNew(w1);(dim)++;
      continue;
    }
  } /* end of input read */
  *Dim = dim;
  if (dim == 0) return 0;
  for (i =0; i<dim; ++i) cout << "decode_tags: " << i << " " << tags[i] << endl;
  return tags;
}

static char *split_line(char *Line, float *X, const int Dim, int *nread, int *nbad) {
  const char *p1;
  char dummy[MAX_LINE_LENGTH];
  char* p2;
  int i;
  *nread = 0;
  *nbad =0;
  if (strlen(Line) <= 1) return Line;
  memset(X,0,Dim*sizeof(X[0]));
  p1 = Line;
  for (i=0; i< Dim; ++i) {
    float value = strtod(p1,&p2);
    if (p2 == p1) { /* try to read a bunch of chars to go on */
      int nread=0;
      sscanf(p1,"%s%n",dummy,&nread);
      if (nread == 0) break; // means end of line
      (*nbad)++;
      p1 += nread;
      X[i] = 1e30;
    } else {
      X[i] = value;
      p1 = p2;
    }
    (*nread)++;
  }
  return p2;
}


static int read_data(FILE *File, int Dim, void (Processor)(int*,float*), 
	      int *ProcData, bool print_bad_lines) {
  char line[MAX_LINE_LENGTH];
  int count = 0;
  int miss = 0; int more = 0;
  int bad = 0;
  float x[MAXVAR];

  if (Dim>MAXVAR)
    Dim = MAXVAR;

  while (fgets(line,MAX_LINE_LENGTH,File)) {
    char *left_over;
    int nread;
    int nbad;
    if (line[0] == '#' || line[0] == '@') continue;
    size_t len=strlen(line);
    if (len <= 1) continue;
    if (line[len-1] == '\n') line[len-1] = '\0'; 
    left_over = split_line(line,x,Dim,&nread, &nbad);
    bool error = false;
    if (nread < Dim) {miss++; error=true;}
    if (atof(left_over)) {more++; error = true;}
    if (nbad) {bad++; error = true;}
    if (print_bad_lines && error)
      cerr << " read_data: bad line '" << line << "'\n";
    if (Processor) Processor(ProcData,x);
    count ++;
  }
  if (miss)
    cerr << " read_data: missed items on " << miss << " rows\n";
  if (more)
    cerr << " read_data: too many items on " << more << " rows \n";
  if (bad)
    cerr << " read_data: bad conversions on " << bad <<  " rows \n";
  return count;
}




static char* unslashed_topdir(char *Topdir) {
  if (strstr(Topdir,"//") == Topdir)
    return Topdir+2;
  else
    return Topdir;
}

static int open_hbook_file(char *name, const int Lrec) {
  int istat;
  int lrec = Lrec;
  char* topdir = unslashed_topdir(TOPDIR);
  HROPEN(1, topdir, name, "N", lrec, istat);
  if (istat == 0) return 1;
 return 0;
}

static int tuple_book(int Id, char *TupleName, char **Tags, int Dim) {
#define MAX_LENGTH 30
  char tags[MAXVAR][MAX_LENGTH]; /* emulate the implementation of fortran arrays of strings */
  int i;
  char title[128];
  if (Dim > MAXVAR) { 
    cout << " tuple_book:  tuple " << Id << " truncated\n"; 
    Dim = MAXVAR;
  }
  for (i=0; i<Dim; i++) {
    strncpy(tags[i], Tags[i], MAX_LENGTH-1);
  }
  strcpy(title,TupleName); /* may be useless, but not sure */
  HBOOKN(Id,title,Dim,TOPDIR,60000,tags);
  return 1;
}

static void tuple_end(int Id) {
  int icycle = 0;
  HROUT(Id,icycle," ");
  HREND(unslashed_topdir(TOPDIR));
}

static void my_hfn(int *Id,float *X) {
  HFN(*Id,X);
}

static int make_tuple(const char *AsciiName, char* HbkFileName, int Id, int Lrec) {
  FILE *in;
  char **tags;
  int dim;
  int nrec = 0;
  in = fopen(AsciiName,"r");
  if (!in) {
    cerr << " make_tuple: cannot open" << AsciiName << endl;
    return 0;
  }
  tags = decode_tags(in, &dim);
  if (!tags) return 0;
  if (!open_hbook_file(HbkFileName, Lrec)) {
    cerr << " make_tuple: could not open " << HbkFileName << endl;
    fclose(in);
    return 0;
  }
  tuple_book(Id, "toto", tags, dim);
  nrec = read_data(in, dim, my_hfn, &Id, /* print_bad_lines =  */ false);
  cout << " make_tuple: " << nrec << " entries written to "<<  HbkFileName << endl;
  tuple_end(Id);
  return 1;
}

static void usage(const char* progname) {
  cerr << "Usage: " << progname<< "[OPTION] LIST HBOOKFILE [-l lrec] \n"
       << "Convert a poloka list to a HBOOK n-tuple file\n\n"
       << "    -l INT : max record length (default: 1024)\n\n";
  exit(EXIT_FAILURE);
} 

int main(int argc, char **argv) {
  
  char *fileNames[2] = {NULL,NULL};
  int lrec = 1024;
  int count = 0;

  if (argc < 3) usage(argv[0]);

  for (int i=1; i< argc; ++i) {
    char *arg= argv[i];
    if (arg[0] == '-' && arg[1] != '\0') {
      switch (arg[1]) {
      case 'l': ++i; lrec = atoi(argv[i]); break;
      case 'h':
      default:  usage(argv[0]);
      }
    } else {
      fileNames[count++] = arg;
    }
  }

  if (count == 0 || count > 2) usage(argv[0]);

  char *hbkName=NULL;
  char string[256];

  if (count == 1) {
    sprintf(string,"%s.hbk",CutExtension(fileNames[0]));
    hbkName = string;
  } else 
    hbkName = fileNames[1];
  HLIMIT(NWPAWC);

  if (!make_tuple(fileNames[0], hbkName, 1, lrec)) 
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
