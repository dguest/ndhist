#include "HdfFromHist.hh"
#include "Histogram.hh"
#include "H5Cpp.h"

#include <cassert> 

int histToFile(const Histogram& hist, H5::CommonFG& file, std::string name){

  using namespace H5; 

  std::vector<Dimension> dims = hist.get_dims(); 

  const hsize_t n_dims = dims.size(); 
  hsize_t ds_dims[n_dims]; 
  hsize_t total_entries = 1;
  for (unsigned dim = 0; dim < dims.size(); dim++) { 
    hsize_t bins = dims.at(dim).n_bins + 2; 	// extra for overflow bins
    ds_dims[dim] = bins; 
    total_entries *= bins; 
  }

  H5::DataSpace data_space(n_dims, ds_dims); 
  H5::DataSet dataset = file.createDataSet(name, PredType::NATIVE_DOUBLE, 
					   data_space); 
  std::vector<double> data = hist.get_values(); 
  assert(data.size() == total_entries); 
  dataset.write(&data[0], PredType::NATIVE_DOUBLE); 

  for (unsigned dim = 0; dim < n_dims; dim++) { 
    Dimension& dim_info = dims.at(dim); 
    dim_atr(dataset, dim, dim_info); 
  }
  return 1; 
}


void dim_atr(H5::DataSet& target, unsigned number, const Dimension& dim)
{
  using namespace H5;
  DataSpace space(H5S_SCALAR);
  IntType int_type(PredType::NATIVE_INT);
  IntType uint_type(PredType::NATIVE_UINT); 
  std::string axis_name = dim.name + "_axis"; 
  Attribute axis = target.createAttribute(axis_name, uint_type, space);
  axis.write(uint_type, &number);

  std::string n_bin_name = dim.name + "_bins"; 
  Attribute n_bin = target.createAttribute(n_bin_name, int_type, space); 
  n_bin.write(int_type, &dim.n_bins); 
  FloatType f_type(PredType::NATIVE_DOUBLE); 
  Attribute max = target.createAttribute(dim.name + "_max", f_type, space); 
  max.write(f_type, &dim.high); 
  Attribute min = target.createAttribute(dim.name + "_min", f_type, space); 
  max.write(f_type, &dim.low); 
  
}
