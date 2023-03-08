#include <cassert>
#include <iostream>

#include "SbtDetectorElem.h"
#include "SbtDetectorType.h"
#include "SbtHit.h"
#include "SbtSimple3DFittingAlg.h"
#include "SbtSpacePoint.h"
#include "SbtTrack.h"

using namespace std;

ClassImp(SbtSimple3DFittingAlg);

SbtSimple3DFittingAlg::SbtSimple3DFittingAlg() : _nSpOnTrk(0) {
  if (getDebugLevel() > 0)
    cout << "SbtSimple3DFittingAlg:  DebugLevel= " << getDebugLevel() << endl;
  _algName = "Simple3D";
}

bool SbtSimple3DFittingAlg::fitTrack(SbtTrack& candidateTrack) {
  _nSpOnTrk = 0;
  vector<SbtSpacePoint*> SPCandList = candidateTrack.GetSpacePointList();
  ;

  // define here the number of space points from tracking detectors
  vector<SbtSpacePoint*>::iterator SPIterator;
  for (SPIterator = SPCandList.begin(); SPIterator != SPCandList.end();
       SPIterator++) {
    if ((*SPIterator)->GetDetectorElem()->GetTrackingID() >= 0) {
      _nSpOnTrk++;
    }
  }

  if (getDebugLevel()) {
    cout << "SbtSimple3DFittingAlg: SP candidate List size = "
         << SPCandList.size() << endl;
    cout << "SbtSimple3DFittingAlg: _nSpOnTrk = " << _nSpOnTrk << endl;
  }

  // implement simple Least Squares Method for 3D track
  // fit x, y coordinates independently

  // define vector and matrix of double
  // assume we fit tracks with at least 4 SpacePoints

  // Matrix of derivatives of line functional form wrt parameters
  TMatrixD FF(2 * _nSpOnTrk, 4);
  // Matrix of derivatives of line functional form wrt parameters
  TVectorD MM(2 * _nSpOnTrk);  // Vector of x SpacePoints measurements
  TMatrixD VV(2 * _nSpOnTrk,
              2 * _nSpOnTrk);  // Vector of SpacePoints measurements Errors
  TMatrixD RR(2 * _nSpOnTrk, 2 * _nSpOnTrk);
  // initialize Vector Error Matrix to zero
  for (int i = 0; i < 2 * _nSpOnTrk; i++) {
    for (int j = 0; j < 2 * _nSpOnTrk; j++) {
      VV[i][j] = 0.0;
      RR[i][j] = 0.0;
    }
  }

  // Matrix of derivatives of line functional form wrt parameters
  TMatrixD Fx(_nSpOnTrk, 2);
  // Matrix of derivatives of line functional form wrt parameters
  TMatrixD Fy(_nSpOnTrk, 2);
  TVectorD Mx(_nSpOnTrk);  // Vector of x SpacePoints measurements
  TVectorD My(_nSpOnTrk);  // Vector of y SpacePoints measurements
  TMatrixD Vx(_nSpOnTrk,
              _nSpOnTrk);  // Vector of x SpacePoints measurements Errors
  TMatrixD Vy(_nSpOnTrk,
              _nSpOnTrk);  // Vector of y SpacePoints measurements Errors
  // initialize Vector Error Matrix to zero
  for (int i = 0; i < _nSpOnTrk; i++) {
    for (int j = 0; j < _nSpOnTrk; j++) {
      Vx[i][j] = 0;
      Vy[i][j] = 0.0;
    }
  }

  vector<SbtSpacePoint*> SPList;
  SPList.clear();
  if (candidateTrack.GetType() == SbtEnums::objectType::reconstructed) {
    SPList = SPCandList;
  }
  else if (candidateTrack.GetType() == SbtEnums::objectType::simulated) {
    vector<SbtSpacePoint*>::iterator SPIter;
    for (SPIter = SPCandList.begin(); SPIter != SPCandList.end(); SPIter++) {
      // SP for fit only from strip detectors
      // DUT = strip detector not contemplated
      // modification NN 7/11/2013 in order to accept only tracking detectors
      // if ( (*SPIter)->GetDetectorElem()->GetDetectorType()->GetIntType() ==
      // 0)
      if ((*SPIter)->GetDetectorElem()->GetTrackingID() >= 0) {
        SPList.push_back(*SPIter);

        if (getDebugLevel()) {
          cout << "SP (x, y, z) =  "
               << "(" << (*SPIter)->GetXPosition() << ", "
               << (*SPIter)->GetYPosition() << ", " << (*SPIter)->GetZPosition()
               << ")" << endl;
          cout << "SP (Dx, Dy, Dz) =  "
               << "(" << (*SPIter)->GetXPositionErr() << ", "
               << (*SPIter)->GetYPositionErr() << ", "
               << (*SPIter)->GetZPositionErr() << ")" << endl;
        }
      }
    }
    if (SPList.size() < 8) {
      cout << "SbtSimple3DFittingAlg: ";
      cout << "Warning: simulated track has SP size = " << SPList.size()
           << " no track fit performed" << endl;
      candidateTrack.SetFitStatus(0);
      return false;
    }
  }

  if (getDebugLevel()) {
    cout << "Candidate Track Type = " << candidateTrack.GetType() << endl;
    cout << "SpacePoint size = " << SPList.size() << endl;
  }

  vector<SbtSpacePoint*>::iterator aSpacePoint;

  if (candidateTrack.GetType() == SbtEnums::objectType::reconstructed) {
    TVectorD Lambda(4);  // Vector of best parameters for x
    TMatrixD Cov(4, 4);  // Covariance Matrix of Lambda parameters

    TMatrixD CovX(2, 2);  // Covariance Matrix of LambdaX parameters
    TMatrixD CovY(2, 2);  // Covariance Matrix of LambdaY parameters

    // initialize Vectors and Cov Matrix for track paramters
    for (int i = 0; i < 4; i++) {
      Lambda[i] = Lambda[i] = 0.0;
      for (int j = 0; j < 4; j++) {
        Cov[i][j] = Cov[i][j] = 0.0;
      }
    }

    double xAxis[3] = {1, 0, 0};
    double rotxAxis[3] = {0, 0, 0};
    double c = 0;
    double s = 0;

    int i(0);
    for (aSpacePoint = SPList.begin(); aSpacePoint != SPList.end();
         aSpacePoint++) {
      // z measurements of Trk SpacePoints
      FF[i][0] = 1;
      FF[i][1] = (*aSpacePoint)->GetZPosition();
      FF[i][2] = 0.0;
      FF[i][3] = 0.0;
      FF[i + 1][0] = 0.0;
      FF[i + 1][1] = 0.0;
      FF[i + 1][2] = 1;
      FF[i + 1][3] = (*aSpacePoint)->GetZPosition();

      // x measurements of Trk SpacePoints
      MM[i] = (*aSpacePoint)->GetXPosition();
      // y measurements of Trk SpacePoints
      MM[i + 1] = (*aSpacePoint)->GetYPosition();

      // U measurement Errors of Trk SpacePoints
      VV[i][i] =
          (*aSpacePoint)->GetXPositionErr() * (*aSpacePoint)->GetXPositionErr();
      // V measurement Errors of Trk SpacePoints
      VV[i + 1][i + 1] =
          (*aSpacePoint)->GetYPositionErr() * (*aSpacePoint)->GetYPositionErr();

      (*aSpacePoint)->GetDetectorElem()->LocalToMaster(xAxis, rotxAxis);
      c = rotxAxis[0];
      s = rotxAxis[1];

      RR[i][i] = c;
      RR[i][i + 1] = -s;
      RR[i + 1][i] = s;
      RR[i + 1][i + 1] = c;

      i = i + 2;
    }

    // if (getDebugLevel() >1){
    //   PrintMeasMatrix( Fx, Fy, Mx, My, Vx, Vy);
    // }

    TMatrixD FFT = FF;
    FFT.T();  // define FxT Transpose Matrix
    TMatrixD VVInv = VV;
    VVInv.Invert();
    TMatrixD RRInv = RR;
    RRInv.Invert();
    TMatrixD Product = FFT * RR * VVInv * RRInv * FF;

    Cov = Product.Invert();  // Evaluate Covariance Matrix

    Lambda = Cov * FFT * RR * VVInv * RRInv * MM;

    //  int isGoodFit = FitStatus(LambdaX,LambdaY,CovX,CovY); // 0=failed 1=OK
    int isGoodFit = 1;

    // line equation x = Ax + Bx z
    // line equation y = Ay + By z

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        CovX[i][j] = Cov[i][j];
        CovY[i][j] = Cov[i + 2][j + 2];
      }
    }

    candidateTrack.SetAx(Lambda[0]);
    candidateTrack.SetAy(Lambda[2]);
    candidateTrack.SetBx(Lambda[1]);
    candidateTrack.SetBy(Lambda[3]);
    candidateTrack.SetFitStatus(isGoodFit);
    candidateTrack.SetXCovMatrix(CovX);
    candidateTrack.SetYCovMatrix(CovY);
    double chi2 = Chi2(candidateTrack);
    int ndof = 2 * (SPList.size() - 2);
    candidateTrack.SetChi2(chi2);
    candidateTrack.SetNdof(ndof);
    candidateTrack.Residual();
  }

  else if (candidateTrack.GetType() == SbtEnums::objectType::simulated) {
    TVectorD LambdaX(2);  // Vector of best parameters for x
    TVectorD LambdaY(2);  // Vector of best parameters for y
    TMatrixD CovX(2, 2);  // Covariance Matrix of LambdaX parameters
    TMatrixD CovY(2, 2);  // Covariance Matrix of LambdaY parameters
    // initialize Vectors and Cov Matrix for track paramters
    for (int i = 0; i < 2; i++) {
      LambdaX[i] = LambdaY[i] = 0.0;
      LambdaX[i] = LambdaY[i] = 0.0;
      for (int j = 0; j < 2; j++) {
        CovX[i][j] = CovY[i][j] = 0.0;
      }
    }

    int i(0);
    for (aSpacePoint = SPList.begin(); aSpacePoint != SPList.end();
         aSpacePoint++) {
      // z measurements of Trk SpacePoints
      Fx[i][0] = 1;
      Fx[i][1] = (*aSpacePoint)->GetZPosition();
      // z measurements of Trk SpacePoints
      Fy[i][0] = 1;
      Fy[i][1] = (*aSpacePoint)->GetZPosition();
      // x measurements of Trk SpacePoints
      Mx[i] = (*aSpacePoint)->GetXPosition();
      // y measurements of Trk SpacePoints
      My[i] = (*aSpacePoint)->GetYPosition();
      // x measurement Errors of Trk SpacePoints
      Vx[i][i] =
          (*aSpacePoint)->GetXPositionErr() * (*aSpacePoint)->GetXPositionErr();
      // y measurement Errors of Trk SpacePoints
      Vy[i][i] =
          (*aSpacePoint)->GetYPositionErr() * (*aSpacePoint)->GetYPositionErr();
      i++;
    }

    if (getDebugLevel() > 1) {
      PrintMeasMatrix(Fx, Fy, Mx, My, Vx, Vy);
    }

    TMatrixD FxT = Fx;
    FxT.T();  // define FxT Transpose Matrix
    TMatrixD FyT = Fy;
    FyT.T();  // define FyT Transpose Matrix
    TMatrixD VxInv = Vx;
    VxInv.Invert();  // define inverse x Error Matrix
    TMatrixD VyInv = Vy;
    VyInv.Invert();  // define inverse y Error Matrix
    TMatrixD ProductX = FxT * VxInv * Fx;
    TMatrixD ProductY = FyT * VyInv * Fy;
    CovX = ProductX.Invert();  // Evaluate Covariance Matrix
    CovY = ProductY.Invert();  // Evaluate Covariance Matrix

    LambdaX = CovX * FxT * VxInv * Mx;
    LambdaY = CovY * FyT * VyInv * My;

    if (getDebugLevel() > 1) {
      PrintFitMatrix(LambdaX, LambdaY, CovX, CovY);
    }

    int isGoodFit = FitStatus(LambdaX, LambdaY, CovX, CovY);  // 0=failed 1=OK

    // line equation x = Ax + Bx z
    // line equation y = Ay + By z

    candidateTrack.SetAx(LambdaX[0]);
    candidateTrack.SetAy(LambdaY[0]);
    candidateTrack.SetBx(LambdaX[1]);
    candidateTrack.SetBy(LambdaY[1]);
    candidateTrack.SetFitStatus(isGoodFit);
    candidateTrack.SetXCovMatrix(CovX);
    candidateTrack.SetYCovMatrix(CovY);
    double chi2 = Chi2(candidateTrack);
    int ndof = 2 * (SPList.size() - 2);
    candidateTrack.SetChi2(chi2);
    candidateTrack.SetNdof(ndof);
    candidateTrack.Residual();
  }

  return true;
}

