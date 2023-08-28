#include "wc.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  long fsize;
  FILE *fp;
  count_t count, total;
  struct timespec begin, end;
  int nChildProc = 0;		
		
  /* 1st arg: filename */
  if(argc < 2) {
    printf("usage: wc <filname> [# processes] [crash rate]\n");
    return 0;
  }
		
  /* 2nd (optional) arg: number of child processes */
  if (argc > 2) {
    nChildProc = atoi(argv[2]);
    if(nChildProc < 1) nChildProc = 1;
    if(nChildProc > 10) nChildProc = 10;
  }

  /* 3rd (optional) arg: crash rate between 0% and 100%. Each child process has that much chance to crash*/
  if(argc > 3) {
    crashRate = atoi(argv[3]);
    if(crashRate < 0) crashRate = 0;
    if(crashRate > 50) crashRate = 50;
    printf("crashRate RATE: %d\n", crashRate);
  }
		
  printf("# of Child Processes: %d\n", nChildProc);
  printf("crashRate RATE: %d\n", crashRate);

  count.linecount = 0;
  count.wordcount = 0;
  count.charcount = 0;
  total.linecount = 0;
  total.wordcount = 0;
  total.charcount = 0;

  // start to measure time
  clock_gettime(CLOCK_REALTIME, &begin);

  int fd[nChildProc + 1][2];
  int i = 0;
  int status;
  pid_t pid;
  if (nChildProc != 0)
    {
      for (int j = 0; j < nChildProc; j ++)
	{
	  pipe(fd[j]);
	}

      for (i = 0; i < nChildProc; i ++)
	{
	  pid = fork();
	  if (pid == 0)
	    {
	      close(fd[i][0]);
	      // Open file in read-only mode
	      fp = fopen(argv[1], "r");

	      if(fp == NULL) {
		printf("File open error: %s\n", argv[1]);
		printf("usage: wc <filname>\n");
		return 0;
	      }

	      // get a file size
	      fseek(fp, 0L, SEEK_END);
	      fsize = ftell(fp);
			    
	      /* word_count() has 3 arguments.
	       * 1st: file descriptor
	       * 2nd: starting offset
	       * 3rd: number of bytes to count from the offset
	       */
	      if (i == nChildProc - 1 && fsize%nChildProc != 0)
		{
		  count = word_count(fp, (fsize/nChildProc)*i, (fsize/nChildProc) + (fsize%nChildProc));
		}
	      else
		{
		  count = word_count(fp, (fsize/nChildProc)*i, fsize/nChildProc);
		}
	      fclose(fp);
	      write(fd[i][1], &count, sizeof(count_t));
	      close(fd[i][1]);
	      exit(0);
	    }
	  waitpid(pid, &status, 0);
	  if (WIFSIGNALED(status))
	    {
	      i --;
	    }
	}

      for (int j = 0; j < nChildProc; j ++)
	{
	  read(fd[j][0], &count, sizeof(count_t));
	  total.linecount += count.linecount;
	  total.wordcount += count.wordcount;
	  total.charcount += count.charcount;

	  close(fd[j][0]);
	  close(fd[j][1]);
	}
    }

  else
    {
      // Open file in read-only mode
      fp = fopen(argv[1], "r");

      if(fp == NULL) {
	printf("File open error: %s\n", argv[1]);
	printf("usage: wc <filname>\n");
	return 0;
      }
		
      // get a file size
      fseek(fp, 0L, SEEK_END);
      fsize = ftell(fp);
		
      /* word_count() has 3 arguments.
       * 1st: file descriptor
       * 2nd: starting offset
       * 3rd: number of bytes to count from the offset
       */
		
      count = word_count(fp, 0, fsize);

      fclose(fp);
    }
		
  clock_gettime(CLOCK_REALTIME, &end);
  long seconds = end.tv_sec - begin.tv_sec;
  long nanoseconds = end.tv_nsec - begin.tv_nsec;
  double elapsed = seconds + nanoseconds*1e-9;

  printf("\n========= %s =========\n", argv[1]);
  printf("Total Lines : %d \n", total.linecount);
  printf("Total Words : %d \n", total.wordcount);
  printf("Total Characters : %d \n", total.charcount);
  printf("======== Took %.3f seconds ========\n", elapsed);

  return(0);
}

