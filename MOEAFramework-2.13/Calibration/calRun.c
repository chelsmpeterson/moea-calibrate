/* external file for substituting soil hydraulic, nutrient, and crop yield
   parameters into RZWQM.DAT files and evaluating performance criteria */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "moeaframework.h"

/* root path */
char root_path[] = "C:\\Users\\cmptrsn2\\moea-calibrate";

/* number of variables, objectives, and constraints */
int nvars = 32;
int nobjs = 4;
int ncons = 0;

/* conversions */
float cmPerM = 100.0;
float m2PerHa = pow(10, 4);
float ugPerKg = pow(10, 9);
float kgPerMg = 1000.0;

/* functions and variables for calculating no. of days between dates */
const int monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
struct Date {int d, m, y; };
int countLeaps(struct Date dt);
int getDays(struct Date dt);
int getDiff(struct Date dt1, struct Date dt2);

/* objective function */
double mean(double y[], int n);
double RRMSE(double y_obs[], double y_pred[], int n);

/* function for Brooks-Corey soil moisture curve */
double thetaCurve(double theta_r, double theta_s, double lam, double tau_b, double tau);

/* function for implementing RZWQM given new input parameters and evaluating
   calibration performance criteria */
void evaluate(char* site, char* run_folder, double* vars, double* objs);

/* main routine for parsing the decision variable inputs, evaluating the
   problem, and printing the objectives */
int main(int argc, char* argv[]) {
  // site name and run folder
  char* site = argv[1];
  char* run_folder = argv[2];

  // initialize variables and objectives
 	double vars[nvars];
 	double objs[nobjs];

 	// initialize algorithm
	MOEA_Init(nobjs, ncons);

 	// implement algorithm
 	while (MOEA_Next_solution() == MOEA_SUCCESS) {
 		MOEA_Read_doubles(nvars, vars);
 		evaluate(site, run_folder, vars, objs);
 		MOEA_Write(objs, NULL);
 	}

  // end algorithm
 	MOEA_Terminate();

 	return EXIT_SUCCESS;
}

/* function for implementing RZWQM given new input parameters and evaluating
   calibration performance criteria */
