#include "AvswuUtils.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "cauchy.h"
#include "jerasure.h"
#include "liberation.h"
#include "reed_sol.h"
#include "timing.h"
#include <assert.h>
#include <errno.h>
#include <gf_rand.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif

#include <spdlog/spdlog.h>
#include <string>

using namespace std;

#define N 10

enum Coding_Technique {
  Reed_Sol_Van,
};

/* Global variables for signal handler */
int readins, n;
enum Coding_Technique method;

int jfread(void *ptr, int size, int nmembers, FILE *stream) {
  if (stream != NULL)
    return fread(ptr, size, nmembers, stream);

  MOA_Fill_Random_Region(ptr, size);
  return size;
}

int main(int argc, char **argv) {
  FILE *fp, *fp2;     // file pointers
  char *block;        // padding file
  int size, newsize;  // size of file and temp size
  struct stat status; // finding file size

  enum Coding_Technique tech; // coding technique (parameter)
  int k, m, w, packetsize;    // parameters
  int buffersize;             // paramter
  int i;                      // loop control variables
  int blocksize;              // size of k+m files
  int total;
  int extra;

  /* Jerasure Arguments */
  char **data;
  char **coding;
  int *matrix;

  /* Creation of file name variables */
  char temp[5];
  char *s1, *s2, *extension;
  char *fname;
  int md;
  char *curdir;

  /* Timing variables */
  struct timing t1, t2, t3, t4;
  double tsec;
  double totalsec;
  struct timing start;

  // turn on debug
  spdlog::set_level(spdlog::level::debug);

  /* Start timing */
  timing_set(&t1);
  totalsec = 0.0;
  matrix = NULL;

  packetsize = 0;
  buffersize = 0;

  /* Setting of coding technique and error checking */
  tech = Reed_Sol_Van;
  w = 8;
  k = 6;
  m = 3;

  // use a test data file, or argv[1]
  string input_file =
      string(AVSWU_BASEDIR) + "/veins-client/test_data/test_data_10_shard.data";
  if (argc > 1) {
    input_file = argv[1];
  }

  /* Set global variable method for signal handler */
  method = tech;

  /* Get current working directory for construction of file names */
  curdir = (char *)malloc(sizeof(char) * 1000);
  assert(curdir == getcwd(curdir, 1000));

  /* Open file and error check */

  fp = fopen(input_file.c_str(), "rb");
  if (fp == NULL) {
    fprintf(stderr, "Unable to open file.\n");
    exit(0);
  }

  /* Create Coding directory */
  i = mkdir("Coding", S_IRWXU);
  if (i == -1 && errno != EEXIST) {
    fprintf(stderr, "Unable to create Coding directory.\n");
    exit(0);
  }

  /* Determine original size of file */
  stat(input_file.c_str(), &status);
  size = status.st_size;

  newsize = size;

  /* Find new size by determining next closest multiple */
  if (packetsize != 0) {
    if (size % (k * w * packetsize * sizeof(long)) != 0) {
      while (newsize % (k * w * packetsize * sizeof(long)) != 0)
        newsize++;
    }
  } else {
    if (size % (k * w * sizeof(long)) != 0) {
      while (newsize % (k * w * sizeof(long)) != 0)
        newsize++;
    }
  }

  if (buffersize != 0) {
    while (newsize % buffersize != 0) {
      newsize++;
    }
  }

  /* Determine size of k+m files */
  blocksize = newsize / k;

  /* Allow for buffersize and determine number of read-ins */
  if (size > buffersize && buffersize != 0) {
    if (newsize % buffersize != 0) {
      readins = newsize / buffersize;
    } else {
      readins = newsize / buffersize;
    }
    block = (char *)malloc(sizeof(char) * buffersize);
    blocksize = buffersize / k;
  } else {
    readins = 1;
    buffersize = size;
    block = (char *)malloc(sizeof(char) * newsize);
  }

  /* Break inputfile name into the filename and extension */
  s1 = (char *)malloc(sizeof(char) * (strlen(input_file.c_str()) + 20));
  s2 = strrchr((char *)input_file.c_str(), '/');
  if (s2 != NULL) {
    s2++;
    strcpy(s1, s2);
  } else {
    strcpy(s1, input_file.c_str());
  }
  s2 = strchr(s1, '.');
  if (s2 != NULL) {
    extension = strdup(s2);
    *s2 = '\0';
  } else {
    extension = strdup("");
  }

  /* Allocate for full file name */
  fname = (char *)malloc(sizeof(char) *
                         (strlen(input_file.c_str()) + strlen(curdir) + 20));
  sprintf(temp, "%d", k);
  md = strlen(temp);

  /* Allocate data and coding */
  data = (char **)malloc(sizeof(char *) * k);
  coding = (char **)malloc(sizeof(char *) * m);
  for (i = 0; i < m; i++) {
    coding[i] = (char *)malloc(sizeof(char) * blocksize);
    if (coding[i] == NULL) {
      perror("malloc");
      exit(1);
    }
  }

  spdlog::debug("blocksize={}", blocksize);

  /* Create coding matrix */
  timing_set(&t3);
  matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

  timing_set(&start);
  timing_set(&t4);
  totalsec += timing_delta(&t3, &t4);

  /* Read in data until finished */
  n = 1;
  total = 0;

  while (n <= readins) {
    /* Check if padding is needed, if so, add appropriate
       number of zeros */
    if (total < size && total + buffersize <= size) {
      total += jfread(block, sizeof(char), buffersize, fp);
    } else if (total < size && total + buffersize > size) {
      extra = jfread(block, sizeof(char), buffersize, fp);
      for (i = extra; i < buffersize; i++) {
        block[i] = '0';
      }
    } else if (total == size) {
      for (i = 0; i < buffersize; i++) {
        block[i] = '0';
      }
    }

    /* Set pointers to point to file data */
    for (i = 0; i < k; i++) {
      data[i] = block + (i * blocksize);
    }

    timing_set(&t3);
    /* Encode according to coding method */
    jerasure_matrix_encode(k, m, w, matrix, data, coding, blocksize);

    // data debug to see values in data array
    // char temp[blocksize];
    // for (uint32_t i = 0; i < k; i++) {
    //   for (size_t j = 0; j < blocksize; j++) {
    //     temp[j] = data[i][j];
    //   }
    //   spdlog::debug("data[{}][575]={}", i, temp[575]);
    //   spdlog::debug("data[{}][576]={}", i, temp[575]);
    // }

    timing_set(&t4);

    /* Write data and encoded data to k+m files */
    for (i = 1; i <= k; i++) {
      if (fp == NULL) {
        bzero(data[i - 1], blocksize);
      } else {
        sprintf(fname, "%s/Coding/%s_k%0*d%s", curdir, s1, md, i, extension);
        if (n == 1) {
          fp2 = fopen(fname, "wb");
        } else {
          fp2 = fopen(fname, "ab");
        }
        fwrite(data[i - 1], sizeof(char), blocksize, fp2);
        fclose(fp2);
      }
    }
    for (i = 1; i <= m; i++) {
      if (fp == NULL) {
        bzero(data[i - 1], blocksize);
      } else {
        sprintf(fname, "%s/Coding/%s_m%0*d%s", curdir, s1, md, i, extension);
        if (n == 1) {
          fp2 = fopen(fname, "wb");
        } else {
          fp2 = fopen(fname, "ab");
        }
        fwrite(coding[i - 1], sizeof(char), blocksize, fp2);
        fclose(fp2);
      }
    }
    n++;
    /* Calculate encoding time */
    totalsec += timing_delta(&t3, &t4);
  }

  // parse filename from path to input file
  std::string base_filename =
      input_file.substr(input_file.find_last_of("/\\") + 1);
  std::string::size_type const p(base_filename.find_last_of('.'));
  std::string file_without_extension = base_filename.substr(0, p);

  /* Create metadata file */
  if (fp != NULL) {
    sprintf(fname, "%s/Coding/%s_meta.txt", curdir, s1);
    fp2 = fopen(fname, "wb");
    fprintf(fp2, "%s\n", file_without_extension.c_str());
    fprintf(fp2, "%d\n", size);
    fprintf(fp2, "%d %d %d %d %d\n", k, m, w, packetsize, buffersize);
    fprintf(fp2, "%s\n", "reed_sol_van");
    fprintf(fp2, "%d\n", tech);
    fprintf(fp2, "%d\n", readins);
    fclose(fp2);
  }

  /* Free allocated memory */
  free(s1);
  free(fname);
  free(block);
  free(curdir);

  /* Calculate rate in MB/sec and print */
  timing_set(&t2);
  tsec = timing_delta(&t1, &t2);
  printf("Encoding (MB/sec): %0.10f\n",
         (((double)size) / 1024.0 / 1024.0) / totalsec);
  printf("En_Total (MB/sec): %0.10f\n",
         (((double)size) / 1024.0 / 1024.0) / tsec);

  return 0;
}
