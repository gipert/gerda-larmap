/* map-merger.cc
 *
 * Merge two (or more) LAr probability maps.
 * USAGE: map-merger [-o outfile.root] file.root file.root [file.root ...]
 *
 * Author: Luigi Pertoldi - pertoldi@pd.infn.it
 * Created: Sun 24 Mar 2019
 */

// STL
#include <iostream>
#include <vector>

// ROOT
#include "TFile.h"
#include "TH3.h"
#include "TMath.h"

bool divide_maps(TH3* out, const TH3* h1, const TH3* h2);

int main(int argc, char** argv) {

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    std::string outfile = "gerda-larmap-merged.root";
    auto result1 = std::find(args.begin(), args.end(), "-o");
    auto result2 = std::find(args.begin(), args.end(), "--output");
    if (result1 != args.end()) {
        outfile = *(result1+1);
        args.erase(result1, result1+2);
    }
    else if (result2 != args.end()) {
        outfile = *(result2+1);
        args.erase(result2, result2+2);
    }

    std::cout << "INFO: merging together ";
    for (auto& f : args) std::cout << f << ' ';
    std::cout << std::endl;

    // for (auto& f : args) {
    //     std::cout << "\rINFO: checking file " << f << "..." << std::flush;

    //     TFile rf(f.c_str(), "read");

    //     if (rf.IsZombie()) {
    //         std::cerr << "ERROR: Invalid input file detected\n";
    //         return 1;
    //     }
    //     if (!rf.Get("LAr_vertex_map")) {
    //         std::cerr << "ERROR: Could not find \"LAr_vertex_map\" object\n";
    //         return 1;
    //     }
    //     if (!rf.Get("LAr_vertex_hits_map")) {
    //         std::cerr << "ERROR: Could not find \"LAr_vertex_hits_map\" object\n";
    //         return 1;
    //     }
    // }
    // std::cout << std::endl;

    std::cout << "INFO: adding everything together...\n";

    TFile rf(args[0].c_str(), "read");
    auto vertices      = dynamic_cast<TH3*>(rf.Get("LAr_vertex_map"));
    auto vertices_hits = dynamic_cast<TH3*>(rf.Get("LAr_vertex_hits_map"));

    for (size_t i = 1; i < args.size(); ++i) {
        std::cout << "\rINFO: Processing file " << args[i] << "..." << std::flush;
        TFile _rf(args[i].c_str(), "read");
        vertices->Add(dynamic_cast<TH3*>(_rf.Get("LAr_vertex_map")));
        vertices_hits->Add(dynamic_cast<TH3*>(_rf.Get("LAr_vertex_hits_map")));
    }
    std::cout << std::endl;

    std::cout << "INFO: dividing...\n";
    // save some time by copying only the structure
    auto& h = vertices_hits;
    TH3F prob_map(
        "LAr_prob_map", h->GetTitle(),
        h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax(),
        h->GetNbinsY(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax(),
        h->GetNbinsZ(), h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax()
    );

    divide_maps(&prob_map, vertices_hits, vertices);

    TFile fout(outfile.c_str(), "recreate");

    vertices->Write();
    vertices_hits->Write();
    prob_map.Write();

    std::cout << "INFO: " << outfile << " created.\n";

    return 0;
}

// NB: does not check for input consistency
bool divide_maps(TH3* out, const TH3* h1, const TH3* h2) {

    if (!out or !h1 or !h2) {
        std::cerr << "ERROR: passing a nullptr to divide_maps.";
        return false;
    }

    for (int i = 0; i < h1->GetNcells(); ++i) {
        double c1 = h1->GetBinContent(i);
        double c2 = h2->GetBinContent(i);

        if (c2 <= 0) {
            out->SetBinContent(i, -1); // -1 means zero statistics
            out->SetBinError(i, -1);
        }
        else {
            out->SetBinContent(i, c1/c2);
            // compute uncertainty according to Bernoulli statistics
            // (leads to non-sense values when c1 = 0 or c1 = c2)
            out->SetBinError(i, TMath::Sqrt((c1/c2)*(1-c1/c2)/c2));
        }
    }

    return true;
}
