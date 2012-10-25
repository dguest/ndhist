#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <string> 
#include <map>

class IBinner; 

struct Axis
{
  std::string name;
  int n_bins; 
  double low; 
  double high; 
};

class Histogram
{
public: 
  Histogram(const std::vector<Axis>&); 
  ~Histogram(); 
  void fill(const std::map<std::string, double>, double weight = 1); 
  void fill(const std::vector<double>&, double weight = 1); 
  void fill(std::vector<double>&, double weight = 1); 
  std::vector<Axis> get_axes() const; 
  std::vector<double> get_values() const; 
private: 
  std::vector<Axis> m_dimsensions; 
  IBinner* m_binner; 
  std::vector<double> m_values; 
}; 

#endif //HISTOGRAM_H
