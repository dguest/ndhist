#include "Histogram.hh"
#include "H5Cpp.h"
#include "Binners.hh"
#include <stdexcept>
#include <set>
#include <algorithm>
#include <cassert> 

Histogram::Histogram(int n_bins, double low, double high, std::string units, 
		     unsigned flags): 
  Histogram({{"x",n_bins,low,high,units}}, flags)
{ 
  assert(m_dimsensions.size() == 1); 
}

Histogram::Histogram(const std::initializer_list<Axis>& dims, 
		     unsigned flags): 
  Histogram(std::vector<Axis>(dims), flags) 
{
}

Histogram::Histogram(const std::vector<Axis>& dims, unsigned flags) 
{ 
  m_wt2 = 0; 
  m_wt2_ext = "Wt2"; 
  m_eat_nan = flags & hist::eat_nan; 
  m_n_nan = 0; 
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
  if (flags & hist::wt2) { 
    m_wt2 = new std::vector<double>(n_values, 0); 
  }
}

Histogram::Histogram(const Histogram& old): 
  m_binner(0), 
  m_dimsensions(old.m_dimsensions), 
  m_values(old.m_values), 
  m_chunking(old.m_chunking), 
  m_n_nan(old.m_n_nan), 
  m_eat_nan(old.m_eat_nan), 
  m_wt2(0)
{ 
  assert(old.m_binner); 
  m_binner = old.m_binner->clone(); 
  if (old.m_wt2) { 
    *m_wt2 = *old.m_wt2; 
  }
}

Histogram& Histogram::operator=(Histogram old)
{ 
  using std::swap; 
  swap(*this, old);
  return *this; 
}

Histogram::~Histogram() 
{
  delete m_binner; 
  delete m_wt2; 
}

void swap(Histogram& f, Histogram& s) 
{
  using std::swap; 
  swap(f.m_binner,      s.m_binner); 
  swap(f.m_dimsensions, s.m_dimsensions); 
  swap(f.m_values,      s.m_values); 
  swap(f.m_chunking,    s.m_chunking); 
  swap(f.m_n_nan,       s.m_n_nan); 
  swap(f.m_eat_nan,     s.m_eat_nan); 
  swap(f.m_wt2,         s.m_wt2); 
  swap(f.m_wt2_ext,     s.m_wt2_ext); 
}

void Histogram::fill(const std::map<std::string, double>& input, 
		     double weight) { 
  safe_fill(input, weight); 
}

void Histogram::fill(const std::vector<double>& input, 
		     double weight) { 
  safe_fill(input, weight); 
}
void Histogram::fill(const std::initializer_list<double>& input, 
		     double weight) { 
  safe_fill(std::vector<double>(input), weight); 
}

void Histogram::fill(double value, double weight) { 
  assert(m_dimsensions.size() == 1); 
  std::vector<double> v(1,value); 
  safe_fill(v, weight); 
}

void Histogram::set_wt_ext(const std::string& ext) { 
  if (ext.size() == 0) { 
    throw std::invalid_argument(
      "tried to use no wt2 extension: this will overwrite the normal hist");
  }
  m_wt2_ext = ext; 
}

void Histogram::write_to(H5::CommonFG& file, 
			 const std::string& name, int deflate) 
  const 
{
  write_internal(file, name, deflate, m_values); 
  if (m_wt2) { 
    write_internal(file, name + m_wt2_ext, deflate, *m_wt2); 
  }
}

// ==================== private ==========================

namespace { 
  // attribute adding function 
  template<typename M> 
  void write_attr(H5::DataSet&, const std::string& name, M* val); 
  template<typename M>
  void write_attr_vec(H5::DataSet&, const std::string& name, M vec); 

  // various overloads to use in template
  H5::PredType get_type(double val); 
  H5::PredType get_type(int val); 
  H5::PredType get_type(unsigned val); 
  H5::StrType get_type(const std::string& val); 
}

void Histogram::write_internal(
  H5::CommonFG& file, const std::string& name, int deflate, 
  const std::vector<double>& values) const
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
  params.setChunk(n_dims, ds_chunks.data());
  params.setDeflate(deflate); 
  H5::DataSpace data_space(n_dims, ds_dims.data()); 
  H5::DataSet dataset = file.createDataSet(name, PredType::NATIVE_DOUBLE, 
					   data_space, params); 
  assert(values.size() == total_entries); 
  dataset.write(&values[0], PredType::NATIVE_DOUBLE); 

  for (unsigned dim = 0; dim < n_dims; dim++) { 
    const Axis& dim_info = m_dimsensions.at(dim); 
    dim_atr(dataset, dim, dim_info); 
  }
  write_attr(dataset, "nan", &m_n_nan); 
  
}


template<typename T> 
void Histogram::safe_fill(T input, double weight) {
  try { 
    int bin = m_binner->get_bin(input); 
    m_values.at(bin) += weight; 
    if (m_wt2) { 
      m_wt2->at(bin) += weight*weight; 
    }
  }
  catch (std::range_error& r) { 
    if (m_eat_nan) { 
      m_n_nan++; 
    }
    else { 
      throw; 
    }
  }
}

void Histogram::dim_atr(H5::DataSet& target, unsigned number, 
			const Axis& dim) const
{
  using namespace H5;

  write_attr(target, dim.name + "_axis", &number); 
  write_attr(target, dim.name + "_bins", &dim.n_bins); 
  write_attr(target, dim.name + "_max", &dim.high); 
  write_attr(target, dim.name + "_min", &dim.low); 
  write_attr(target, dim.name + "_units", &dim.units); 
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

namespace { 
  template<typename M> 
  void write_attr(H5::DataSet& loc, const std::string& name, M* value) { 
    auto type = get_type(*value); 
    loc.createAttribute(name, type, H5S_SCALAR).write(type, value); 
  }
  // template<typename M> 
  // void write_attr_vec(H5::DataSet& loc, const std::string& name, M vec) { 
  //   auto type = get_type(*vec); 
  //   loc.createAttribute(name, type, H5S_SCALAR).write(type, value); 
  // }

  H5::PredType get_type(int) { 
    return H5::PredType::NATIVE_INT; 
  }
  H5::PredType get_type(unsigned) { 
    return H5::PredType::NATIVE_UINT; 
  }
  H5::PredType get_type(double) { 
    return H5::PredType::NATIVE_DOUBLE; 
  }
  H5::StrType get_type(const std::string&) { 
    return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  }

}
