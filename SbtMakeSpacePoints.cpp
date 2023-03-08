#include <cassert>
#include <iostream>
#include <string>

#include "SbtCluster.h"
#include "SbtDetectorType.h"
#include "SbtEvent.h"
#include "SbtHit.h"
#include "SbtMakeSpacePoints.h"
#include "SbtSpacePoint.h"

ClassImp(SbtMakeSpacePoints);

SbtMakeSpacePoints::SbtMakeSpacePoints(std::string ErrorMethod, double trackDetErr)
    : _DebugLevel(0) {
  std::cout << "SbtMakeSpacePoints:  DebugLevel= " << _DebugLevel << std::endl;
  _errorMethod = ErrorMethod;
  _trackDetErr = trackDetErr;

  if (_DebugLevel)
    std::cout << "SbtMakeSpacePoints:  _errorMethod = " << _errorMethod << std::endl;
}

//
// for each wafer with strips, create space points corresponding one strip hit
// on one side and another strip hit on the other side
//
// get a space point for each pixel hit
//
void SbtMakeSpacePoints::makeSpacePoints(SbtEvent* event) {
  // create a List of SpacePoint for each Telescope detector

  for (auto& hit1 : event->GetHitList()) {
    if (hit1.GetDetectorElem()->GetDetectorType()->GetType() == "singleside") {
      TVector3 point(-999., -999., -999);
      if (hit1.isOnSingleSide(point)) {
        // fill the SpacePoint list corresponding to the DetElemID
        event->AddSpacePoint(SbtSpacePoint(point, hit1.GetDetectorElem(), &hit1, _errorMethod, _trackDetErr));

        if (_DebugLevel > 1) {
          std::cout << "SbtMakeSpacePoints::CreateSpacePoints() new point" << std::endl
               << "  "
               << "DetElemID = " << hit1.GetDetectorElem()->GetID()
               << " "
               << "point = " << point[0] << " " << point[1] << " " << point[2]
               << std::endl;
        }
      }
    }

    else {
      if (hit1.GetSide() != SbtEnums::view::U) continue;
      for (auto& hit2 : event->GetHitList()) {
        if (hit2.GetSide() != SbtEnums::view::V) continue;
        if (_DebugLevel > 3) {
          std::cout << "consider"
               << " hit1, detid=" << hit1.GetDetectorElem()->GetID()
               << " side=" << hit1.GetSide()
               << " hit2, detid=" << hit2.GetDetectorElem()->GetID()
               << " side=" << hit2.GetSide() << std::endl;
        }
        if ((hit1.GetDetectorElem()->GetID() == hit2.GetDetectorElem()->GetID())) {
          if (_DebugLevel > 2) {
            std::cout << "SbtMakeSpacePoints::CreateSpacePoints() DetElemID =  "
                 << hit1.GetDetectorElem()->GetID() << std::endl;
          }

          TVector3 point(-999., -999., -999);
          if (hit1.Intersection(hit2, point)) {
            // fill the SpacePoint list corresponding to the DetElemID
            event->AddSpacePoint(SbtSpacePoint(point, hit1.GetDetectorElem(), &hit1, &hit2, _errorMethod, _trackDetErr));

            if (_DebugLevel > 1) {
              std::cout << "SbtMakeSpacePoints::CreateSpacePoints() new point"
                   << std::endl
                   << "  "
                   << "DetElemID= "
                   << hit1.GetDetectorElem()->GetID() << " "
                   << "point= " << point[0] << " " << point[1] << " "
                   << point[2] << std::endl;
            }
          }
        }
      }
    }
  }
  // consider here SP from pixel detectors
  //
  for (auto& pixelCluster : event->GetPxlClusterList()) {
    // fill the SpacePoint List with the pixel SP
    event->AddSpacePoint(SbtSpacePoint(&pixelCluster, _errorMethod, _trackDetErr));
  }

  if (_DebugLevel) {
    std::cout << "SbtMakeSpacePoints::makeSpacePoints()" << std::endl;
    std::cout << "Size SpacePointList = " << event->GetSpacePointList().size() << std::endl;
  }
}
