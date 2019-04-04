/* map-smoother.cc
 *
 * Smoother of GERDA LAr probability maps.
 * Does a 3D Moving Window Averaging
 *
 * Author: Luigi Pertoldi - pertoldi@pd.infn.it
 * Created: Sun 4 Apr 2019
 */

// STL
#include <iostream>

// ROOT
#include "TFile.h"
#include "TH3D.h"

int main(int argc, char** argv) {

    auto filename = argc > 1 ? std::string(argv[1]) : "gerda-larmap-merged.root";
    std::cout << "INFO: smoothing " << filename << "...\n";
    TFile f(filename.c_str(), "read");
    auto h = dynamic_cast<TH3D*>(f.Get("LAr_prob_map"));
    TH3D h_sm(
        "LAr_prob_map_smooth", h->GetTitle(),
        h->GetNbinsX(), h->GetXaxis()->GetXbins()->GetArray(),
        h->GetNbinsX(), h->GetYaxis()->GetXbins()->GetArray(),
        h->GetNbinsX(), h->GetZaxis()->GetXbins()->GetArray()
    );

    // int ncells = h->GetNcells();
    // for (int i = 0; i < ncells; ++i) {
    // }

    filename.erase(std::find(filename.begin(), filename.end(), '.'), filename.end());
    auto outname = filename + "-smoothed.root";
    TFile fout(outname.c_str(), "RECREATE");
    h_sm.Write();

    return 0;
}
