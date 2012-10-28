#include "Histogram.hh"
#include "Binners.hh"
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

  std::vector<double> v_corrd; 
  v_corrd.push_back(1.3); 
  v_corrd.push_back(4.5); 
  v_corrd.push_back(0.3); 

  Axis x; 
  x.name = "x"; 
  x.n_bins = 10; 
  x.low = 0; 
  x.high = 10; 

  Axis y; 
  y.name = "y"; 
  y.n_bins = 10; 
  y.low = 0; 
  y.high = 10; 
  Axis z; 
  z.name = "z"; 
  z.n_bins = 1; 
  z.low = 0; 
  z.high = 1; 

  std::vector<Axis> dims; 
  dims.push_back(x); 
  dims.push_back(y); 
  dims.push_back(z); 

  Histogram hist(dims); 
  Histogram copy_hist(hist); 
  hist = copy_hist; 
  hist.fill(corrd);
  corrd["y"] -= 1; 
  hist.fill(corrd); 
  hist.fill(v_corrd); 

  H5::H5File file("test.h5", H5F_ACC_TRUNC); 
  hist.write_to(file, "testhist"); 
		  
  return 0; 
}
