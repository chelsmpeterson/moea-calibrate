#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <locale.h>

/* root path */
char root_path[] = "C:\\Users\\cmptrsn2\\moea-calibrate";

/* function that makes new run folder for problem */
void makeFolder(char* site, char* run_folder);

int main(int argc, char* argv[]) {
  // site name and run folder
  char* site = argv[1];
  char* run_folder = argv[2];

  makeFolder(site, run_folder);

  return(0);
}

/* function that makes new run folder for problem */
void makeFolder(char* site, char* run_folder) {
  char files[11][30] = {"RZWQM.DAT","RZINIT.DAT","CNTRL.DAT","Gleams.DAT",
                        "ipnames.DAT","plgen.DAT","mzcer040.CUL","mzdssat.rzx",
                        "sbgro040.CUL","sbdssat.rzx","expdata.DAT"};
  int i;

  // make scenario path
  char scen_path[200];
  snprintf(scen_path, sizeof(scen_path), "%s\\Scenarios", root_path);

  // string for warehouse folder
  char wh_folder[50];
  snprintf(wh_folder, sizeof(wh_folder), "Site%s_cal_wh", site);

  // make new directory
  chdir(scen_path);
  mkdir(run_folder);

  // copy directory and contents
  char copy_path[200];
  for (i=0; i<11; i++) {
    snprintf(copy_path, sizeof(copy_path), "copy %s\\%s %s\\%s > NUL", wh_folder, files[i], run_folder, files[i]);
    system(copy_path);
  }

  // make path to run folder
  char run_path[300];
  snprintf(run_path, sizeof(run_path), "%s\\%s", scen_path, run_folder);

  /* edit ipnames.dat file */
  FILE *fp_in, *fp_out;
  char row[400];
  char ip_files[8][12] = {"cntrl.dat","rzwqm.dat","MetData.MET","MetData.BRK",
                          "rzinit.dat","plgen.dat","MetData.SNO",".ana"};
  char file_path[400];

  chdir(run_path);
  fp_in = fopen("ipnames.dat","r+");
  fp_out = fopen("ipnames_new.dat","w");
  i = 0;
  while (fgets(row, 400, (FILE*)fp_in)) {
    if (i == 0 || i == 1 || i == 4 || i == 5) {
      snprintf(file_path, sizeof(file_path), "%s\\%s\n", run_path, ip_files[i]);
      fprintf(fp_out, file_path);
    } else if (i == 2 || i == 3 || i == 6) {
      snprintf(file_path, sizeof(file_path), "%s\\Meteorology\\Site%s%s\n", scen_path, site, ip_files[i]);
      fprintf(fp_out, file_path);
    } else if (i == 7) {
      snprintf(file_path, sizeof(file_path), "%s\\%s%s\n", run_path, run_folder, ip_files[i]);
      fprintf(fp_out, file_path);
    } else {
      fprintf(fp_out, row);
    }
    i++;
  }
  fclose(fp_in); fclose(fp_out);
  remove("ipnames.dat");
  rename("ipnames_new.dat","ipnames.dat");

  /* edit mzdssat.rzx file */

  // path to DSSAT database
  char db_path[300];
  char db_path_caps[300];
  snprintf(db_path, sizeof(db_path), "%s\\databases\\dssat", root_path);
  strcpy(db_path_caps, db_path);
  for (i = 0; db_path[i]!='\0'; i++) {
     if (db_path[i] >= 'a' && db_path[i] <= 'z') {
        db_path_caps[i] = db_path[i] - 32;
     }
  }
  strcat(db_path_caps, "\\\n");

  // path to scenario
  char run_path_caps[300];
  strcpy(run_path_caps, run_path);
  for (i = 0; run_path[i]!='\0'; i++) {
     if (run_path[i] >= 'a' && run_path[i] <= 'z') {
        run_path_caps[i] = run_path[i] - 32;
     }
  }
  strcat(run_path_caps, "\\\n");

  fp_in = fopen("mzdssat.rzx","r+");
  fp_out = fopen("mzdssat_new.rzx","w");
  i = 0;
  while (fgets(row, 300, (FILE*)fp_in)) {
    if (strcmp(site, "A") == 0) {
      if (i == 70) {
        fprintf(fp_out, db_path_caps); // update database path
      } else if (i == 71) {
        fprintf(fp_out, run_path_caps); // update run path
      } else {
        fprintf(fp_out, row);
      }
    } else if (strcmp(site, "E") == 0) {
      if (i == 66) {
        fprintf(fp_out, db_path_caps); // update database path
      } else if (i == 67) {
        fprintf(fp_out, run_path_caps); // update run path
      } else {
        fprintf(fp_out, row);
      }
    }
    i++;
  }
  fclose(fp_in); fclose(fp_out);
  remove("mzdssat.rzx");
  rename("mzdssat_new.rzx","mzdssat.rzx");

  /* edit sbdssat.rzx file */
  fp_in = fopen("sbdssat.rzx","r+");
  fp_out = fopen("sbdssat_new.rzx","w");
  i = 0;
  while (fgets(row, 300, (FILE*)fp_in)) {
    if (i == 70) {
      fprintf(fp_out, db_path_caps); // update database path
    } else if (i == 71) {
      fprintf(fp_out, run_path_caps); // update run path
    } else {
      fprintf(fp_out, row);
    }
    i++;
  }
  fclose(fp_in); fclose(fp_out);
  remove("sbdssat.rzx");
  rename("sbdssat_new.rzx","sbdssat.rzx");
}
