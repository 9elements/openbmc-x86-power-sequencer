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
			.OrSignalInputs = {{"o1", false, 30}},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"o1_stable", false}
		});
	
  path root = temp_directory_path() / unique_path();
  create_directories(root);

  {
	SignalProvider sp(cfg, root.string());
	StateMachine sm(cfg, sp);

	Signal *a1 = sp.Find("a1");
	EXPECT_NE(a1, nullptr);
	Signal *a2 = sp.Find("a2");
	EXPECT_NE(a2, nullptr);
	Signal *o1 = sp.Find("o1");
	EXPECT_NE(o1, nullptr);
	Signal *out = sp.Find("out");
	EXPECT_NE(out, nullptr);
	Signal *o1_stable = sp.Find("o1_stable");
	EXPECT_NE(o1_stable, nullptr);
	sm.EvaluateState();


	for (int i = 0; i < 256; i++) {
		a1->SetLevel(i & 1);
		a2->SetLevel(i & 4);
		o1->SetLevel(i & 64);
		boost::chrono::steady_clock::time_point start = boost::chrono::steady_clock::now();

		sm.Poll();
		sm.EvaluateState();
		sp.DumpSignals();

		boost::chrono::nanoseconds ns;

		while (ns.count() < 10000) {
			ns = boost::chrono::steady_clock::now() - start;
		}

		sp.DumpSignals();

	}
  }
  
  //remove_all(root);
}