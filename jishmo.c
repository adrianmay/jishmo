#include <stdio.h>

#include <libavutil/motion_vector.h>
#include <libavformat/avformat.h>

static AVFormatContext *fmt_ctx = NULL;

int extract_motion_vectors(char *src_filename){
  int ret = 0;
  if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) { //So the link works too
    fprintf(stderr, "Could not open source file %s\n", src_filename);
    exit(1);
  }
}

int main(int argc, char **argv) { 
  return extract_motion_vectors(argv[1]); 
}


