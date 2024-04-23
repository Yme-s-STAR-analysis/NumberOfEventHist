#include <TSystem>

class StMaker;
class StChain;
class StPicoDstMaker;
class StPicoEvent;
class StHistMaker;

StChain *chain;
void readPicoDst(const Char_t *inputFile = "file.list", TString JobIdName = "nEv") {
    Int_t nEvents = 15000000;

    gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
    loadSharedLibraries();

    gSystem->Load("StUtilities");
    gSystem->Load("StPicoEvent");
    gSystem->Load("StPicoDstMaker");
    gSystem->Load("StHistMaker");
    gSystem->Load("BadRunTool");
    gSystem->Load("StCFMult");
    gSystem->Load("TpcShiftTool");
    gSystem->Load("CentCorrTool");
    gSystem->Load("MeanDcaTool");
    gSystem->Load("TriggerTool");

    chain = new StChain();

    TString Name = JobIdName;
    Name.Append(".root");
    StPicoDstMaker *picoMaker = new StPicoDstMaker(2, inputFile, "picoDst");
    StHistMaker *anaMaker = new StHistMaker("ana", picoMaker, Name);

    chain->Init();
    cout << "chain->Init();" << endl;
    int total = picoMaker->chain()->GetEntries();
    cout << " Total entries = " << total << endl;
    if (nEvents > total) {
        nEvents = total;
    }
    int percent = 0; // (0->9)
    for (Int_t i = 0; i < nEvents; i++) {
        if (i > nEvents*1.0*percent / 10) {
            cout << " Working on eventNumber " << i << " / " << nEvents << endl;
            percent += 1;
        }

        chain->Clear();
        int iret = chain->Make(i);

        if (iret) {
            cout << "Bad return code!" << iret << endl;
            break;
        }

        total++;
    }

    cout << "****************************************** " << endl;
    cout << "Work done... now its time to close up shop!" << endl;
    cout << "****************************************** " << endl;
    chain->Finish();
    cout << "****************************************** " << endl;
    cout << "total number of events  " << nEvents << endl;
    cout << "****************************************** " << endl;

    delete chain;
}
