package CalProbs;
import java.io.*;
import java.util.*;
import org.moeaframework.problem.ExternalProblem;
import org.moeaframework.core.Solution;
import org.moeaframework.core.variable.RealVariable;

/* class that defines the RZWQM2 calibration problem, including the number of
   parameters (variables), objectives (performance metrics), and constraints
   (parameter ranges) */

public class calProb1 extends ExternalProblem {
  // name of site being calibrated
  String site;

  // run folder for site being calibrated with all files required to run RZWQM2
  String run_folder;

  public calProb1(String site, String run_folder) throws IOException {
    super("./Calibration/calRun.exe", site, run_folder);
    this.site = site;
    this.run_folder = run_folder;
  }

  @Override
  public String getName() {
    return "calProb1";
  }

  @Override
  public int getNumberOfVariables() {
    return 32; // number of calibration parameters
  }

  @Override
  public int getNumberOfObjectives() {
    return 4; // number of calibration objectives
  }

  @Override
  public int getNumberOfConstraints() {
    return 0; // numer of calibration constraints (apart from parameter ranges)
  }

  /* read in hydrologic, nutrient, and crop parameter ranges from csv files
     in ParamRanges folder */
  @Override
  public Solution newSolution() {
    Solution solution = new Solution(getNumberOfVariables(), getNumberOfObjectives());

    /* hydrologic parameter ranges */
    int i = 0; // cumulative parameter counter
    try {
      File file = new File("./Calibration/ParamRanges/HydroRanges.csv");
      Scanner reader = new Scanner(file);
      int j = 0; // row counter
      while (reader.hasNextLine()) {
        String line = reader.nextLine();
        String[] entries = line.split("[,]", 0);
        if (j >= 1) {
          solution.setVariable(i, new RealVariable(Float.parseFloat(entries[1]),Float.parseFloat(entries[2])));
          i++;
        }
        j++;
      }
      reader.close();
    } catch (FileNotFoundException e) {
      System.out.println("An error occurred.");
      e.printStackTrace();
    }

    /* nutrient parameter ranges */
    try {
      File file = new File("./Calibration/ParamRanges/NutrientRanges.csv");
      Scanner reader = new Scanner(file);
      int j = 0;
      while (reader.hasNextLine()) {
        String line = reader.nextLine();
        String[] entries = line.split("[,]", 0);
        if (j >= 1) {
          solution.setVariable(i, new RealVariable(Float.parseFloat(entries[1]),Float.parseFloat(entries[2])));
          i++;
        }
        j++;
      }
      reader.close();
    } catch (FileNotFoundException e) {
      System.out.println("An error occurred.");
      e.printStackTrace();
    }

    /* corn parameter ranges */
    try {
      File file = new File("./Calibration/ParamRanges/CornRanges.csv");
      Scanner reader = new Scanner(file);
      int j = 0;
      while (reader.hasNextLine()) {
        String line = reader.nextLine();
        String[] entries = line.split("[,]", 0);
        if (j >= 1) {
          solution.setVariable(i, new RealVariable(Float.parseFloat(entries[1]),Float.parseFloat(entries[2])));
          i++;
        }
        j++;
      }
      reader.close();
    } catch (FileNotFoundException e) {
      System.out.println("An error occurred.");
      e.printStackTrace();
    }

    /* soybean parameter ranges */
    try {
      File file = new File("./Calibration/ParamRanges/SoybeanRanges.csv");
      Scanner reader = new Scanner(file);
      int j = 0;
      while (reader.hasNextLine()) {
        String line = reader.nextLine();
        String[] entries = line.split("[,]", 0);
        if (j >= 1) {
          solution.setVariable(i, new RealVariable(Float.parseFloat(entries[1]),Float.parseFloat(entries[2])));
          i++;
        }
        j++;
      }
      reader.close();
    } catch (FileNotFoundException e) {
      System.out.println("An error occurred.");
      e.printStackTrace();
    }

    return solution;
  }
}
