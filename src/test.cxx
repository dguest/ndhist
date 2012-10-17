#include "Histogram.hh"
#include "Binners.hh"
#include "HdfFromHist.hh"

#include "H5Cpp.h"
#include <vector> 
#include <iostream>

int main(int narg, char* argv[]) { 
  LinBinner binner("x", 1, 0, 3); 
  binner.add_dimension(new LinBinner("y", 1, 0, 3)); 
  binner.add_dimension(new LinBinner("z", 1, 0, 1)); 

  std::map<std::string, double> corrd; 
  corrd["x"] = 1.3; 
  corrd["y"] = 4.5; 
  corrd["z"] = 0.3; 
  std::cout << binner.get_bin(corrd) << std::endl; 

  Dimension x; 
  x.name = "x"; 
  x.n_bins = 10; 
  x.low = 0; 
  x.high = 10; 

  Dimension y; 
  y.name = "y"; 
  y.n_bins = 10; 
  y.low = 0; 
  y.high = 10; 
  Dimension z; 
  z.name = "z"; 
  z.n_bins = 1; 
  z.low = 0; 
  z.high = 1; 

  std::vector<Dimension> dims; 
  dims.push_back(x); 
  dims.push_back(y); 
  dims.push_back(z); 

  Histogram hist(dims); 
  hist.fill(corrd);
  corrd["y"] -= 1; 
  hist.fill(corrd); 
  std::vector<double> vals = hist.get_values();
  int count = 0; 
  for (std::vector<double>::const_iterator itr = vals.begin(); 
       itr != vals.end(); itr++) {
    std::cout << *itr << " "; 
    count++; 
    if (count == 12) { 
      std::cout << std::endl;
      count = 0; 
    }
  }
  std::cout << std::endl; 

  H5::H5File file("test.h5", H5F_ACC_TRUNC); 
  histToFile(hist, file, "testhist"); 
		  
  return 0; 
}
