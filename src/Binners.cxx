#include "Binners.hh"
#include <stdexcept>
#include <algorithm>
#include <cmath>

LinBinner::LinBinner(std::string name, int n_bins, double low, double high): 
  m_name(name), 
  m_n_bins(n_bins), 
  m_low(low), 
  m_high(high), 
  m_subbinner(0)
{
}

LinBinner::LinBinner(const LinBinner& o): 
  m_name(o.m_name), 
  m_n_bins(o.m_n_bins), 
  m_low(o.m_low), 
  m_high(o.m_high), 
  m_subbinner(0)
{ 
  if (o.m_subbinner) { 
    m_subbinner = o.m_subbinner->clone(); 
  }
}

LinBinner& LinBinner::operator=(LinBinner o)
{ 
  using std::swap; 
  swap(m_subbinner, o.m_subbinner); 
  swap(m_name, o.m_name); 
  swap(m_n_bins, o.m_n_bins); 
  swap(m_low, o.m_low); 
  swap(m_high, o.m_high); 
  return *this; 
}

LinBinner* LinBinner::clone() const 
{
  return new LinBinner(*this); 
}

LinBinner::~LinBinner() 
{
  delete m_subbinner; 
}

int LinBinner::get_bin(const std::map<std::string, double>& locator) const 
{
  typedef std::map<std::string, double> DMap; 
  DMap::const_iterator bin_itr = locator.find(m_name); 
  if (bin_itr == locator.end()) { 
    throw std::runtime_error("could not find " + m_name + " in values given"); 
  }
  double value = bin_itr->second; 
  int bin = get_bin(value); 
  if (m_subbinner) { 
    int sub_bin = m_subbinner->get_bin(locator); 
    bin += sub_bin * (m_n_bins + 2); 
  }

  return bin; 
}

int LinBinner::get_bin(const std::vector<double>& locator, size_t offset) 
  const 
{
  size_t index = locator.size() - offset - 1; 
  if (index < 0) { 
    throw std::runtime_error("could not find " + m_name + " in values given"); 
  }
  double value = locator.at(index); 
  int bin = get_bin(value); 
  if (m_subbinner) { 
    int sub_bin = m_subbinner->get_bin(locator, offset + 1); 
    bin += sub_bin * (m_n_bins + 2); 
  }
  return bin; 
}


int LinBinner::get_bin(double value) const 
{
  if (std::isnan(value)) { 
    throw std::range_error("binner was passed NaN"); 
  }
  int bin = 0; 
  if (value < m_low) { 
    bin = 0; 
  }
  else if (value >= m_high) { 
    bin = m_n_bins + 1; 
  }
  else { 
    double range = m_high - m_low; 
    double frac_range = (value - m_low) / range; 
    bin = int(frac_range * m_n_bins) + 1; 
  }
  return bin; 
}

void LinBinner::add_dimension(IBinner* sub_binner)
{
  if (m_subbinner) { 
    m_subbinner->add_dimension(sub_binner); 
  }
  else { 
    m_subbinner = sub_binner; 
  }
}
