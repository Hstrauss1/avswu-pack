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

#include "AvswuUtils.h"

using namespace std;
using namespace avswu_utils;

#define N 10

/* Global variables for signal handler */
int readins, n;

/* Function prototype */
void ctrl_bs_handler(int dummy);

int main(int argc, char **argv) {
  FILE *fp; // File pointer

  /* Jerasure arguments */
  char **data;
  char **coding;
  int *erasures;
  int *erased;
  int *matrix;

  /* Parameters */
  int k, m, w, packetsize, buffersize;
  int tech;
  char *c_tech;

  int i, j;           // loop control variable, s
  int blocksize = 0;  // size of individual files
  int origsize;       // size of file before padding
  int total;          // used to write data, not padding to file
  struct stat status; // used to find size of individual files
  int numerased;      // number of erased files

  /* Used to recreate file names */
  char *temp;
  char *cs1, *cs2, *extension;
  char *fname;
  int md;

  /* Used to time decoding */
  struct timing t1, t2, t3, t4;
  double tsec;
  double totalsec;

  signal(SIGQUIT, ctrl_bs_handler);

  matrix = NULL;
  totalsec = 0.0;

  /* Start timing */
  timing_set(&t1);

  /* Error checking parameters */
  string input_file;
  if (argc == 2) {
    input_file = argv[1];
  } else {
    input_file = "test_data/test_data_10_shard.data";
  }

  string curr_dir = string(AVSWU_BASEDIR) + "/veins-client";

  spdlog::info("input_file={}", input_file);

  /* Begin recreation of file names */
  cs1 = (char *)malloc(sizeof(char) * strlen(input_file.c_str()));
  cs2 = strrchr((char *)input_file.c_str(), '/');
  if (cs2 != NULL) {
    cs2++;
    strcpy(cs1, cs2);
  } else {
    strcpy(cs1, input_file.c_str());
  }
  cs2 = strchr(cs1, '.');
  if (cs2 != NULL) {
    extension = strdup(cs2);
    *cs2 = '\0';
  } else {
    extension = strdup("");
  }
  fname =
      (char *)malloc(sizeof(char *) * (100 + strlen(input_file.c_str()) + 20));

  spdlog::info("curdir={}", curr_dir);

  /* Read in parameters from metadata file */
  sprintf(fname, "%s/Coding/%s_meta.txt", curr_dir.c_str(), cs1);

  fp = fopen(fname, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Error: no metadata file %s\n", fname);
    exit(1);
  }
  temp = (char *)malloc(sizeof(char) * (strlen(input_file.c_str()) + 20));
  if (fscanf(fp, "%s", temp) != 1) {
    fprintf(stderr, "Metadata file - bad format\n");
    exit(0);
  }

  if (fscanf(fp, "%d", &origsize) != 1) {
    fprintf(stderr, "Original size is not valid\n");
    exit(0);
  }
  if (fscanf(fp, "%d %d %d %d %d", &k, &m, &w, &packetsize, &buffersize) != 5) {
    fprintf(stderr, "Parameters are not correct\n");
    exit(0);
  }
  c_tech = (char *)malloc(sizeof(char) * (strlen(input_file.c_str()) + 20));
  if (fscanf(fp, "%s", c_tech) != 1) {
    fprintf(stderr, "Metadata file - bad format\n");
    exit(0);
  }
  if (fscanf(fp, "%d", &tech) != 1) {
    fprintf(stderr, "Metadata file - bad format\n");
    exit(0);
  }
  if (fscanf(fp, "%d", &readins) != 1) {
    fprintf(stderr, "Metadata file - bad format\n");
    exit(0);
  }
  fclose(fp);

  /* Allocate memory */
  erased = (int *)malloc(sizeof(int) * (k + m));
  for (i = 0; i < k + m; i++)
    erased[i] = 0;
  erasures = (int *)malloc(sizeof(int) * (k + m));

  data = (char **)malloc(sizeof(char *) * k);
  coding = (char **)malloc(sizeof(char *) * m);
  if (buffersize != origsize) {
    for (i = 0; i < k; i++) {
      data[i] = (char *)malloc(sizeof(char) * (buffersize / k));
    }
    for (i = 0; i < m; i++) {
      coding[i] = (char *)malloc(sizeof(char) * (buffersize / k));
    }
    blocksize = buffersize / k;
  }

  sprintf(temp, "%d", k);
  md = strlen(temp);
  timing_set(&t3);

  /* Create coding matrix or bitmatrix */
  matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

  timing_set(&t4);
  totalsec += timing_delta(&t3, &t4);

  /* Begin decoding process */
  total = 0;
  n = 1;
  while (n <= readins) {
    numerased = 0;
    /* Open files, check for erasures, read in data/coding */
    for (i = 1; i <= k; i++) {
      sprintf(fname, "%s/Coding/%s_k%0*d%s", curr_dir.c_str(), cs1, md, i,
              extension);
      fp = fopen(fname, "rb");
      if (fp == NULL) {
        erased[i - 1] = 1;
        erasures[numerased] = i - 1;
        numerased++;
        // printf("%s failed\n", fname);
      } else {
        if (buffersize == origsize) {
          stat(fname, &status);
          blocksize = status.st_size;
          data[i - 1] = (char *)malloc(sizeof(char) * blocksize);
          assert(blocksize == fread(data[i - 1], sizeof(char), blocksize, fp));
        } else {
          fseek(fp, blocksize * (n - 1), SEEK_SET);
          assert(buffersize / k ==
                 fread(data[i - 1], sizeof(char), buffersize / k, fp));
        }
        fclose(fp);
      }
    }
    for (i = 1; i <= m; i++) {
      sprintf(fname, "%s/Coding/%s_m%0*d%s", curr_dir.c_str(), cs1, md, i,
              extension);
      fp = fopen(fname, "rb");
      if (fp == NULL) {
        erased[k + (i - 1)] = 1;
        erasures[numerased] = k + i - 1;
        numerased++;
        // printf("%s failed\n", fname);
      } else {
        if (buffersize == origsize) {
          stat(fname, &status);
          blocksize = status.st_size;
          coding[i - 1] = (char *)malloc(sizeof(char) * blocksize);
          assert(blocksize ==
                 fread(coding[i - 1], sizeof(char), blocksize, fp));
        } else {
          fseek(fp, blocksize * (n - 1), SEEK_SET);
          assert(blocksize ==
                 fread(coding[i - 1], sizeof(char), blocksize, fp));
        }
        fclose(fp);
      }
    }
    /* Finish allocating data/coding if needed */
    if (n == 1) {
      for (i = 0; i < numerased; i++) {
        if (erasures[i] < k) {
          data[erasures[i]] = (char *)malloc(sizeof(char) * blocksize);
        } else {
          coding[erasures[i] - k] = (char *)malloc(sizeof(char) * blocksize);
        }
      }
    }

    erasures[numerased] = -1;
    timing_set(&t3);

    // int no_erasures[1];
    // no_erasures[0] = -1;

    /* START TEMP FOR DEBUGGING */
    vector<vector<char>> data_view = create_data_view(data, blocksize, k);
    vector<vector<char>> coding_view = create_data_view(coding, blocksize, m);

    for (size_t i = 0; i < (k + m); i++) {
      spdlog::debug("erasures[{}]={}", i, erasures[i]);
    }

    /* END TEMP FOR DEBUGGING */

    /* Choose proper decoding method */
    i = jerasure_matrix_decode(k, m, w, matrix, 1, erasures, data, coding,
                               blocksize);
    timing_set(&t4);

    /* Exit if decoding was unsuccessful */
    if (i == -1) {
      fprintf(stderr, "Unsuccessful!\n");
      exit(0);
    }

    /* START TEMP FOR DEBUGGING */
    data_view = create_data_view(data, blocksize, k);
    coding_view = create_data_view(coding, blocksize, m);
    /* END TEMP FOR DEBUGGING */

    /* Create decoded file */
    sprintf(fname, "%s/Coding/%s_decoded%s", curr_dir.c_str(), cs1, extension);
    if (n == 1) {
      fp = fopen(fname, "wb");
    } else {
      fp = fopen(fname, "ab");
    }
    for (i = 0; i < k; i++) {
      if (total + blocksize <= origsize) {
        fwrite(data[i], sizeof(char), blocksize, fp);
        total += blocksize;
      } else {
        for (j = 0; j < blocksize; j++) {
          if (total < origsize) {
            fprintf(fp, "%c", data[i][j]);
            total++;
          } else {
            break;
          }
        }
      }
    }
    n++;
    fclose(fp);
    totalsec += timing_delta(&t3, &t4);
  }

  /* Free allocated memory */
  free(cs1);
  free(extension);
  free(fname);
  free(data);
  free(coding);
  free(erasures);
  free(erased);

  /* Stop timing and print time */
  timing_set(&t2);
  tsec = timing_delta(&t1, &t2);
  printf("Decoding (MB/sec): %0.10f\n",
         (((double)origsize) / 1024.0 / 1024.0) / totalsec);
  printf("De_Total (MB/sec): %0.10f\n\n",
         (((double)origsize) / 1024.0 / 1024.0) / tsec);

  return 0;
}

void ctrl_bs_handler(int dummy) {
  time_t mytime;
  mytime = time(0);
  fprintf(stderr, "\n%s\n", ctime(&mytime));
  fprintf(stderr, "You just typed ctrl-\\ in decoder.c\n");
  fprintf(stderr, "Total number of read ins = %d\n", readins);
  fprintf(stderr, "Current read in: %d\n", n);
  signal(SIGQUIT, ctrl_bs_handler);
}
