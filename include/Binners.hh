#ifndef BINNERS_H
#define BINNERS_H

#include <string> 
#include <map> 

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

#endif //BINNERS_H
