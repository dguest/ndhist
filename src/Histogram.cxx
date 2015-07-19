#include "Histogram.hh"
#include "Binners.hh"
#include "H5Cpp.h"

#include <stdexcept>
#include <set>
#include <algorithm>
#include <cassert>

namespace {
  // internal check used in setup (will throw invalid_argument)
  void check_dimensions(const std::vector<Axis>& axes);
}

//______________________________________________________________________
// constructors / destructors / copy / swap

Histogram::Histogram(int n_bins, double low, double high, std::string units,
		     unsigned flags):
  Histogram({{"x",n_bins,low,high,units}}, flags)
{
  assert(m_dimsensions.size() == 1);
}

Histogram::Histogram(const std::initializer_list<Axis>& dims,
		     unsigned flags):
  Histogram(std::vector<Axis>(dims), flags)
{
}

Histogram::Histogram(const std::vector<Axis>& dims, unsigned flags) :
  m_binner(0),
  m_dimsensions(dims),
  m_n_nan(0),
  m_eat_nan(flags & hist::eat_nan),
  m_wt2(0),
  m_wt2_ext("Wt2")
{
  check_dimensions(dims);
  assert(dims.size() > 0);
  Axes::const_reverse_iterator itr = dims.rbegin();
  int n_values = itr->n_bins + 2;
  m_binner = new LinBinner(itr->name, itr->n_bins, itr->low, itr->high);
  itr++;
  for (; itr != dims.rend(); itr++) {
    m_binner->add_dimension(new LinBinner(itr->name, itr->n_bins,
					 itr->low, itr->high));
    n_values *= (itr->n_bins + 2);
  }
  assert(m_binner);
  m_values = std::vector<double>(n_values, 0);
  if (flags & hist::wt2) {
    m_wt2 = new std::vector<double>(n_values, 0);
  }
}

Histogram::Histogram(const Histogram& old):
  m_binner(0),
  m_dimsensions(old.m_dimsensions),
  m_values(old.m_values),
  m_chunking(old.m_chunking),
  m_n_nan(old.m_n_nan),
  m_eat_nan(old.m_eat_nan),
  m_wt2(0),
  m_wt2_ext(old.m_wt2_ext)
{
  assert(old.m_binner);
  m_binner = old.m_binner->clone();
  if (old.m_wt2) {
    *m_wt2 = *old.m_wt2;
  }
}

Histogram::Histogram(Histogram&& old):
  m_binner(old.m_binner),
  m_dimsensions(std::move(old.m_dimsensions)),
  m_values(std::move(old.m_values)),
  m_chunking(std::move(old.m_chunking)),
  m_n_nan(old.m_n_nan),
  m_eat_nan(old.m_eat_nan),
  m_wt2(old.m_wt2),
  m_wt2_ext(std::move(old.m_wt2_ext))
{
  // take ownership of the pointers
  old.m_binner = 0;
  old.m_wt2 = 0;
}

Histogram& Histogram::operator=(Histogram old)
{
  std::swap(*this, old);
  return *this;
}

Histogram::~Histogram()
{
  delete m_binner;
  delete m_wt2;
}

//______________________________________________________________________
// fill methods

void Histogram::fill(const std::map<std::string, double>& input,
		     double weight) {
  safe_fill(input, weight);
}

void Histogram::fill(const std::vector<double>& input,
		     double weight) {
  safe_fill(input, weight);
}
void Histogram::fill(const std::initializer_list<double>& input,
		     double weight) {
  safe_fill(std::vector<double>(input), weight);
}

void Histogram::fill(double value, double weight) {
  assert(m_dimsensions.size() == 1);
  std::vector<double> v(1,value);
  safe_fill(v, weight);
}

//______________________________________________________________________
// file IO related

void Histogram::set_wt_ext(const std::string& ext) {
  if (ext.size() == 0) {
    throw std::invalid_argument(
      "tried to use no wt2 extension: this will overwrite the normal hist");
  }
  m_wt2_ext = ext;
}

void Histogram::write_to(H5::CommonFG& file,
			 const std::string& name, int deflate)
  const
{
  write_internal(file, name, deflate, m_values);
  if (m_wt2) {
    write_internal(file, name + m_wt2_ext, deflate, *m_wt2);
  }
}

// ==================== private ==========================

// ____________________________________________________________________
// attribute adding forward declarations

namespace {
  // store attributes as arrays (indexed by axis number)
  void add_axis_attributes(H5::DataSet&, const std::vector<Axis>& axes);

  // utility function to save any other information
  template<typename M>
  void write_attr(H5::DataSet&, const std::string& name, M val);

  // type getter used in the template (adding other types should be trivial)
  H5::PredType get_type(int val);
}

// ______________________________________________________________________
// various Histogram internal methods

