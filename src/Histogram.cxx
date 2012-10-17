#include "Histogram.hh"
#include "Binners.hh"
#include <stdexcept>

Histogram::Histogram(const std::vector<Dimension>& dims) : 
  m_dimsensions(dims)
{
  if (dims.size() == 0) {
    throw std::runtime_error("tried to initialize hist with no dimensions");
  }
  typedef std::vector<Dimension> DimVec;
  DimVec::const_iterator itr = dims.begin(); 
  int n_values = itr->n_bins + 2; 
  m_binner = new LinBinner(itr->name, itr->n_bins, itr->low, itr->high);
  itr++; 
  for (; itr != dims.end(); itr++) { 
    m_binner->add_dimension(new LinBinner(itr->name, itr->n_bins, 
					 itr->low, itr->high)); 
    n_values *= (itr->n_bins + 2); 
  }
  m_values = std::vector<double>(n_values, 0); 
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

std::vector<Dimension> Histogram::get_dims() const { 
  return m_dimsensions; 
}
std::vector<double> Histogram::get_values() const { 
  return m_values; 
}

