#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <string> 
#include <map>

class IBinner; 

struct Dimension
{
  std::string name;
  int n_bins; 
  double low; 
  double high; 
};

class Histogram
{
public: 
  Histogram(const std::vector<Dimension>&); 
  ~Histogram(); 
  void fill(const std::map<std::string, double>, double weight = 1); 
  std::vector<Dimension> get_dims() const; 
  std::vector<double> get_values() const; 
private: 
  std::vector<Dimension> m_dimsensions; 
  IBinner* m_binner; 
  std::vector<double> m_values; 
}; 

#endif //HISTOGRAM_H
