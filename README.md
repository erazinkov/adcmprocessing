# adcmprocessing

Console Application for spectrum analysis (Tagged Neutron Method).

## Technologies used:
  * C++17, ROOT 6.32.10.
   > [!IMPORTANT]
  > The correctness of the application's results depends on the experimental setup.
  
   
  
## Features:
  ### Input
  * Binary files from JINR AFI Electronics ADCM family (ADCM-16, ADCM-32).
  ### Output
  * Saves total energy gamma spectrum to ROOT file (.root).
  * Saves the amplitude spectrum with energy calibration (.ps).
  * Saves the Time-of-Flight (TOF) spectrum (.ps).
   > [!NOTE]
  > Output directory **results/**.
  
## Usage
  * adcmprocessing \<path-to-data-file\>
 
