#ifndef INC_TRAJ_AMBERRESTARTNC_H
#define INC_TRAJ_AMBERRESTARTNC_H
#ifdef BINTRAJ
#include "TrajectoryIO.h"
#include "NetcdfFile.h"
// Class: Traj_AmberRestartNC
/// TrajectoryIO class for reading and writing Amber Netcdf Restarts
class Traj_AmberRestartNC : public TrajectoryIO, private NetcdfFile {
  public:
    Traj_AmberRestartNC();
    static TrajectoryIO* Alloc() { return (TrajectoryIO*)new Traj_AmberRestartNC(); }
    ~Traj_AmberRestartNC();
    // Inherited functions
    bool ID_TrajFormat(CpptrajFile&);
    int setupTrajin(std::string const&, Topology*);
    int setupTrajout(std::string const&, Topology*, int, bool);
    int openTrajin();
    void closeTraj();
    int readFrame(int,Frame&);
    int writeFrame(int,Frame const&);
    int processWriteArgs(ArgList&);
    void Info();
  private:
    double restartTime_;
    bool singleWrite_;
    double time0_;
    double dt_;
    FileName filename_;

    int readVelocity(int, Frame&) { return 1; }
    int processReadArgs(ArgList&) { return 0; }
};
#endif
#endif  
