/* create-larmap.cc
 *
 * Author: Luigi Pertoldi - pertoldi@pd.infn.it
 * Created: Sun 24 Mar 2019
 *
 * USAGE: create-larmap [<settings-dir>] [<MaGe-filelist>] [<output-filename>]
 *
 */
#include <iostream>

// ROOT
#include "TChain.h"
#include "TH3D.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TMath.h"

// GERDA sw
#include "KTreeFile.h"
#include "gerda-ada/T4SimConfig.h"
#include "gerda-ada/T4SimHandler.h"

#include "ProgressBar.h"

bool divide_maps(TH3D *h1, const TH3D *h2);

int main(int argc, char** argv) {

    // load JSON settings
    auto configdir = argc > 1 ? std::string(argv[1]) : "settings";
    katrin::KTree config;
    try { katrin::KTreeFile(configdir + "/prob-map-settings.json").Read(config); }
    catch (katrin::KException &e) { std::cerr << e.what() << std::endl; }

    // open MaGe files
    auto filelist = argc > 2 ? argv[2] : config["files"].As<std::string>();
    std::cout << "INFO: opening MaGe files: " << filelist << std::endl;
    TChain chain("fTree");
    chain.Add(filelist.c_str());

    // initialize probability map
    auto& s = config;
    TH3D vertex_map(
        "LAr_vertex_map", "LAr_vertex_map",
        s["bins"][0].As<int>(), s["range-cm"][0][0].As<double>(), s["range-cm"][0][1].As<double>(),
        s["bins"][1].As<int>(), s["range-cm"][1][0].As<double>(), s["range-cm"][1][1].As<double>(),
        s["bins"][2].As<int>(), s["range-cm"][2][0].As<double>(), s["range-cm"][2][1].As<double>()
    );

    TH3D vertex_hits_map(
        "LAr_vertex_hits_map", "LAr_vertex_hits_map",
        s["bins"][0].As<int>(), s["range-cm"][0][0].As<double>(), s["range-cm"][0][1].As<double>(),
        s["bins"][1].As<int>(), s["range-cm"][1][0].As<double>(), s["range-cm"][1][1].As<double>(),
        s["bins"][2].As<int>(), s["range-cm"][2][0].As<double>(), s["range-cm"][2][1].As<double>()
    );

    // set up the Tier4izer
    gada::T4SimConfig simConfig;
    simConfig.LoadMapping(configdir + "/mapping-spmMerged.json");
    simConfig.LoadLArSettings(configdir + "/lar-settings-55cm_0.50cov.json");

    gada::T4SimHandler simHandler(&chain, &simConfig);
    simHandler.SetBranchAddresses();

    // set up the TTree reader object
    TTreeReader reader;
    TTreeReaderArray<float> x(reader, "vertex_xpos");
    TTreeReaderArray<float> y(reader, "vertex_ypos");
    TTreeReaderArray<float> z(reader, "vertex_zpos");

    reader.SetTree(&chain);

    ProgressBar bar(chain.GetEntries());
    chain.LoadTree(0);

    std::cout << "INFO: looping over events ";
    while (reader.Next()) {
        vertex_map.Fill(x[0], y[0], z[0]); // vertex_totnum should be always == 1

        // check if the photon reaches the LAr instrumentation
        simHandler.GetEntry(reader.GetCurrentEntry());
        if (simHandler.GetLArEvent()->GetIsVetoed()) vertex_hits_map.Fill(x[0], y[0], z[0]);

        bar.Update();
    }
    std::cout << std::endl;

    // perform the division
    auto prob_map = dynamic_cast<TH3D*>(vertex_hits_map.Clone("LAr_prob_map"));
    divide_maps(prob_map, &vertex_map);

    // write to disk
    auto filename = argc > 3 ? argv[3] : config["output"].Or("gerda-larmap.root").As<std::string>();
    TFile fout(filename.c_str(), "recreate");
    vertex_map.Write();
    vertex_hits_map.Write();
    prob_map->Write();

    std::cout << "INFO: file '" + filename + "' created\n";

    return 0;
}

// NB: does not check for input consistency
bool divide_maps(TH3D *h1, const TH3D *h2) {

    if (!h1 or !h2) {
        std::cerr << "ERROR: passing a nullptr to divide_maps.";
        return false;
    }

    for (int i = 0; i < h1->GetNcells(); ++i) {
        double c1 = h1->GetBinContent(i);
        double c2 = h2->GetBinContent(i);

        if (c2 == 0) {
            h1->SetBinContent(i, -1); // -1 means zero statistics
            h1->SetBinError(i, -1);
        }
        else {
            h1->SetBinContent(i, c1/c2);
            // compute uncertainty according to Bernoulli statistics
            // (leads to non-sense values when c1 = 0 or c1 = c2)
            h1->SetBinError(i, TMath::Sqrt((c1/c2)*(1-c1/c2)/c2));
        }
    }

    return true;
}
