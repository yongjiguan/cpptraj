#ifndef INC_DATAIO_GRACE_H
#define INC_DATAIO_GRACE_H
#include "DataIO.h"
/// Read/write Grace data files.
class DataIO_Grace : public DataIO {
  public:
    DataIO_Grace() : DataIO(true, false, false), isInverted_(false) {} // Valid for 1D only
    static BaseIOtype* Alloc() { return (BaseIOtype*)new DataIO_Grace(); } 
    static void WriteHelp();
    int processReadArgs(ArgList &) { return 0; }
    int ReadData(FileName const&,DataSetList&,std::string const&);
    int processWriteArgs(ArgList &);
    int WriteData(FileName const&,DataSetList const&);
    bool ID_DataFormat(CpptrajFile&) { return false; }
  private:
    int WriteDataNormal(CpptrajFile&,DataSetList const&);
    int WriteDataInverted(CpptrajFile&,DataSetList const&);
    bool isInverted_; ///<  For 1D writes invert X/Y.
};
#endif