// write method called by the public Histogram write methods
void Histogram::write_internal(
  H5::CommonFG& file, const std::string& name, int deflate,
  const std::vector<double>& values) const
{
  if (H5Lexists(file.getLocId(), name.c_str(), H5P_DEFAULT)) {
    throw HistogramSaveError("tried to overwrite '" + name + "'");
  }
  using namespace H5;

  // define the DataSpace
  const hsize_t n_dims = m_dimsensions.size();
  std::vector<hsize_t> ds_dims(n_dims);
  std::vector<hsize_t> ds_chunks(n_dims);
  hsize_t total_entries = 1;
  for (unsigned dim = 0; dim < n_dims; dim++) {
    // 2 extra for overflow bins
    hsize_t bins = m_dimsensions.at(dim).n_bins + 2;
    ds_dims.at(dim) = bins;
    // datasets can be "chucked", i.e. stored and retrieved as smaller
    // pieces. Probably not needed for HEP histograms.
    ds_chunks.at(dim) = get_chunk_size(bins); // for now just returns value
    total_entries *= bins;
  }
  H5::DataSpace data_space(n_dims, ds_dims.data());

  // write the file
  H5::DSetCreatPropList params;
  params.setChunk(n_dims, ds_chunks.data());
  params.setDeflate(deflate);
  H5::DataSet dataset = file.createDataSet(
    name, PredType::NATIVE_DOUBLE, data_space, params);
  assert(values.size() == total_entries);
  dataset.write(values.data(), PredType::NATIVE_DOUBLE);
  // add some attributes
  add_axis_attributes(dataset, m_dimsensions);
  write_attr(dataset, "nan", m_n_nan);
}

// Internal wrapper on fill method. Takes care of NaN inputs, and
// filling the weight**2 hist (if it exists)
template<typename T>
void Histogram::safe_fill(T input, double weight) {
  try {
    int bin = m_binner->get_bin(input);
    m_values.at(bin) += weight;
    if (m_wt2) {
      m_wt2->at(bin) += weight*weight;
    }
  }
  catch (std::range_error& r) {
    if (m_eat_nan) {
      m_n_nan++;
    }
    else {
      throw;
    }
  }
}

// internal chunking function (may do more elaborate chunking someday)
int Histogram::get_chunk_size(int input) const {
  return input;
}

namespace {
  // throw exceptions if the constructor doesn't make sense.
  void check_dimensions(const std::vector<Axis>& axes) {
    if (axes.size() == 0) {
      throw std::invalid_argument(
	"Histogram: tried to initialize with no dimensions");
    }
    std::set<std::string> names;
    for (size_t pos = 0; pos < axes.size(); pos++) {
      auto& axis = axes.at(pos);
      auto& name = axis.name;
      if (name.size() == 0) {
	throw std::invalid_argument(
	  "Histogram: unnamed axis " + std::to_string(pos));
      }
      if (names.count(name)) {
	throw std::invalid_argument(
	  "Histogram: axis name " + name + " was used twice");
      } else {
	names.insert(name);
      }
      if (axis.low >= axis.high) {
	throw std::invalid_argument(
	  "Histogram: axis " + name + " has bounds with high <= low");
      }
      if (axis.n_bins < 1) {
	throw std::invalid_argument(
	  "Histogram: axis " + name + " has < 1 bin");
      }
    }
  }

  //________________________________________________________________________
  // implementation of the attribute adder functions

  // axis structure which is safe to store in HDF5
  struct H5Axis {
    const char* name;
    int n_bins;
    double min;
    double max;
    const char* units;
  };

  // much less ugly function to add axis attributes as arrays.
  void add_axis_attributes(H5::DataSet& targ, const std::vector<Axis>& axes)
  {
    // build the axis objects
    std::vector<H5Axis> h5_axes;
    for (const auto& ax: axes){
      H5Axis hax;
      hax.name = ax.name.c_str();
      hax.n_bins = ax.n_bins;
      hax.min = ax.low;
      hax.max = ax.high;
      hax.units = ax.units.c_str();
      h5_axes.push_back(hax);
    }
    // setup the dataspace
    hsize_t nax = h5_axes.size();
    hsize_t dim[] = {nax};
    H5::DataSpace space(1, dim);

    // setup data type
    auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
    stype.setCset(H5T_CSET_UTF8);
    auto dtype = H5::PredType::NATIVE_DOUBLE;
    auto itype = H5::PredType::NATIVE_INT;

    H5::CompType type(sizeof(H5Axis));
    type.insertMember("name",   HOFFSET(H5Axis, name  ), stype);
    type.insertMember("n_bins", HOFFSET(H5Axis, n_bins), itype);
    type.insertMember("min",    HOFFSET(H5Axis, min   ), dtype);
    type.insertMember("max",    HOFFSET(H5Axis, max   ), dtype);
    type.insertMember("units",  HOFFSET(H5Axis, units ), stype);

    // write the attribute
    auto attr = targ.createAttribute("axes", type, space);
    attr.write(type, h5_axes.data());
  }

  // templates to write attributes.
  template<typename M>
  void write_attr(H5::DataSet& loc, const std::string& name, M value) {
    auto type = get_type(value);
    loc.createAttribute(name, type, H5S_SCALAR).write(type, &value);
  }

  // called by the attribute writers to get the correct datatype.
  H5::PredType get_type(int) {
    return H5::PredType::NATIVE_INT;
  }

}

//______________________________________________________________________
// exception definitions

HistogramSaveError::HistogramSaveError(const std::string& what):
  std::runtime_error(what)
{}

