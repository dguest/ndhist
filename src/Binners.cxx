#include "Binners.hh"
#include <stdexcept>


LinBinner::LinBinner(std::string name, int n_bins, double low, double high): 
  m_name(name), 
  m_n_bins(n_bins), 
  m_low(low), 
  m_high(high), 
  m_subbinner(0)
{
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

  if (m_subbinner) { 
    int sub_bin = m_subbinner->get_bin(locator); 
    bin += sub_bin * (m_n_bins + 2); 
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