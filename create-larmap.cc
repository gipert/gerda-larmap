#include <iostream>

// ROOT
#include "TChain.h"
#include "TH3D.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

// GERDA sw
#include "KTreeFile.h"
#include "gerda-ada/T4SimConfig.h"
#include "gerda-ada/T4SimHandler.h"

#include "ProgressBar.h"

int main() {

    // load JSON settings
    katrin::KTree config;
    try { katrin::KTreeFile("settings/map-settings.json").Read(config); }
    catch (katrin::KException &e) { std::cerr << e.what() << std::endl; }

    std::cout << "INFO: opening MaGe files\n";
    // open MaGe files
    TChain chain("fTree");
    chain.Add(config["files"].As<std::string>().c_str());

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
    simConfig.LoadMapping("settings/mapping-spmMerged.json");
    simConfig.LoadLArSettings("settings/lar-settings-55cm_0.50cov.json");

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

    // write to disk
    TFile fout("gerda-larmap.root", "recreate");
    vertex_map.Write();
    vertex_hits_map.Write();

    std::cout << "INFO: file 'gerda-larmap.root' created\n";

    return 0;
}
