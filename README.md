## ndhist

Lightweight histogram class for filling and saving to HDF5.

Histograms can be arbitrary dimensions (up to 32 can be saved by 
HDF5). The result is saved as a HDF5 DataSet, with attributes 
indicating the binning. Attributes are saved as an array, with the 
Nth entry in the array corresponding to the Nth axis. The following
attributes are saved: 

- `names`: axis names, each axis must have a unique name
- `n_bins`: number of bins in this axis (not including overflow)
- `min`: bottom range of the axis
- `max`: upper range of the axis
- `units`: (optional) string indicating units

### Constructor Flags

Several flags can be passed to the Histogram constructor. 

 - `hist::wt2`: Fill a second histogram with weights of weight^2. The
   second histogram will be saved as "<first histogram name>Wt2". This is
   useful to calculate statistical error in weighted samples, but will
   double the amount of space required in memory / on disk.

 - `hist::eat_nan`: Without this flag, passing `fill(...)` NaN results in a
   `std::range_error`. With it, Histogram will record the number of times
   NaN is passed, and save an additional attribute `nan`.  There is only
   one NaN counter for all axes.
