#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void usage(char *prog)
{
  fprintf(stdout, "Usage: %s logfile Thread ID\n", prog);
  fprintf(stdout, "\t%s creates logfile-%%15d.log file. %%15d is given Thread ID.\n", prog);
  return;
}

int main(int argc, char *argv[])
{
  FILE *fp, *fp_out;
  uint64_t process;
  char buf[4096];
  int flag = 0;
  uint64_t p, p_o = 0;
  char out_file[128];

  if (argc < 3) {
    fprintf(stderr, "is not given file name or Thread ID\n");
    usage(argv[0]);
    return 1;
  }

  process = atoll(argv[2]);

  if (snprintf(out_file, sizeof(out_file), "%s-%lu.log", argv[1], process) >= sizeof(out_file)) {
    fprintf(stderr, "can not generate output file name.\n%s may be too long.\n", argv[1]);
    return 4;
  }

  if ((fp = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "can not open %s\n", argv[1]);
    return 2;
  }

  fp_out = fopen(out_file, "w");
  while (fgets(buf, 4096, fp) != NULL) {
    if (sscanf(buf, "%15lu:", &p) == 1) {
      // fprintf(stdout, "read process id = %d\n", p);
      if (p != process) {
        p_o = 0;
        continue;
      }
      p_o = p;
      fputs(buf, fp_out);
    } else if (p_o == process) {
      fputs(buf, fp_out);
    }
  }

  fclose(fp);
  fclose(fp_out);
  
  return 0;
}
