import java.io.*;
import java.util.*;
import org.moeaframework.Analyzer;
import CalProbs.calProb1;

/* Java class that pools RZWQM2 calibration results over many MOEAs */

public class CalCollect {
	public static void main(String[] args) {
		/* site name */
		String site = args[0];

		/* make string for site folder */
		String site_folder = "./Calibration/Site" + site + "/";

		/* dummy string for run folder */
		String run_folder = "Site" + site + "_cal";

		/* specify which algorithms were compared */
		String[] algos = {"eMOEA","GDE3"};

		/* number of function evaluations */
		Integer nfe = 500;

		/* setup analyzer */
		Analyzer analyzer = new Analyzer()
			.withProblemClass(calProb1.class, site, run_folder)
			.includeAllMetrics()
			.showAll();

		/* pool results over all MOEAs */
		for (String algo : algos) {
			// load results of each MOEA into analyzer
			try {
				File resultsFile = new File(site_folder + "Results/" + algo + "results" + nfe + ".txt");
				analyzer.loadAs(algo, resultsFile);
			} catch (IOException e) {
				System.out.println("An error occurred loading results");
				e.printStackTrace();
			}
		}

		// save reference set (best objectives over all MOEAs) without parameter values
		try {
			File refFile = new File(site_folder + "ReferenceSets/refSet" + nfe + ".txt");
			analyzer.saveReferenceSet(refFile);
			System.out.println("Successfully saved reference set objectives");
		} catch (IOException e) {
			System.out.println("An error occurred saving reference set objectives");
			e.printStackTrace();
		}

		// save reference set with parameter values
		try {
			File refVarFile = new File(site_folder + "ReferenceSets/refSetwPars" + nfe + ".txt");
			analyzer.saveAs(null, refVarFile);
			System.out.println("Successfully saved reference set with parameters");
		} catch (IOException e) {
			System.out.println("An error occurred saving reference set with parameters");
			e.printStackTrace();
		}

		// save performance metric results
		try {
			File analysisFile = new File(site_folder + "Analysis/analysis" + nfe + ".txt");
			analyzer.saveAnalysis(analysisFile);
			System.out.println("Successfully saved analysis results");
		} catch (IOException e) {
			System.out.println("An error occurred saving analysis results");
			e.printStackTrace();
		}

	}
}
