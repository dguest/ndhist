#include "Histogram.hh"
#include "H5Cpp.h"
#include "Binners.hh"
#include <stdexcept>
#include <set>
#include <algorithm>
#include <cassert> 

Histogram::Histogram(int n_bins, double low, double high, std::string units) 
{ 
  Axis a = {"x",n_bins,low,high,units};
  init(std::vector<Axis>(1,a)); 
  assert(m_dimsensions.size() == 1); 
}

Histogram::Histogram(const std::vector<Axis>& dims)
{
  init(dims); 
}
void Histogram::init(const std::vector<Axis>& dims) 
{ 
  m_dimsensions = dims; 
  check_dimensions(dims); 
  assert(dims.size() > 0); 
  Axes::const_reverse_iterator itr = dims.rbegin(); 
  int n_values = itr->n_bins + 2; 
  m_binner = new LinBinner(itr->name, itr->n_bins, itr->low, itr->high);
  itr++; 
  for (; itr != dims.rend(); itr++) { 
    m_binner->add_dimension(new LinBinner(itr->name, itr->n_bins, 
					 itr->low, itr->high)); 
    n_values *= (itr->n_bins + 2); 
  }
  assert(m_binner); 
  m_values = std::vector<double>(n_values, 0); 
}

Histogram::Histogram(const Histogram& old): 
  m_binner(0), 
  m_dimsensions(old.m_dimsensions), 
  m_values(old.m_values)
{ 
  assert(old.m_binner); 
  m_binner = old.m_binner->clone(); 
}

Histogram& Histogram::operator=(const Histogram& old)
{ 
  delete m_binner; 
  m_binner = old.m_binner->clone(); 
  m_dimsensions = old.m_dimsensions; 
  m_values = old.m_values; 
  return *this; 
}

Histogram::~Histogram() 
{
  delete m_binner; 
}

void Histogram::fill(const std::map<std::string, double> input, 
		     double weight) { 
  int bin = m_binner->get_bin(input); 
  m_values.at(bin) += weight; 
}

void Histogram::fill(const std::vector<double>& input, 
		     double weight) { 
  int bin = m_binner->get_bin(input); 
  m_values.at(bin) += weight; 
}

void Histogram::fill(double value, double weight) { 
  assert(m_dimsensions.size() == 1); 
  std::vector<double> v(1,value); 
  int bin = m_binner->get_bin(v);
  m_values.at(bin) += weight; 
}

void Histogram::write_to(H5::CommonFG& file, std::string name, int deflate) 
  const 
{
  using namespace H5; 
  const hsize_t n_dims = m_dimsensions.size(); 
  std::vector<hsize_t> ds_dims(n_dims); 
  std::vector<hsize_t> ds_chunks(n_dims); 
  hsize_t total_entries = 1;
  for (unsigned dim = 0; dim < n_dims; dim++) { 
    // 2 extra for overflow bins
    hsize_t bins = m_dimsensions.at(dim).n_bins + 2; 	
    ds_dims.at(dim) = bins; 
    ds_chunks.at(dim) = get_chunk_size(bins); // for now just returns value
    total_entries *= bins; 
  }
  H5::DSetCreatPropList params; 
  params.setChunk(n_dims, &ds_chunks[0]);
  params.setDeflate(deflate); 
  H5::DataSpace data_space(n_dims, &ds_dims[0]); 
  H5::DataSet dataset = file.createDataSet(name, PredType::NATIVE_DOUBLE, 
					   data_space, params); 
  assert(m_values.size() == total_entries); 
  dataset.write(&m_values[0], PredType::NATIVE_DOUBLE); 

  for (unsigned dim = 0; dim < n_dims; dim++) { 
    const Axis& dim_info = m_dimsensions.at(dim); 
    dim_atr(dataset, dim, dim_info); 
  }
}


void Histogram::dim_atr(H5::DataSet& target, unsigned number, 
			const Axis& dim) const
{
  using namespace H5;
  DataSpace space(H5S_SCALAR);
  IntType int_type(PredType::NATIVE_INT);
  IntType uint_type(PredType::NATIVE_UINT); 
  StrType str_type(PredType::C_S1, H5T_VARIABLE);

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
  min.write(f_type, &dim.low); 

  std::string unit_name = dim.name + "_units"; 
  Attribute units = target.createAttribute(unit_name, str_type, space); 
  units.write(str_type, &dim.units); 
}

int Histogram::get_chunk_size(int input) const { 
  return input; 
}

void Histogram::check_dimensions(const std::vector<Axis>& axes) { 
  if (axes.size() == 0) {
    throw std::logic_error("Histogram: tried to initialize with no"
			   " dimensions");
  }
  std::set<std::string> names; 
  for (Axes::const_iterator itr = axes.begin(); itr != axes.end(); itr++) { 
    if (itr->name.size() == 0) { 
      throw std::logic_error("Histogram: unnamed axis"); 
    }
    if (names.count(itr->name)) { 
      throw std::logic_error("Histogram: axis name " + itr->name + 
			     " was used twice"); 
    }
    if (itr->low > itr->high) { 
      throw std::logic_error("Histogram: axis " + itr->name + " has high "
			     "bound below low bound"); 
    }
    
  }
}
