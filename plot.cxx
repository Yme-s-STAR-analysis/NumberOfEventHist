void plot() {

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

    TFile* tf = new TFile("Nev.hadd.root");
    TH1F* h1 = (TH1F*)tf->Get("hNev");

    TCanvas* c = new TCanvas();
    c->cd();
    gPad->SetLogy();
    gStyle->SetOptStat(0);

    const char* labels[10] = {
        "Raw Data",
        "Bad Run Cut",
        "MB Triggers",
        "Vr < 1 cm",
        "|Vz| < 50 cm",
        "<DCAz> Cut",
        "<sDCAxy> Cut",
        "both <DCA> Cut",
        "Pile-up Cut",
        "0~80%% Central"
    };

    for (int i=0; i<10; i++) {
        h1->GetXaxis()->SetBinLabel(i+1, labels[i]);
    }

    h1->GetXaxis()->SetTitle("");
 //   h1->GetYaxis()->SetRangeUser(1e7, 7e8);


    h1->Draw("histtext");
    TLatex* lat = new TLatex();
    lat->DrawLatexNDC(0.4, 0.8, "STAR BES-II");
    lat->DrawLatexNDC(0.4, 0.7, "Au + Au @ DATASET GeV");

    // print the numbers
    for (int i=0; i<10; i++) {
        int x = (int)h1->GetBinContent(i+1);
        cout << "Column " << i << ", " << labels[i] << " : " << x << ".\n";
    }

    c->Print("stat.pdf");
}
