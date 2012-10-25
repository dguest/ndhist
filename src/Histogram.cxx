#include "Histogram.hh"
#include "Binners.hh"
#include <stdexcept>
#include <algorithm>
#include <cassert> 

Histogram::Histogram(int n_bins, double low, double high) 
{ 
  Axis a = {"x",n_bins,low,high};
  init(std::vector<Axis>(1,a)); 
  assert(get_axes().size() == 1); 
}

Histogram::Histogram(const std::vector<Axis>& dims)
{
  init(dims); 
}
void Histogram::init(const std::vector<Axis>& dims) 
{ 
  m_dimsensions = dims; 

  if (dims.size() == 0) {
    throw std::runtime_error("tried to initialize hist with no dimensions");
  }
  typedef std::vector<Axis> Axes;
  Axes::const_reverse_iterator itr = dims.rbegin(); 
  int n_values = itr->n_bins + 2; 
  m_binner = new LinBinner(itr->name, itr->n_bins, itr->low, itr->high);
  itr++; 
  for (; itr != dims.rend(); itr++) { 
    m_binner->add_dimension(new LinBinner(itr->name, itr->n_bins, 
					 itr->low, itr->high)); 
    n_values *= (itr->n_bins + 2); 
  }
  m_values = std::vector<double>(n_values, 0); 
}

Histogram::Histogram(const Histogram& old): 
  m_binner(old.m_binner->clone()), 
  m_dimsensions(old.m_dimsensions), 
  m_values(old.m_values)
{ 
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
  std::vector<double> cp(input); 
  fill(cp); 
}

void Histogram::fill(std::vector<double>& input, 
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

std::vector<Axis> Histogram::get_axes() const { 
  return m_dimsensions; 
}
std::vector<double> Histogram::get_values() const { 
  return m_values; 
}