int SbtSimple3DFittingAlg::FitStatus(TVectorD LambdaX, TVectorD LambdaY,
                                     TMatrixD CovX, TMatrixD CovY) {
  // check fit status here
  int fitStatus = 1;
  return fitStatus;
}

void SbtSimple3DFittingAlg::PrintMeasMatrix(TMatrixD aFx, TMatrixD aFy,
                                            TVectorD aMx, TVectorD aMy,
                                            TMatrixD aVx, TMatrixD aVy) {
  cout << "SbtSimple3DFittingAlg::PrintMeasMatrix " << endl;

  for (int i = 0; i < _nSpOnTrk; i++) {
    cout << "Fx[" << i << "][0]=" << aFx[i][0] << "\t "
         << "Fx[" << i << "][1]=" << aFx[i][1] << endl;
  }
  cout << endl;

  for (int i = 0; i < _nSpOnTrk; i++) {
    cout << "Fy[" << i << "][0]=" << aFy[i][0] << "\t "
         << "Fy[" << i << "][1]=" << aFy[i][1] << endl;
  }
  cout << endl;
  for (int i = 0; i < _nSpOnTrk; i++) {
    cout << "Mx[" << i << "]=" << aMx[i] << "\t "
         << "My[" << i << "]=" << aMy[i] << endl;
  }
  cout << endl;
  for (int i = 0; i < _nSpOnTrk; i++) {
    for (int j = 0; j < _nSpOnTrk; j++) {
      cout << "Vx[" << i << "][" << j << "]=" << aVx[i][j] << "\t ";
      if (j == _nSpOnTrk - 1) cout << endl;
    }
  }
  cout << endl;
  for (int i = 0; i < _nSpOnTrk; i++) {
    for (int j = 0; j < _nSpOnTrk; j++) {
      cout << "Vy[" << i << "][" << j << "]=" << aVy[i][j] << "\t ";
      if (j == _nSpOnTrk - 1) cout << endl;
    }
  }
  cout << endl;
}

void SbtSimple3DFittingAlg::PrintFitMatrix(TVectorD aLambdaX, TVectorD aLambdaY,
                                           TMatrixD aCovX, TMatrixD aCovY) {
  cout << "SbtSimple3DFittingAlg::PrintFitMatrix " << endl;
  cout << "LambdaX[0]=" << aLambdaX[0] << "\t"
       << "LambdaX[1]=" << aLambdaX[1] << endl;
  cout << "LambdaY[0]=" << aLambdaY[0] << "\t"
       << "LambdaY[1]=" << aLambdaY[1] << endl;
  cout << endl;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      cout << "CovX[" << i << "][" << j << "]=" << aCovX[i][j] << "\t"
           << "CovY[" << i << "][" << j << "]=" << aCovY[i][j] << "\t";
      if (j == 1) cout << endl;
    }
  }
  cout << endl;
}

SbtSimple3DFittingAlg::~SbtSimple3DFittingAlg() {}
