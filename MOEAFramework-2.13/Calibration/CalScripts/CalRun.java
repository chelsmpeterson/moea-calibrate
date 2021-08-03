import java.io.*;
import java.util.*;
import org.moeaframework.Analyzer;
import org.moeaframework.Executor;
import org.moeaframework.core.PRNG;
import Utilities.SynchronizedMersenneTwister;
import CalProbs.calProb1;

/* Java class for running RZWQM2 calibration with many algorithms in parallel */

public class CalRun implements Runnable {
	String site, algo, site_folder, run_folder;
	static String proj_path = "C:/Users/cmptrsn2/moea-calibrate";

	/* class that defines calibration run instance */
	public CalRun(String site, String algo) throws IOException {
		this.site = site;
    this.algo = algo;
		this.site_folder = "./Calibration/Site" + site + "/";
		this.run_folder = "Site" + site + "_cal_" + algo;
	}

	/* function that makes run folder for a given site */
  public static void makeRunFolder(String site, String run_folder) {
		String command = "./Calibration/makeFoldCal " + site + " " + run_folder;
		try {
			Process process = Runtime.getRuntime().exec(command);
			BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
			String line;
			while ((line = errorReader.readLine()) != null) {
					System.out.println(line);
			}
			errorReader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/* function that deletes folder/directory */
	public static boolean deleteDirectory(File dir) {
		if (dir.isDirectory()) {
			File[] children = dir.listFiles();
			for (int i = 0; i < children.length; i++) {
				boolean success = deleteDirectory(children[i]);
				if (!success) {
					return false;
				}
			}
		}
		return dir.delete();
	}

	/* main code block that is implemented for each thread */
  public void run() {
		// make run folder
		makeRunFolder(this.site, this.run_folder);

		// check if the executable exists
		File file = new File("./Calibration/calRun.exe");
		if (!file.exists()) {
			System.err.println("Please compile the calRun executable");
			return;
		}

		// number of function evaluations, i.e. maximum number of generations
		int nfe = 500;

		// setup executor
		Executor executor = new Executor()
				.withProblemClass(calProb1.class, this.site, this.run_folder)
				.withMaxEvaluations(nfe)
				.withCheckpointFrequency(10)
				.distributeOnAllCores();

		// set up analyzer
		Analyzer analyzer = new Analyzer()
				.withSameProblemAs(executor)
				.includeAllMetrics()
				.showAll();

    // set random seed
		PRNG.setRandom(SynchronizedMersenneTwister.getInstance());

		// make checkpoint file to save intermediate results
		File checkPoint = new File(this.site_folder + "Checkpoints/" + this.algo + "_cal_" + nfe + ".state");

		// run analyzer
		analyzer.add(this.algo, executor.withAlgorithm(this.algo).withCheckpointFile(checkPoint).run());

		// save results
		try {
			File resultsFile = new File(site_folder + "Results/");
			analyzer.saveData(resultsFile, "", "results" + nfe + ".txt");
			System.out.println("Successfully saved all results after " + nfe + " generations");
		} catch (IOException e) {
			System.out.println("An error occurred saving results");
			e.printStackTrace();
		}

		// delete run folder
		deleteDirectory(new File(proj_path + "/Scenarios/" + this.run_folder));
	}

	/* main routine that starts each thread */
	public static void main(String[] args) throws IOException {
		// site name
		String site = args[0];

		// specify which algorithms to compare
		String[] algos = {"eMOEA","GDE3"};

		// start thread for each algorithm
		for (String algo : algos) {
			CalRun run = new CalRun(site, algo);
			Thread t = new Thread(run);
      t.start();
		}
	}
}