void evaluate(char* site, char* run_folder, double* vars, double* objs) {
  FILE *fp, *fp_in, *fp_out;
  char row_in[255], row_out[255];
  char *row_array[50];
  int i, j, k, n;

  /* make paths */

  // observations path
  char obs_path[200];
  snprintf(obs_path, sizeof(obs_path), "%s\\MOEAFramework-2.13\\Calibration\\Observations", root_path);

  // run path
  char run_path[200];
  snprintf(run_path, sizeof(run_path), "%s\\Scenarios\\%s", root_path, run_folder);

  // binary path
  char bin_path[200];
  snprintf(bin_path, sizeof(bin_path), "%s\\bin\\RZWQMrelease.exe", root_path);

  /* start and end dates of simulation and calibration periods */
  struct Date simStDate = {1, 1, 1990};
  struct Date calStDate = {1, 1, 1993};
  struct Date calEndDate = {31, 12, 1996};

  /* read in observations */

  // arrays for storing tile flow, tile N, and crop yield data
  int nDays = getDiff(calStDate, calEndDate) + 1;
  int startYr = calStDate.y;
  int endYr = calEndDate.y;
  int nYears = endYr - startYr + 1;
  int nCornYrs = nYears/2;
  double obsFlow[nDays];
  double obsLoad[nDays];
  double obsYield[nYears];
  double obsCornYield[nCornYrs];
  double obsSbYield[nCornYrs];

  // start and end indices of calibration period in observation dataframes
  struct Date obsStDate = {1, 1, 1993};
  int startCalInd = getDiff(obsStDate, calStDate) + 1;
  int endCalInd = getDiff(obsStDate, calEndDate) + 1;

  // get column in observation dataframes for site
  char sites[2][2] = {"A", "E"};
  int site_cols[] = {1, 2};
  for (i=0; i<2; i++) {
    if (strcmp(site, sites[i]) == 0)
      col_n = site_cols[i];
  }

  // tile flow observations
  chdir(obs_path);
  fp = fopen("tileFlowObserved.csv","r");
  i = 0; // row counter
  j = 0; // day counter
  while (fgets(row_in, 255, (FILE*)fp)) {
    if (i >= startCalInd && i <= endCalInd) {
      // split row
      char *token = strtok(row_in, ",");
      k = 0;
      while (token != NULL) {
        row_array[k] = token;
        token = strtok(NULL, ",");
        k++;
      }
      obsFlow[j] = atof(row_array[col_n]); // m/d
      j++;
    }
    i++;
  }
  fclose(fp);

  // tile N observations
  fp = fopen("tileNObserved.csv","r");
  i = 0; // row counter
  j = 0; // day counter
  while (fgets(row_in, 255, (FILE*)fp)) {
    if (i >= startCalInd && i <= endCalInd) {
      // split row
      char *token = strtok(row_in, ",");
      k = 0;
      while (token != NULL) {
        row_array[k] = token;
        token = strtok(NULL, ",");
        k++;
      }
      obsLoad[j] = atof(row_array[col_n]); // kg/ha
      j++;
    }
    i++;
  }
  fclose(fp);

  // crop yield observations
  fp = fopen("cropYieldObserved.csv","r");
  i = 0; // row counter
  j = 0; // year counter
  while (fgets(row_in, 255, (FILE*)fp)) {
    if (i >= 1) { // skip column names
      // split row
      char *token = strtok(row_in, ",");
      k = 0;
      while (token != NULL) {
        row_array[k] = token;
        token = strtok(NULL, ",");
        k++;
      }

      // save crop yields within year range
      if (atoi(row_array[0]) >= startYr && atoi(row_array[0]) <= endYr) {
        obsYield[j] = atof(row_array[col_n])*kgPerMg; // kg/ha
        j++;
      }
    }
    i++;
  }
  fclose(fp);

  // sort observed crop yields into corn and soybean yields
  if (strcmp(site,"A") == 0 || strcmp(site,"E") == 0) {
    for (i=0; i<nCornYrs; i++) {
      obsCornYield[i] = obsYield[2*i];
      obsSbYield[i] = obsYield[2*i+1];
    }
  } else {
    for (i=0; i<nCornYrs; i++) {
      obsCornYield[i] = obsYield[2*i+1];
      obsSbYield[i] = obsYield[2*i];
    }
  }

  /* sort hydraulic, nutrient, and crop parameters */
  int n_hydro_par = 13;
  int n_nutri_par = 3;
  int n_corn_par = 5;
  int n_soy_par = 11;
  double hydroPar[n_hydro_par];
  char nutriPar[n_nutri_par][255];
  char cornPar[n_corn_par][255];
  char soyPar[n_soy_par][255];

  // hydrologic params
  j = 0;
  for (i=0; i<n_hydro_par; i++) {
    hydroPar[i] = vars[j];
    j++;
  }

  // nutrient params
  for (i=0; i<n_nutri_par; i++) {
    snprintf(nutriPar[i], sizeof(nutriPar[i]), "%.3E", vars[j]);
    j++;
  }

  // corn params
  for (i=0; i<n_corn_par; i++) {
    if (i == 1 || i == 4)
    {
      snprintf(cornPar[i], sizeof(cornPar[i]), "%.3f", vars[j]);
    } else {
      snprintf(cornPar[i], sizeof(cornPar[i]), "%.1f", vars[j]);
    }
    j++;
  }

  // soybean params
  for (i=0; i<n_soy_par; i++) {
    if (i == 1 || i == 5 || i == 8 || i == 10) {
     snprintf(soyPar[i], sizeof(soyPar[i]), "%.3f", vars[j]);
    } else if (i == 6 || i == 7) {
     snprintf(soyPar[i], sizeof(soyPar[i]), "%.1f", vars[j]);
    } else {
     snprintf(soyPar[i], sizeof(soyPar[i]), "%.2f", vars[j]);
    }
    j++;
  }

  /* substitute new parameters into RZWQM.DAT file */
  int start_soil_row = 169;
  int end_soil_row = 189;
  int n_layers = 7;
  double tb_i, lam_i, n2_i, ks_i, tr_i, ts_i, tbk_i, c2_i, lks_i, dhdL;
  char tb_buf[255], lam_buf[255], ks_buf[255], theta_buf[255], c2_buf[255], lks_buf[255], dhdL_buf[255];
  double tau_all[] = {333, 100, 15000};

  chdir(run_path);
  fp_in = fopen("RZWQM.DAT","r");
  fp_out = fopen("RZWQM_new.DAT","w");

  /* write file up to soil hydraulic parameters */
  for (i=0; i<start_soil_row; i++) {
    fgets(row_in, 255, (FILE*)fp_in);
    fprintf(fp_out, row_in);
  }

  /* write soil hydraulic parameters */
  for (i=0; i<n_layers; i++) {
    /* sort hydro parameter values */
    if (i == 0) {
      tb_i = hydroPar[1];
      ks_i = hydroPar[4];
      lks_i = hydroPar[7];
      lam_i = hydroPar[10];
    } else if (i >= 1 && i <= 5) {
      tb_i = hydroPar[2];
      ks_i = hydroPar[5];
      lks_i = hydroPar[8];
      lam_i = hydroPar[11];
    } else {
      tb_i = hydroPar[3];
      ks_i = hydroPar[6];
      lks_i = hydroPar[9];
      lam_i = hydroPar[12];
    }

    /* first row */
    fgets(row_in, 255, (FILE*)fp_in);

    // split row
    char *token = strtok(row_in, " ");
    j = 0;
    while (token != NULL) {
      row_array[j] = token;
      token = strtok(NULL, " ");
      j++;
    }

    // get second exponent on K-curve, theta residual, and theta saturaton
    n2_i = atof(row_array[3]);
    tr_i = atof(row_array[5]);
    ts_i = atof(row_array[6]);

    // sub new tb, lambda, and ksat values into buffers
    snprintf(tb_buf, sizeof(tb_buf), "%.6lf", tb_i);
    snprintf(lam_buf, sizeof(lam_buf), "%.6lf", lam_i);
    snprintf(ks_buf, sizeof(ks_buf), "%.6lf", ks_i);

    // reconstruct row string
    strcpy(row_out, row_array[0]);
    strcat(row_out, "  ");
    for (k=1; k<j; k++) {
      if (k == 1) {
        strcat(row_out, tb_buf);
        strcat(row_out, "  ");
      } else if (k == 2) {
        strcat(row_out, lam_buf);
        strcat(row_out, "  ");
      } else if (k == 4) {
        strcat(row_out, ks_buf);
        strcat(row_out, "  ");
      } else if (k < (j-1)) {
        strcat(row_out, row_array[k]);
        strcat(row_out, "  ");
      } else {
        strcat(row_out, row_array[k]);
      }
    }

    // write new row string to RZWQM.DAT
    fprintf(fp_out, row_out);

    /* second row */
    fgets(row_in, 255, (FILE*)fp_in);

    // split row
    token = strtok(row_in, " ");
    j = 0;
    while (token != NULL) {
      row_array[j] = token;
      token = strtok(NULL, " ");
      j++;
    }

    // set TbK = Tb
    tbk_i = tb_i;

    // update C2 parameter based on new Ksat and Tbk
    c2_i = ks_i*pow(tbk_i, n2_i);
    snprintf(c2_buf, sizeof(c2_buf), "%.2E", c2_i);

    // reconstruct row string
    snprintf(theta_buf, sizeof(theta_buf), "%.6lf", thetaCurve(tr_i, ts_i, lam_i, tb_i, tau_all[0])); // update theta 1/3
    strcpy(row_out, theta_buf);
    strcat(row_out, "  ");
    for (k=1; k<j; k++) {
      if (k <= 2) { // update theta 1/10 and 15 bar
        snprintf(theta_buf, sizeof(theta_buf), "%.6lf", thetaCurve(tr_i, ts_i, lam_i, tb_i, tau_all[k]));
        strcat(row_out, theta_buf);
        strcat(row_out, "  ");
      } else if (k == 3) {
        strcat(row_out, tb_buf); // tbk = tb
        strcat(row_out, "  ");
      } else if (k == 4) {
        strcat(row_out, c2_buf);
        strcat(row_out, "  ");
      } else if (k < (j-1)) {
        strcat(row_out, row_array[k]);
        strcat(row_out, "  ");
      } else {
        strcat(row_out, row_array[k]);
      }
    }

    // write new row string to RZWQM.DAT
    fprintf(fp_out, row_out);

    /* third row (just LKsat) */
    fgets(row_in, 255, (FILE*)fp_in);

    // sub new LKsat value into buffer
    snprintf(lks_buf, sizeof(lks_buf), "%.6lf\n", lks_i);

    // write new LKsat to RZWQM.DAT file
    fprintf(fp_out, lks_buf);
  }

  /* write file up to dhdL */
  int dhdL_row = 246;
  for (i = end_soil_row+1; i<dhdL_row; i++) {
    fgets(row_in, 255, (FILE*)fp_in);
    fprintf(fp_out, row_in);
  }

  /* write dhdL row */
  dhdL = hydroPar[0];
  fgets(row_in, 255, (FILE*)fp_in);
  char *token = strtok(row_in, " ");
  j = 0;
  while (token != NULL) {
    row_array[j] = token;
    token = strtok(NULL, " ");
    j++;
  }
  strcpy(row_out, row_array[0]);
  strcat(row_out, "  ");
  for (k=1; k<j; k++) {
    if (k == 15) {
      snprintf(dhdL_buf, sizeof(dhdL_buf), "%.3E", dhdL);
      strcat(row_out, dhdL_buf);
      strcat(row_out, "  ");
    } else if (k < (j-1)) {
      strcat(row_out, row_array[k]);
      strcat(row_out, "  ");
    } else {
      strcat(row_out, row_array[k]);
    }
  }
  fprintf(fp_out, row_out);

  /* sub in new nutrient parameters */

  // write file up to reaction rate constants
  int rxn_row = 640;
  for (i = dhdL_row+1; i<rxn_row; i++) {
    fgets(row_in, 255, (FILE*)fp_in);
    fprintf(fp_out, row_in);
  }

  // write reaction rate row (kden)
  fgets(row_in, 255, (FILE*)fp_in);
  token = strtok(row_in, " ");
  j = 0;
  while (token != NULL) {
    row_array[j] = token;
    token = strtok(NULL, " ");
    j++;
  }
  strcpy(row_out, row_array[0]);
  strcat(row_out, "  ");
  for (k=1; k<j; k++) {
   if (k == 2) {
     strcat(row_out, nutriPar[0]); // kden
     strcat(row_out, "  ");
   } else if (k < (j-1)) {
     strcat(row_out, row_array[k]);
     strcat(row_out, "  ");
   } else {
     strcat(row_out, row_array[k]);
   }
  }
  fprintf(fp_out, row_out);

  // write file up to decay rate constants
  int decay_row = 648;
  for (i = rxn_row+1; i<decay_row; i++) {
    fgets(row_in, 255, (FILE*)fp_in);
    fprintf(fp_out, row_in);
  }

  // write decay rate row
  fgets(row_in, 255, (FILE*)fp_in);
  token = strtok(row_in, " ");
  j = 0;
  while (token != NULL) {
    row_array[j] = token;
    token = strtok(NULL, " ");
    j++;
  }
  strcpy(row_out, row_array[0]);
  strcat(row_out, "  ");
  for (k=1; k<j; k++) {
    if (k == 1) {
      strcat(row_out, nutriPar[1]); // kfrp
      strcat(row_out, "  ");
    } else if (k == 4) {
      strcat(row_out, nutriPar[2]); // kshp
      strcat(row_out, "\n");
    } else if (k < (j-1)) {
      strcat(row_out, row_array[k]);
      strcat(row_out, "  ");
    }
  }
  fprintf(fp_out, row_out);

  /* write rest of file */
  while (fgets(row_in, 255, (FILE*)fp_in)) {
    fprintf(fp_out, row_in);
  }
  fclose(fp_in);
  fclose(fp_out);

  /* replace RZWQM.DAT file */
  remove("RZWQM.DAT");
  rename("RZWQM_new.DAT","RZWQM.DAT");

  //* sub in new corn cultivar parameters into .CUL files *//

  /* update mzcer040.CUL file */
  fp_in = fopen("mzcer040.cul","r");
  fp_out = fopen("mzcer040_new.cul","w");
  int corn_row = 13;
  i = 0; // row counter
  while (fgets(row_in, 255, (FILE*)fp_in)) {
    if (i == corn_row) {
     // split row
     char *token = strtok(row_in, " ");
     j = 0;
     while (token != NULL) {
       row_array[j] = token;
       token = strtok(NULL, " ");
       j++;
     }

     // sub in new parameters
     strcpy(row_out, row_array[0]);
     strcat(row_out, " ");
     for (k=1; k<j; k++) {
       if (k == 2) {
         strcat(row_out, row_array[k]);
         strcat(row_out, "       ");
       } else if (k >= 4 && k <= 8) {
         strcat(row_out, cornPar[k-4]);
         strcat(row_out, " ");
       } else if (k < (j-1)) {
         strcat(row_out, row_array[k]);
         strcat(row_out, " ");
       } else {
         strcat(row_out, row_array[k]);
       }
     }
     fprintf(fp_out, row_out);
    } else {
     fprintf(fp_out, row_in);
    }
    i++;
  }
  fclose(fp_in);
  fclose(fp_out);

  // replace .CUL file
  remove("mzcer040.cul");
  rename("mzcer040_new.cul","mzcer040.cul");

  /* update sbgro040.CUL file */
  fp_in = fopen("sbgro040.cul","r");
  fp_out = fopen("sbgro040_new.cul","w");
  int soy_row = 20;
  int soy_param_no[] = {5, 6, 7, 9, 10, 12, 13, 14, 16, 17, 18};
  i = 0; // row counter
  while (fgets(row_in, 255, (FILE*)fp_in)) {
    if (i == soy_row) {
      // split row
      char *token = strtok(row_in, " ");
      j = 0;
      while (token != NULL) {
        row_array[j] = token;
        token = strtok(NULL, " ");
        j++;
      }

      // sub in new parameters
      n = 0; // soybean parameter counter
      strcpy(row_out, row_array[0]);
      strcat(row_out, " ");
      for (k=1; k<j; k++) {
        if (k == soy_param_no[n]) {
          strcat(row_out, soyPar[n]);
          strcat(row_out, " ");
          n++;
        } else if (k == 2) {
          strcat(row_out, row_array[k]);
          strcat(row_out, "   ");
        } else if (k == 3) {
          strcat(row_out, row_array[k]);
          strcat(row_out, "      ");
        } else if (k < (j-1)) {
          strcat(row_out, row_array[k]);
          strcat(row_out, " ");
        } else {
          strcat(row_out, row_array[k]);
        }
      }
      fprintf(fp_out, row_out);
    } else {
      fprintf(fp_out, row_in);
    }
    i++;
  }
  fclose(fp_in);
  fclose(fp_out);

  // replace .CUL file
  remove("sbgro040.cul");
  rename("sbgro040_new.cul","sbgro040.cul");

  /* run RZWQM with new parameters */
  char exe_str[300];
  snprintf(exe_str, sizeof(exe_str), "%s > NUL", bin_path);
  system(exe_str);

  /* read in model output */
  double simFlow[nDays];
  double simLoad[nDays];
  double simYield[nYears];
  double simCornYield[nCornYrs];
  double simSbYield[nCornYrs];

  /* tile flow and N load */

  // find indices for calibration start and end dates in DAILY.PLT
  int startSimInd = getDiff(simStDate, calStDate) + 58;
  int endSimInd = getDiff(simStDate, calEndDate) + 58;

  // get output from daily.plt
  fp = fopen("DAILY.PLT","r");
  i = 0; // row counter
  j = 0; // day counter
  while (fgets(row_in, 255, (FILE*)fp)) {
    if (i >= startSimInd && i <= endSimInd) {
      // split row
      char *token = strtok(row_in, " ");
      k = 0;
      while (token != NULL) {
        row_array[k] = token;
        token = strtok(NULL, " ");
        k++;
      }

      // save flow and N load data to arrays
      simFlow[j] = atof(row_array[2])/cmPerM; // m/d
      simLoad[j] = atof(row_array[6])*pow(cmPerM,2)*m2PerHa/ugPerKg; // convert ug/cm^2/d to kg/ha/d
      j++;
    }
    i++;
  }
  fclose(fp);

  /* crop yield */
  int yieldInd[nYears];

  // find index of calibration start and end years in OVERVIEW.OUT
	fp = fopen("OVERVIEW.OUT","r");
  i = 0; // row counter
  j = 0; // year counter
  while (fgets(row_in, 255, (FILE*)fp)) {
    // split row
		char *token = strtok(row_in, " ");
		k = 0;
		while (token != NULL) {
			row_array[k] = token;
			token = strtok(NULL, " ");
			k++;
		}

    // identify index of planting date for start year
		if (strcmp(row_array[0],"PLANTING") == 0 && strcmp(row_array[1],"DATE") == 0) {
      if (atoi(row_array[5]) >= startYr && atoi(row_array[5]) <= endYr) {
        yieldInd[j] = i;
        j++;
      }
		}
    i++;
  }
  fclose(fp);

  // collect crop yields from start year to end year
  fp = fopen("OVERVIEW.OUT","r");
  i = 0; // row counter
	j = 0; // year counter
	while (fgets(row_in, 255, (FILE*)fp)) {
    // split row
		char *token = strtok(row_in, " ");
		k = 0;
		while (token != NULL) {
			row_array[k] = token;
			token = strtok(NULL, " ");
			k++;
		}

    // identify rows with yields and save yields to array
    if (j < (nYears-1)) {
      if (i >= yieldInd[j] && i <= yieldInd[j+1]) {
        if (strcmp(row_array[0],"Maize") == 0 || strcmp(row_array[0],"Soybean") == 0) {
          if (strcmp(row_array[1],"YIELD") == 0) {
            simYield[j] = atof(row_array[3]); // kg/ha;
            j++;
          }
        }
      }
    } else if (j == (nYears-1)) {
      if (i >= yieldInd[j]) {
        if (strcmp(row_array[0],"Maize") == 0 || strcmp(row_array[0],"Soybean") == 0) {
          if (strcmp(row_array[1],"YIELD") == 0) {
            simYield[j] = atof(row_array[3]); // kg/ha;
            j++;
          }
        }
      }
    }
    i++;
	}
	fclose(fp);

  // sort simulated crop yields into corn and soybean yields
  if (strcmp(site,"A") == 0 || strcmp(site,"E") == 0) {
    for (i=0; i<nCornYrs; i++) {
      simCornYield[i] = simYield[2*i];
      simSbYield[i] = simYield[2*i+1];
    }
  } else {
    for (i=0; i<nCornYrs; i++) {
      simCornYield[i] = simYield[2*i+1];
      simSbYield[i] = simYield[2*i];
    }
  }

  /* evaluate objective functions */
  objs[0] = RRMSE(obsFlow, simFlow, nDays); // tile flow RRMSE
  objs[1] = RRMSE(obsLoad, simLoad, nDays); // tile N RRMSE
  objs[2] = RRMSE(obsCornYield, simCornYield, nCornYrs); // corn yield RRMSE
  objs[3] = RRMSE(obsSbYield, simSbYield, nCornYrs); // sb yield RRMSE
}

