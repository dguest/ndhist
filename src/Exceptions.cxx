#include "Exceptions.hh"

//______________________________________________________________________
// exception definitions

HistogramSaveError::HistogramSaveError(const std::string& what):
  std::runtime_error(what)
{}

HistogramBinningError::HistogramBinningError(const std::string& what):
  std::logic_error(what)
{}

HistogramNanError::HistogramNanError(const std::string& what):
  HistogramBinningError(what)
{}
