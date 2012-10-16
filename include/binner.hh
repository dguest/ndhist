#ifndef BINNER_H
#define BINNER_H

#include <string> 
#include <map> 
#include <vector> 

class IBinner
{
public: 
  virtual ~IBinner() {}; 
  virtual int get_bin(const std::map<std::string, double>&) const = 0; 
  virtual void add_dimension(IBinner*) = 0; 
}; 

class LinBinner: public IBinner 
{
public: 
  LinBinner(std::string name, int n_bins, double low, double high); 
  ~LinBinner(); 
  int get_bin(const std::map<std::string, double>& locator) const; 
  void add_dimension(IBinner* sub_binner); 
private: 
  std::string m_name; 
  int m_n_bins; 
  double m_low; 
  double m_high; 
  IBinner* m_subbinner; 
};

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

#endif //BINNER_H