/* functions and variables for calculating no. of days between dates */

// function that counts number of leap years before a date
int countLeaps(struct Date dt) {
  int y = dt.y;
  if (dt.m <= 2)
    y--;
  return(floor(y/4) - floor(y/100) + floor(y/400));
}

// function that calculates number of days before a date
int getDays(struct Date dt) {
  // initialize count using years and day of month
  long int n = dt.y * 365 + dt.d;

  // add days for months prior to given date
  for (int i = 0; i < dt.m - 1; i++)
      n += monthDays[i];

  // add a day for every leap year
  n += countLeaps(dt);

  return(n);
}

// function that calculates number of days between dates
int getDiff(struct Date dt1, struct Date dt2) {
  // calculate number of days before dt1 and dt2
  long int n1 = getDays(dt1);
  long int n2 = getDays(dt2);

  return(n2-n1);
}

/* objective functions */

// function that calculates the mean of a vector
double mean(double y[], int n) {
  int i;
  double mu = 0;
  for (i=0; i<n; i++) {
    mu += y[i];
  }
  return(mu/(float)n);
}

double RRMSE(double y_obs[], double y_pred[], int n) {
  int i;
  double mu_obs = mean(y_obs, n);
  double rmse = 0;
  for (i=0; i<n; i++) {
    rmse += pow(y_pred[i]-y_obs[i],2);
  }
  return(sqrt(rmse/(float)n)/mu_obs);
}

/* function for Brooks-Corey soil moisture curve */
double thetaCurve(double theta_r, double theta_s, double lam, double tau_b, double tau) {
  return(theta_r + (theta_s - theta_r)*pow(tau/tau_b,-lam));
}
