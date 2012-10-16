#include "binner.hh"
#include <iostream> 
#include <stdexcept>

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
		  
  return 0; 
}


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

