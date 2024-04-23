#ifndef _StHistMaker_head
#define _StHistMaker_head
#include "StMaker.h"
#include "StThreeVectorF.hh"
#include "TString.h"
#include "TVector3.h"
#include "StarClassLibrary/StThreeVectorF.hh"
#include "StarClassLibrary/StThreeVectorD.hh"

class StPicoDst;
class StPicoEvent;
class StPicoTrack;
class StPicoDstMaker;
class TH1F;
class TProfile;
class TTree;

class BadRunTool;
class StCFMult;
class TpcShiftTool;
class TriggerTool;
class MeanDcaTool;
class CentCorrTool;


class StHistMaker : public StMaker {
	public:
		StHistMaker(const char *name, StPicoDstMaker *picoMaker, const char *outName="tofMatchTree.root");
		virtual ~StHistMaker();

		virtual Int_t Init();
		virtual Int_t Make();
		virtual void  Clear(Option_t *opt="");
		virtual Int_t Finish();

	private:
		StPicoDstMaker *mPicoDstMaker;
		StPicoDst      *mPicoDst;
		StPicoEvent	   *event;
		StPicoTrack    *picoTrack;

		BadRunTool* mtBadRun;
		StCFMult* mtMult;
		TpcShiftTool* mtShift;
		CentCorrTool* mtCent;
		MeanDcaTool* mtDca;
		TriggerTool* mtTrg;

		TH1F* hNev;

		TString mOutputName;
		TFile* mFileOut;

		ClassDef(StHistMaker, 1)
};

ClassImp(StHistMaker)

#endif
