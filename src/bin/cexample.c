#include <stdio.h>
#include <stdlib.h>
#include <csvmtool.h>

int main(int argc, char *argv[]) {
  if ( argc <= 1 ) {
    fprintf(stderr, "Usage: main <Model>\n");
    fprintf(stderr, "For example: main /mnt/hda4/pfc/models/eng/WSJTP\n");
    exit(0);
  }

  SVMToolTagger * t = NULL;

  fprintf(stderr,"Api Sample Program: Tagging \"Hello World! It is a sample program.\"\n\n");
  fprintf(stderr,"\n Steps\n");
  fprintf(stderr,"_______\n");
  fprintf(stderr,"1.- Create Tagger with function --> SVMToolTaggerCreate(<Model>)\n");
  fprintf(stderr,"\n\t\t SVMToolTagger * t = SVMToolTaggerCreate(\"%s\")\n\n",argv[1]);
  t = SVMToolCreateTagger(argv[1]);

  fprintf(stderr,"2.- Initialize Tagger --> SVMToolTaggerInitialize(<Tagger>,\n");
  fprintf(stderr,"                                                  <Strategy>,\n");
  fprintf(stderr,"                                                  <Direction>,\n");
  fprintf(stderr,"                                                  <Sliding Window Length>,\n");
  fprintf(stderr,"                                                  <Sliding Window Interest Point>,\n");
  fprintf(stderr,"                                                  <Weight Filter for Known Words>,\n");
  fprintf(stderr,"                                                  <Weight Filter for Unkown>)\n");

  fprintf(stderr,"\n\t\tSVMToolInitializeTagger(t, 0, \"LR\", 7, 3, 0, 0)\n\n");
  SVMToolInitializeTagger(t, 0, "LR", 7, 3, 0, 0);

  fprintf(stderr,"3.- Tagging Sentence --> SVMToolTaggerRun(<Tokenized Text>,<Number of words>)\n");
  fprintf(stderr,"\n\t\tSVMToolRunTagger(t, \"Hello world ! It is a sample program .\",9)\n\n");
  SVMToolResult *ret = SVMToolRunTagger(t, "Hello world ! It is a sample program .",9);

  fprintf(stderr,"4.- Results\n\n");
  SVMToolResultPrint(ret);

  SVMToolResultDelete(ret);

  fprintf(stderr,"\n5.- Destroying objects --> SVMToolDestroyTagger(t)\n\n");
  SVMToolDestroyTagger(t);

  return 0;
}
