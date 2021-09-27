#include <iostream>
#include "StateMachine.hpp"
#include "Config.hpp"

#include <gtest/gtest.h>
 
struct signalstate {
	string name;
	bool value;
};

static struct Config init(void) {
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
	cfg.Inputs.push_back((struct ConfigInput) {
		.Name = "GPIO_A1",
		.GpioChipName = "",
		.SignalName = "a1",
		.ActiveLow = false,
		.Description = "",
		.InputType = INPUT_TYPE_NULL,
	});
	cfg.Inputs.push_back((struct ConfigInput) {
		.Name = "GPIO_A2",
		.GpioChipName = "",
		.SignalName = "a2",
		.ActiveLow = false,
		.Description = "",
		.InputType = INPUT_TYPE_NULL,
	});
	cfg.Inputs.push_back((struct ConfigInput) {
		.Name = "GPIO_O1",
		.GpioChipName = "",
		.SignalName = "o1",
		.ActiveLow = false,
		.Description = "",
		.InputType = INPUT_TYPE_NULL,
	});
	cfg.Outputs.push_back((struct ConfigOutput) {
		.Name = "GPIO_OUT",
		.GpioChipName = "",
		.SignalName = "out",
		.ActiveLow = false,
		.Description = "",
		.OutputType = OUTPUT_TYPE_NULL,
	});

	return cfg;
}

TEST(Logic, StateChangeAfterEvaluateState) {
  boost::asio::io_context io;
  struct Config cfg = init();
  SignalProvider sp;
  StateMachine sm(cfg, sp);

  sm.EvaluateState();

  Signal *out = sp.Find("out");
  Signal *a1 = sp.Find("a1");
  Signal *a2 = sp.Find("a2");
  Signal *o1 = sp.Find("o1");

  EXPECT_EQ(out->GetLevel(), false);
  a1->SetLevel(true);
  a2->SetLevel(true);
  o1->SetLevel(true);

  EXPECT_EQ(out->GetLevel(), false);
  sm.EvaluateState();
  EXPECT_EQ(out->GetLevel(), true);
}

TEST(Logic, StateChangeAfterIO) {
  boost::asio::io_context io;
  struct Config cfg = init();
  SignalProvider sp;
  StateMachine sm(cfg, sp);
  StateMachineTester smt(&sm);

  sm.EvaluateState();

  std::vector<NullOutput *> vecOut = smt.GetNullOutputs();
  std::vector<NullInput *> vecIn = smt.GetNullInputs();

  EXPECT_EQ(vecOut[0]->GetLevel(), false);

  for (auto it : vecIn) {
    it->SetLevel(true);
  }

  EXPECT_EQ(vecOut[0]->GetLevel(), false);
  sm.EvaluateState();
  EXPECT_EQ(vecOut[0]->GetLevel(), true);
}