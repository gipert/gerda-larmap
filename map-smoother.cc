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
#include <getopt.h>

// ROOT
#include "TFile.h"
#include "TH3F.h"

#include "ProgressBar.h"

int main(int argc, char** argv) {

    std::string progname(argv[0]);

    auto usage = [&]() {
        std::cerr << "USAGE: " << progname << " [-h|--help] [--width|-w <val>] [--output|-o out.root] file.root[:objname]\n";
    };

    const char* const short_opts = "w:o:h";
    const option long_opts[] = {
        { "width",   required_argument, nullptr, 'w' },
        { "output",  required_argument, nullptr, 'o' },
        { "help",    no_argument,       nullptr, 'h' },
        { nullptr,   no_argument,       nullptr, 0   }
    };

    int ws = 3; // window size
    std::string outname = "";

    int opt = 0;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'w':
                ws = strtol(optarg, (char**)NULL, 10);
                break;
            case 'o':
                outname = std::string(optarg);
                break;
            case 'h': // -h or --help
            case '?': // Unrecognized option
            default:
                usage();
                return 1;
        }
    }

    // extra arguments
    std::vector<std::string> args;
    for(; optind < argc; optind++){
        args.emplace_back(argv[optind]);
    }

    if (args.empty()) { usage(); return 1; }

    if (ws <= 1) {
        std::cout << "ERROR: invalid window size\n";
        return 1;
    }

    std::string filename, objname;
    if (args[0].find(':') != std::string::npos) {
        filename = args[0].substr(0, args[0].find_first_of(':'));
        objname = args[0].substr(args[0].find_first_of(':')+1, std::string::npos);
    }
    else {
        filename = args[0];
        objname = "LAr_prob_map";
    }
    std::cout << "INFO: smoothing " << filename << "...\n";
    TFile f(filename.c_str(), "read");

    auto h = dynamic_cast<TH3*>(f.Get(objname.c_str()));
    if (!h) {
        std::cout << "ERROR: " + objname + " object not found\n";
        return 1;
    }
    h->SetName("LAr_prob_map_raw");

    // save some time by copying only the structure
    TH3F h_sm(
        "LAr_prob_map", h->GetTitle(),
        h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax(),
        h->GetNbinsY(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax(),
        h->GetNbinsZ(), h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax()
    );

    double _sum;
    int _terms;

    int nb_x = h->GetNbinsX();
    int nb_y = h->GetNbinsY();
    int nb_z = h->GetNbinsZ();

    std::cout << "INFO: using smoothing window size = " << ws << std::endl;

    ProgressBar bar(nb_x*nb_y*nb_z);
    std::cout << "INFO: ";

    // loop over the histogram cells (no over/underflow)
    for (int i = 1; i <= nb_x; ++i) {
        for (int j = 1; j <= nb_y; ++j) {
            for (int k = 1; k <= nb_z; ++k) {
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
                    if (_i < 1 or _i > nb_x) continue;
                    for (int _j = j-ws; _j <= j+ws; ++_j) {
                        // check if we are out-of-bound
                        if (_j < 1 or _j > nb_y) continue;
                        for (int _k = k-ws; _k <= k+ws; ++_k) {
                            // check if we are out-of-bound
                            if (_k < 1 or _k > nb_z) continue;
                            // check if the bin contains LAr
                            if (h->GetBinContent(_i, _j, _k) >= 0) {
                                _sum += h->GetBinContent(_i, _j, _k);
                                _terms++;
                            }
                        }
                    }
                }
                h_sm.SetBinContent(i, j, k, _terms > 0 ? _sum/_terms : -1);
                bar.Update();
            }
        }
    }
    std::cout << std::endl;

    if (outname.empty()) {
        outname = args[0].substr(args[0].find_last_of('/')+1, args[0].find_last_of('.')-args[0].find_last_of('/')-1)
            + "-smoothed-w" + std::to_string(ws) + ".root";
    }

    filename.erase(std::find(filename.begin(), filename.end(), '.'), filename.end());
    TFile fout(outname.c_str(), "RECREATE");
    h_sm.Write("LAr_prob_map");

    std::cout << "INFO: " + outname + " created" << std::endl;

    return 0;
}
