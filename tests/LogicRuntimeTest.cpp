#include <iostream>
#include <filesystem>
#include <boost/algorithm/string/predicate.hpp>

#include "Logic.hpp"
#include "StateMachine.hpp"

#include <gtest/gtest.h>
#include <unistd.h>
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */

using namespace boost::filesystem;
namespace fs = std::filesystem;

TEST(Waveform, GenerateTest1) {
  struct Config cfg;

  cfg.Logic.push_back((struct ConfigLogic) {
			.Name = "all false",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		});
  cfg.Logic.push_back((struct ConfigLogic) {
			.Name = "Input stable",
			.AndSignalInputs = {{"o1", false, 0}},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out2", false}
		});
  SignalProvider sp(cfg);
  StateMachine sm(cfg, sp);

  Signal *a1 = sp.Find("a1");
  EXPECT_NE(a1, nullptr);
  Signal *a2 = sp.Find("a2");
  EXPECT_NE(a2, nullptr);
  Signal *o1 = sp.Find("o1");
  EXPECT_NE(o1, nullptr);
  Signal *out = sp.Find("out");
  EXPECT_NE(out, nullptr);
  Signal *out2 = sp.Find("out2");
  EXPECT_NE(out2, nullptr);
  sm.EvaluateState();

  path root = temp_directory_path() / unique_path();
  create_directories(root);

  for (int i = 0; i < 256; i++) {
    a1->SetLevel(i & 1);
    a2->SetLevel(i & 4);
    o1->SetLevel(i & 64);
    sm.EvaluateState();
    for (int j = 0; j < 10; j++) {
        sp.DumpSignals(root.string());
        usleep(1);
    }
    sm.Poll();
  }
  cout << "Written signals to " << root.string() << endl;
  int graphs = 0;
  for(auto& p: fs::recursive_directory_iterator(root.string())) {
    graphs++;
  }
  std::string cmd = "gnuplot -e \"unset ytics; set grid x; set xtics rotate 1000; set tmargin 0; set bmargin 0; set term svg size 4096 " + std::to_string(256 * graphs) + " dynamic ; set output 'output.svg'; set yrange [-0.5: 1.5]; set multiplot layout " + std::to_string(graphs) + ",1; ";
  for(auto& p: fs::recursive_directory_iterator(root.string())) {
    if (boost::algorithm::ends_with(std::string(p.path()), std::string(".txt"))) {
      cmd += "set ylabel '" + p.path().filename().string() + "';";
      cmd += "plot '" + string(p.path()) + "' using 1:2 with lines title '" + p.path().filename().string() + "'; ";
    }
  }
  cmd += "\"";
  system(cmd.c_str());
  remove_all(root);
}