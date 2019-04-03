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

int main(int argc, char** argv) {

    auto primaries_th = 100;

    auto filename = argc > 1 ? argv[1] : "gerda-larmap-merged.root";
    std::cout << "INFO: visiting " << filename << "...\n";
    TFile f(filename, "read");
    auto h = dynamic_cast<TH3D*>(f.Get("LAr_prob_map"));
    auto v = dynamic_cast<TH3D*>(f.Get("LAr_vertex_map"));

    int missing_v, missing, big_error, non_phys, p_equal_one, p_equal_zero;
    missing_v = missing = big_error = non_phys = p_equal_one = p_equal_zero = 0;

    int ncells = h->GetNcells();

    for (int i = 0; i < ncells; ++i) {
        auto _p     = h->GetBinContent(i);
        auto _sigma = h->GetBinError(i);
        auto _v     = v->GetBinContent(i);

        if      (_v <= 0) missing_v++;

        if      (_p < 0) missing++;
        else if (_p > 1) non_phys++;
        else if (_p == 0 and _v < primaries_th) p_equal_zero++;
        else if (_p == 1 and _v < primaries_th) p_equal_one++;
        else if (_sigma > 0.01*_p) big_error++;
    }

    auto ref_empty_perc = round(missing_v*1000./ncells)/10;
    auto empty_perc     = round(missing*1000./ncells)/10;

    std::cout << "INFO: " <<  missing_v << "/" << ncells << " ("
              << ref_empty_perc
              << "%) empty voxels found in the primary vertex map\n";

    if (empty_perc != ref_empty_perc) {
        std::cerr << "WARNING: different number of invalid (<0) voxels found in probability map: "
                  << empty_perc << ". Ignore this warning if you think you *did not* sample all "
                  << "the LAr within the considered space\n";
    }
    if (non_phys) {
        std::cerr << "ERROR: " <<  non_phys << "/" << ncells << " ("
                  << round(non_phys*1000./ncells)/10
                  << "%) non-physical (>1) voxels found\n";
    }
    if (p_equal_zero) {
        std::cerr << "WARNING: " << p_equal_zero << "/" << ncells << " ("
                  << round(p_equal_zero*1000./ncells)/10
                  << "%) voxels with non reliable probability estimate "
                  << "(p = 0 and primaries < " << primaries_th << ") found\n";
    }
    if (p_equal_one) {
        std::cerr << "WARNING: " << p_equal_one << "/" << ncells << " ("
                  << round(p_equal_one*1000./ncells)/10
                  << "%) voxels with non reliable probability estimate "
                  << "(p = 1 and primaries < " << primaries_th << ") found\n";
    }
    if (big_error) {
        std::cerr << "WARNING: " << big_error << "/" << ncells << " ("
                  << round(big_error*1000./ncells)/10
                  << "%) voxels with non reliable probability estimate "
                  << "(σ > 1%) found\n";
    }

    if (missing || non_phys || big_error) return 1;
    else                                  return 0;
}
