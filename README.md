# moea-calibrate

This repository contains Java and C programs that can calibrate RZWQM2 with
any number of multiobjective evolutionary algorithms available in MOEA Framework,
an open-source Java library for multiobjective optimization: http://moeaframework.org/

The repository comes with two primary folders, the Scenarios folder and the
MOEAFramework-2.13 folder.

The Scenarios folder contains the Meteorology folder with .BRK, .MET, .SNO files
that need to be updated for the study locations (sites) of interest and the
site-specific calibration warehouse folder SiteX_cal_wh, which contains the
minimum files needed to execute RZWQM2. You should modify the rzinit.dat
and rzwqm.dat files for your study locations. The warehouse name SiteX_cal_wh can
be changed as needed, where X is any letter, number, or combination thereof that is
used to denote a study site. Further changes to the name are possible, but would
require modifying the C code that uses the warehouse to generate run folders (makeFoldCal.c).

The MOEAFramework-2.13 folder contains source code and examples for running
algorithms in MOEAFramework provided by the developer in addition to the
Calibration folder which contains the Java classes and C executables for
calibrating RZWQM2.

Since RZWQM2 is not open source I opted to leave some required folders out of
the repository. Assuming that you have downloaded RZWQM2 to your C drive, move
the "bin" and "database" folders in the "C:\RZWQM2" folder into your copy of the
moea-calibrate repository. I did this so that all of the code executes within
one folder, which makes it more transferrable between desktops.  

After setting up the folders, follow the instructions in the QuickStart pdf
within the MOEAFramework-2.13 folder to install the Java Development
Kit (JDK). I recommend using Oracle and installing the latest version of Java.
You can also install Eclipse or Netbeans for writing Java programs, but given
that this repository is cross-platform, I would recommend a generic text editor.

After downloading Java, you should check that it has been added to your path.
Go into System Properties -> Environment Variables. Under system variables,
JAVA_HOME should be located at "C:\Program Files\Java\jdk-16.0.2", and Path
should contain "C:\Program Files\Java\jdk-16.0.2\bin".

It is very likely that you have to modify the C executables to customize
the calibration code to your study site. Hence, you need to install a C compiler.
I recommend MinGW-w64 for Windows:
https://sourceforge.net/p/mingw-w64/mailman/message/37287751/
Here is a short youtube tutorial for installation and setting the path:
https://www.youtube.com/watch?v=9PAglZlRolo

Once you have Java and a C compiler installed, you can start modifying the Java
classes to calibrate RZWQM2 with selected algorithms. First, I will go over the
folders and files in the MOEAFramework-2.13/Calibration folder.

1. CalProbs is a package containing Java classes that define the calibration
   problem, i.e. the number of objectives, parameters, and parameter ranges.
   I provide the example class calProbs1, which has 32 parameters and 4 objectives.
2. CalScripts contains the main Java classes for executing the calibration and
   collecting the results across MOEA runs. CalRun.java is the main class where
   you specify which MOEAs and the maximum number of generations. CalCollect.java
   is the class that collects and analyzes the calibration results for multiple
   MOEAs.
3. Observations contains csv files of observed data. As an example, I included
   tile flow volume, tile nitrate load, and crop yield data for two sites.
4. ParamRanges contains csv files for your chosen hydrologic, nutrient, corn,
   and soybean parameter ranges.
5. SiteX is the folder where all calibration results are stored. You should
   change the name to reflect your study site.
6. Utilies contains the SynchronizedMersenneTwister class, which generates
   random seeds in a thread-safe manner. This could potentially be omitted.
   To my knowledge, the developers have yet to provide a reproducible random
   number generator that is thread-safe and does not through errors.
7. calRun.c/.exe is the executable that updates the RZWQM2 parameters and
   computes the objectives, which are the relative/normalized root mean squared
   errors (RRMSEs/nRMSEs) of each model output.
8. makeFoldCal.c/.exe is a program that generates a new run folder from the
   warehouse. This enables you to run multiple calibration runs simultaneously
   with different MOEAs.
9. moeaframework.c/moeaframework.h/moeaframework.o are required source code files
   needed for calRun.exe to work.

As a first step, I would recommend formatting your observed data into csv files.
From here, modify calProbs1.java to reflect the number of parameters you are
calibrating and the number of objectives, which is equal to the number of
model output you are considering. This will also require updating the ParamRanges
csv files to only include those parameters. To recompile, run the command

javac -cp "lib/*;Calibration" Calibration/CalProbs/calProbs1.java

within the MOEAFramework-2.13 folder.

As a next step, update calRun.c by specifying the start/end dates of your
simulations and to properly read/write the hydrologic, nutrient cycling,
and crop cultivar parameters that you are calibrating. After making changes,
you can recompile it with the command

gcc -Wall -o calRun calRun.c moeaframework.o

From here, update CalRun.java in the CalScripts folder to specify the MOEAs and
maximum number of generations you want to implement. First, modify the string
proj_path to reflect the path to the calibration folder on your computer. Then,
in the run() function change the integer "nfe" to the maximum number of generations
that you chose. Finally, in the main() function, modify the String array algos
to include the MOEAs that you want to run (you can just include one).
Be careful not to run more MOEAs than the number of cores on your computer;
otherwise, you will see a drastic decrease in performance.

After making these changes, recompile the class by entering

javac -cp "lib/*;Calibration" Calibration/CalRun.java

within the MOEAFramework-2.13 folder on the command line.

Then, you can start the calibration runs with the command

java -cp "lib/*;Calibration" CalRun.java X

where X is the site number/letter combination.

If you chose to run many MOEAs, then you can pool the results over all MOEAs
and collect the best objectives & parameters with the Java class CalCollect.java.
This also saves performance metric results. In this case, all you need to do is
update the algorithms string and the nfe (max no. generatations) and then recompile.

For the full list of available algorithms, see
http://moeaframework.org/features.html

For more help on how to use MOEA Framework, see
https://docs.google.com/document/pub?id=1Ts_tnvzZ-nDQ-Ym-RFtqM_LJMUNYKFZJ5WJdZxRmmrY

If you use this code in a publication, please cite my thesis:

Peterson, C. M. (2021). Quantifying the agronomic and water quality tradeoffs
of fertilizer management with multiobjective evolutionary algorithms. (M.S.).
University of Illinois at Urbana-Champaign. Urbana, IL.

and cite MOEA Framework:

Hadka, D. (2015). MOEA Framework: A free and open source Java framework for
multiobjective optimization (Version 2.13) [Computer software].
http://www.moeaframework.org/
