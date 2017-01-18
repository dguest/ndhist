#ifndef NDHIST_EXCEPTIONS
#define NDHIST_EXCEPTIONS

#include <stdexcept>
#include <string>

// exceptions
class HistogramSaveError: public std::runtime_error {
public:
  HistogramSaveError(const std::string&);
};

class HistogramBinningError: public std::logic_error {
public:
  HistogramBinningError(const std::string&);
};

class HistogramNanError: public HistogramBinningError {
public:
  HistogramNanError(const std::string&);
};

#endif
