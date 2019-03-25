/* larmap-doctor.cc
 *
 * Health check of GERDA LAr probability maps.
 *
 * Author: Luigi Pertoldi - pertoldi@pd.infn.it
 * Created: Sun 24 Mar 2019
 */

// STL
#include <iostream>

// ROOT
#include "TFile.h"
#include "TH3D.h"

// other
#include "ProgressBar.h"

int main(int argc, char** argv) {

    auto primaries_th = 100;

    auto filename = argc > 1 ? argv[1] : "gerda-larmap.root";
    std::cout << "INFO: opening " << filename << std::endl;
    TFile f(filename, "read");
    auto h = dynamic_cast<TH3D*>(f.Get("LAr_prob_map"));
    auto v = dynamic_cast<TH3D*>(f.Get("LAr_vertex_map"));

    int missing = 0;
    int big_error = 0;
    int non_phys = 0;
    int ncells = h->GetNcells();

    ProgressBar bar(ncells); std::cout << "INFO: ";
    for (int i = 0; i < ncells; ++i) {
        auto _p     = h->GetBinContent(i);
        auto _sigma = h->GetBinError(i);

        if      (_p < 0) missing++;
        else if (_p > 1) non_phys++;
        else if (_sigma == 0) { if (v->GetBinContent(i) < primaries_th) big_error++; }
        else if (_sigma > 0.01*_p) big_error++;

        bar.Update();
    }
    std::cout << std::endl;

    if (missing) {
        std::cerr << "WARNING: " <<  missing << "/" << ncells << " ("
                  << round(missing*1000./ncells)/10
                  << "%) invalid (<0) voxels found\n";
    }
    if (non_phys) {
        std::cerr << "ERROR: " <<  non_phys << "/" << ncells << " ("
                  << round(non_phys*1000./ncells)/10
                  << "%) non-physical (>1) voxels found\n";
    }
    if (big_error) {
        std::cerr << "WARNING: " << big_error << "/" << ncells << " ("
                  << round(big_error*1000./ncells)/10
                  << "%) voxels with non reliable probability estimate "
                  << "( σ > 1% or primaries < " << primaries_th << ") found\n";
    }

    if (missing || non_phys || big_error) return 1;
    else                                  return 0;
}
