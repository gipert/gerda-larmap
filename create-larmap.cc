/* USAGE: create-larmap [<settings.json>] [<MaGe-filelist>] [<output-filename>]
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

// GERDA sw
#include "KTreeFile.h"
#include "gerda-ada/T4SimConfig.h"
#include "gerda-ada/T4SimHandler.h"

#include "ProgressBar.h"

int main(int argc, char** argv) {

    // load JSON settings
    auto configname = argc > 1 ? argv[1] : "settings/prob-map-settings.json";
    katrin::KTree config;
    try { katrin::KTreeFile(configname).Read(config); }
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

    // perform the division
    auto prob_map = dynamic_cast<TH3D*>(vertex_hits_map.Clone("LAr_prob_map"));
    prob_map->Divide(&vertex_map);

    // write to disk
    auto filename = argc > 3 ? argv[3] : config["output"].Or("gerda-larmap.root").As<std::string>();
    TFile fout(filename.c_str(), "recreate");
    vertex_map.Write();
    vertex_hits_map.Write();
    prob_map->Write();

    std::cout << "INFO: file '" + filename + "' created\n";

    return 0;
}
