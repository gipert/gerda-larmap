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

    auto filename = argc > 1 ? argv[1] : "gerda-larmap.root";
    std::cout << "INFO: opening " << filename << std::endl;
    TFile f(filename, "read");
    auto h = dynamic_cast<TH3D*>(f.Get("LAr_prob_map"));

    int missing = 0;
    int big_error = 0;
    int non_phys = 0;
    int ncells = h->GetNcells();

    ProgressBar bar(ncells); std::cout << "INFO: ";
    for (int i = 0; i < ncells; ++i) {
        if      (h->GetBinContent(i) < 0)                      missing += 1;
        else if (h->GetBinContent(i) > 1)                      non_phys += 1;
        else if (h->GetBinError(i) > 0.01*h->GetBinContent(i)) big_error += 1;
        bar.Update();
    }
    std::cout << std::endl;

    if (missing) {
        std::cerr << "WARNING: " <<  missing << "/" << ncells << " ("
                  << round(missing*1000./ncells)/10
                  << "%) invalid (<0) voxels found\n";
    }
    if (non_phys) {
        std::cerr << "WARNING: " <<  non_phys << "/" << ncells << " ("
                  << round(non_phys*1000./ncells)/10
                  << "%) non-physical (>1) voxels found\n";
    }
    if (big_error) {
        std::cerr << "WARNING: " << big_error << "/" << ncells << " ("
                  << round(big_error*1000./ncells)/10
                  << "%) voxels with uncertainty > 1% found\n";
    }

    if (missing || big_error) return 1;
    else                      return 0;
}
