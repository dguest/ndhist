#ifndef HDF_FROM_HIST_H
#define HDF_FROM_HIST_H

class Histogram; 
class Axis; 
namespace H5 { 
  class CommonFG; 
  class DataSet; 
} 

#include <string> 

int histToFile(const Histogram& hist, H5::CommonFG& file, std::string name); 
void dim_atr(H5::DataSet& target, unsigned number, const Axis& dim); 


#endif 
