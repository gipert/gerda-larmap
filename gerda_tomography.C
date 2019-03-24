/* gerda_tomography.C
 *
 * Author: Luigi Pertoldi - pertoldi@pd.infn.it
 * Created: Sun 24 Mar 2019
 */

const std::string OpenFileDialog() {
   // Prompt for file to be opened. Depending on navigation in
   // dialog the current working directory can be changed.
   // The returned file name is always with respect to the
   // current directory.

   const char *gOpenAsTypes[] = {
      "ROOT files",   "*.root",
      "All files",    "*",
       0,              0
   };

   static TGFileInfo fi;
   fi.fFileTypes = gOpenAsTypes;
   new TGFileDialog(gClient->GetRoot(), gClient->GetRoot(), kFDOpen, &fi);

   return std::string(fi.fFilename);
}

void gerda_tomography() {

    std::string filename = OpenFileDialog();

    TFile* tfile = new TFile(filename.c_str(), "read");
    auto LAr_prob_map = dynamic_cast<TH3D*>(tfile->Get("LAr_prob_map"));

    LAr_prob_map->RebinX(2);
    LAr_prob_map->RebinY(2);
    LAr_prob_map->RebinZ(2);

    std::cout << "INFO: displaying...\n";
    TCanvas * c = new TCanvas(
        "gerda_tomography", "GERDA Tomography",
        600, 1200
    );
    c->SetMargin(0, 0, 0, 0);

    LAr_prob_map->SetTitle("LAr photon detection probability map");
    LAr_prob_map->SetStats(false);
    LAr_prob_map->Draw("a box");

    LAr_prob_map->SetShowProjection("xy col");
    gPad->SetTitle("GERDA Tomography (projection)");

    return;
}
