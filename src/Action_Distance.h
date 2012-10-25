#ifndef INC_ACTION_DISTANCE_H
#define INC_ACTION_DISTANCE_H
#include "Action.h"
#include "ImagedAction.h"
// Class: Action_Distance
/// Action to calculate a distance between atoms in two masks.
class Action_Distance: public Action, ImagedAction {
  public:
    Action_Distance();

    static DispatchObject* Alloc() { return (DispatchObject*)new Action_Distance(); }
    static void Help();

    void Print() {}
  private:
    DataSet *dist_;
    bool useMass_;
    AtomMask Mask1_;
    AtomMask Mask2_;

    Action::RetType Init(ArgList&, TopologyList*, FrameList*, DataSetList*,
                          DataFileList*, int);
    Action::RetType Setup(Topology*, Topology**);
    Action::RetType DoAction(int, Frame*, Frame**);
};
#endif  
