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
#include <string>
#include <vector>

// ROOT
#include "TFile.h"
#include "TH3D.h"

#include "ProgressBar.h"

int main(int argc, char** argv) {

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    int ws = 3; // window size
    auto result1 = std::find(args.begin(), args.end(), "-w");
    auto result2 = std::find(args.begin(), args.end(), "--width");
    if (result1 != args.end()) {
        ws = std::stoi(*(result1+1));
        args.erase(result1, result1+2);
    }
    else if (result2 != args.end()) {
        ws = std::stoi(*(result2+1));
        args.erase(result2, result2+2);
    }
    if (ws <= 1) {
        std::cout << "ERROR: invalid window size\n";
        return 1;
    }

    auto filename = args.size() >= 1 ? args[0] : "gerda-larmap-merged.root";
    std::cout << "INFO: smoothing " << filename << "...\n";
    TFile f(filename.c_str(), "read");
    auto h = dynamic_cast<TH3D*>(f.Get("LAr_prob_map"));
    if (!h) {
        std::cout << "ERROR: LAr_prob_map object not found\n";
        return 1;
    }

    // save some time by copying only the structure
    TH3D h_sm(
        "LAr_prob_map_smooth", h->GetTitle(),
        h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax(),
        h->GetNbinsY(), h->GetYaxis()->GetXmin(), h->GetXaxis()->GetXmax(),
        h->GetNbinsZ(), h->GetZaxis()->GetXmax(), h->GetXaxis()->GetXmax()
    );

    float _sum;
    int _terms;

    int nb_x = h->GetNbinsX();
    int nb_y = h->GetNbinsY();
    int nb_z = h->GetNbinsZ();

    std::cout << "INFO: using smoothing window size = " << ws << std::endl;

    ProgressBar bar(nb_x*nb_y*nb_z);
    std::cout << "INFO: ";

    // loop over the histogram cells (no overflow)
    for (int i = 1; i < nb_x; ++i) {
        for (int j = 1; j < nb_y; ++j) {
            for (int k = 1; k < nb_z; ++k) {
                // if bin does not contain LAr (-1), leave it as it is
                if (h->GetBinContent(i, j, k) < 0) {
                    h_sm.SetBinContent(i, j, k, -1);
                    bar.Update();
                    continue;
                }
                // otherwise calculate the average over the surrounding cube
                _sum = 0; _terms = 0;
                for (int _i = i-ws; _i <= i+ws; ++_i) {
                    // check if we are out-of-bound
                    if (_i <= 0 or _i >= nb_x) continue;
                    for (int _j = j-ws; _j <= j+ws; ++_j) {
                        // check if we are out-of-bound
                        if (_j <= 0 or _j >= nb_y) continue;
                        for (int _k = k-ws; _k <= k+ws; ++_k) {
                            // check if we are out-of-bound
                            if (_k <= 0 or _k >= nb_z) continue;
                            // check if the bin contains LAr
                            if (h->GetBinContent(_i, _j, _k) >= 0) {
                                _sum += h->GetBinContent(_i, _j, _k);
                                _terms++;
                            }
                        }
                    }
                }
                h_sm.SetBinContent(i, j, k, _sum/_terms);
                bar.Update();
            }
        }
    }

    filename.erase(std::find(filename.begin(), filename.end(), '.'), filename.end());
    auto outname = filename + "-smoothed-w" + std::to_string(ws) + ".root";
    TFile fout(outname.c_str(), "RECREATE");
    h_sm.Write("LAr_prob_map");

    std::cout << "INFO: " + outname + " created";

    return 0;
}
