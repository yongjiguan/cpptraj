// DataSet_integer
#include "DataSet_integer.h"
#include "MpiRoutines.h"
#include "CpptrajStdio.h"
using namespace std;

// CONSTRUCTOR
DataSet_integer::DataSet_integer() {
  width=12;
  dType=INT;
  setFormatString();
}

/* DataSet_integer::Xmax(()
 * Return the maximum X value added to this set. By convention this is 
 * always the last value added.
 */
int DataSet_integer::Xmax() {
  // If no data has been added return 0
  if (current==0) return 0;
  it=Data.end();
  it--;
  return ( (*it).first );
}

/* DataSet_integer::Add()
 * Insert data vIn at frame.
 */
void DataSet_integer::Add(int frame, void *vIn) {
  int *value;

  value = (int*) vIn;

  // Always insert at the end
  it=Data.end();
  Data.insert( it, pair<int,int>(frame, *value) );
  current++;
}

/* DataSet_integer::Get()
 * Get data at frame, put into vOut. Return 1 if no data at frame.
 */
int DataSet_integer::Get(void *vOut, int frame) {
  int *value;

  if (vOut==NULL) return 1;
  value = (int*) vOut;
  it=Data.find( frame );
  if (it == Data.end()) return 1;
  *value = (*it).second;
  return 0;
}

/* DataSet_integer::isEmpty()
 */
int DataSet_integer::isEmpty(int frame) {
  it = Data.find( frame );
  if (it == Data.end()) return 1;
  return 0;
}

/* DataSet_integer::WriteBuffer()
 * Write data at frame to CharBuffer. If no data for frame write 0.
 */
void DataSet_integer::WriteBuffer(CharBuffer &cbuffer, int frame) {
  int ival;
  it = Data.find( frame );
  if (it == Data.end())
    ival = 0;
  else
    ival = (*it).second;
  cbuffer.WriteInteger(format, ival);
}

/* DataSet_integer::Width()
 */
int DataSet_integer::Width() {
  return (width + 1);
}

/* DataSet_integer::Sync()
 * Since it seems to be very difficult (or impossible) to define Classes
 * as MPI datatypes, first non-master threads need to convert their maps
 * into 2 arrays, an int array containing frame #s and another int array
 * containing mapped values. These arrays are then sent to the master,
 * where they are converted pairs and inserted into the master map.
 */
int DataSet_integer::Sync() {
  int rank, i, dataSize;
  int *Frames;
  int *Values;

  if (worldsize==1) return 0;

  for (rank = 1; rank < worldsize; rank ++) {
    // Get size of map on rank 
    if (worldrank>0) {
      // NOTE: current should be equal to size(). Check for now
      rprintf( "DataSet_integer syncing. current=%i, size=%u\n",
              current, Data.size());
      if (current != (int) Data.size()) {
        rprintf("ERROR: current and map size are not equal.\n");
        return 1;
      }
      dataSize = current;
    }

    // Send size of map on rank to master, allocate arrays on rank and master
    parallel_sendMaster(&dataSize, 1, rank, 0);
    rprintf("DataSet_integer allocating %i for send/recv\n",dataSize);
    Frames = new int[dataSize];
    Values = new int[dataSize];
      
    // On non-master convert map to 2 int arrays.
    if (worldrank > 0) {
      i=0;
      for ( it = Data.begin(); it != Data.end(); it++ ) {
        Frames[i]=(*it).first;
        Values[i]=(*it).second;
        i++;
      }
    }

    // Send arrays to master
    parallel_sendMaster(Frames, dataSize, rank, 0);
    parallel_sendMaster(Values, dataSize, rank, 0);

    // On master convert arrays to pairs and insert to master map
    if (worldrank==0) {
      for (i=0; i < dataSize; i++) 
        Data.insert( pair<int,int>( Frames[i], Values[i] ) );
    }

    // Free arrays
    delete[] Frames;
    delete[] Values;
  } // End loop over ranks>0

  return 0;
}
