#include "StPidHistMaker.h"

#include <TMath.h>

#include <algorithm>
#include <fstream>
#include <vector>

#include "StBTofUtil/tofPathLength.hh"
#include "StPicoDstMaker/StPicoDstMaker.h"
#include "StPicoEvent/StPicoBTofPidTraits.h"
#include "StPicoEvent/StPicoDst.h"
#include "StPicoEvent/StPicoEpdHit.h"
#include "StPicoEvent/StPicoEvent.h"
#include "StPicoEvent/StPicoTrack.h"
#include "StPicoEvent/StPicoPhysicalHelix.h"
#include "StThreeVectorF.hh"
#include "Stiostream.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TRandom.h"
#include "TTree.h"
#include "phys_constants.h"

#include "StRoot/CentCorrTool/CentCorrTool.h"
#include "StRoot/MeanDcaTool/MeanDcaTool.h"
#include "StRoot/TpcShiftTool/TpcShiftTool.h"
#include "StRoot/TriggerTool/TriggerTool.h"
#include "StRoot/StCFMult/StCFMult.h"

StPidHistMaker::StPidHistMaker(
	const char* name, 
	StPicoDstMaker* picoMaker,
    const char* outName
) : StMaker(name) {
	mOutputName = outName;
	mPicoDstMaker = picoMaker;
	mPicoDst = 0;
}

StPidHistMaker::~StPidHistMaker() {}

Int_t StPidHistMaker::Init() {
  	mFileOut = new TFile(mOutputName, "recreate");

	hNev = new TH1D(
		"hNev", "Number of Events;Cuts;Counts",
		10, -0.5, 9.5
	);
	// column 0: All events without any cut
	// column 1: with bad run cut
	// column 2: mb trigger cut
	// column 3: with vr cut
	// column 4: with 50 vz cut
	// column 5: with DCAz cut (no sDCAxy cut)
	// column 6: with sDCAxy cut (no DCAz cut)
	// column 7: with both DCAz and xy cut
	// column 8: with pile up cut
	// column 9: 0~80% centrality

	// initialize costume modules

	// bad run tool
	mtBadRun = new BadRunTool();

	// mean dca tool
	mtDca = new MeanDcaTool();
	mtDca->ReadParams();

	// centrality tool
	mtCent = new CentCorrTool();
	mtCent->EnableIndianMethod(true);
	mtCent->ReadParams();

	// multiplicity and shift tool
	mtShift = new TpcShiftTool();
	mtShift->Init();
	mtMult = new StCFMult();
	mtMult->ImportShiftTool(mtShift);

	// trigger tool
	mtTrg = new TriggerTool();

	return kStOK;
}

//---------------------------------------------------------
Int_t StPidHistMaker::Finish() {
	mFileOut->cd();
	hNev->Write()
	mFileOut->Close();
	return kStOK;
}

void StPidHistMaker::Clear(Option_t* opt) {}

//---------------------------------------------------------------
Int_t StHistMaker::Make() {
	if (!mPicoDstMaker) {
		LOG_WARN << " No PicoDstMaker! Skip! " << endm;
		return kStWarn;
	}

	mPicoDst = mPicoDstMaker->picoDst();
	if (!mPicoDst) { return kStOK; }
	if (!mPicoDst) { return kStOK; }

	// Load event
	event = (StPicoEvent*)mPicoDst->event();
	if (!event) { return kStOK; }

	hNev->Fill(0); // all events

	Int_t runId = event->runId();
	if (mtBadRun->isBadRun(runId)) { return kStOK; }
	hNev->Fill(1); // with bad run cut

	Int_t trgid = mtTrg->GetTriggerID(event);
	if (trgid < 0) { return kStOK; }
	hNev->Fill(2); // with mb trigger cut

	TVector3 pVtx = event->primaryVertex();
	Double_t vx = pVtx.X();
	Double_t vy = pVtx.Y();
	Double_t vz = pVtx.Z();

	if (fabs(vx) < 1.e-5 && 
		fabs(vy) < 1.e-5 &&
		fabs(vz) < 1.e-5) {
		return kStOK;
	}

	// using Ashish's shifted vr cut
	// -> see: https://drupal.star.bnl.gov/STAR/system/files/Vr_xy_N_Vzcut.txt
	vx = vx - 0.0417;
	vy = vy + 0.2715;
	Double_t vr = sqrt(vx * vx + vy * vy);

	if (vr >= 1.0) { return kStOK; }
	hNev->Fill(3); // with vr cut

	if (fabs(vz) > 50.0) { return kStOK; }
	hNev->Fill(4); // with vr cut

	// check DCA
	if (!mtDca->Make(mPicoDst)) { return kStOK; }
	if (!mtDca->IsBadMeanDcaZEvent(mPicoDst)) {
		hNev->Fill(5); // only dcaz cut
	}
	if (!mtDca->IsBadMeanDcaXYEvent(mPicoDst)) {
		hNev->Fill(6); // only dcaxy cut
	}
	if (mtDca->IsBadMeanDcaZEvent(mPicoDst) || mtDca->IsBadMeanDcaXYEvent(mPicoDst)) {
		return kStOK;
	}
	hNev->Fill(7); // both dca cut

	mtMult->make(mPicoDst);
	Int_t refMult = mtMult->mRefMult;
	Int_t tofMult = mtMult->mTofMult;
	Int_t nTofMatch = mtMult->mNTofMatch;
	Int_t nTofBeta = mtMult->mNTofBeta;

	Int_t refMult3 = mtMult->mRefMult3;
	if (mtCent->IsIndianPileUp(refMult, tofMult, nTofMatch, nTofBeta)) { return kStOK; }
	hNev->Fill(8); // pileup cut

	refMult3 = mtCent->GetIndianRefMult3Corr(
		refMult, refMult3, tofMult, nTofMatch, nTofBeta,
		vz, false
	);
	if (refMult3 < 0) { return kStOK; }
	Int_t cent = mtCent->GetCentrality9(refMult3);
	if (cent < 0 || cent >= 9) { return kStOK; }
	hNev->Fill(9); // 0-80%, also the final number of events for analysis

	return kStOK;
}
