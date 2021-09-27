#include <iostream>
#include "Logic.hpp"
#include "StateMachine.hpp"

#include <gtest/gtest.h>
#include <unistd.h>

struct signalstate {
	string name;
	bool value;
};

struct testcase {
	struct ConfigLogic cfg;
	bool expectedResult;
	struct signalstate inputStates[4];
} testcases[] = {
	{
		{
			.Name = "all false",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		},
		false,
		{
			{"a1", false},
			{"a2", false},
			{"o1", false},
			{},
		}
	},
	{
		{
			.Name = "all false, invert first gate",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = true,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		},
		false,
		{
			{"a1", false},
			{"a2", false},
			{"o1", false},
			{},
		}
	},
	{
		{
			.Name = "or input true",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		},
		false,
		{
			{"a1", false},
			{"a2", false},
			{"o1", true},
			{},
		}
	},
	{
		{
			.Name = "or input true, output active low",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", true}
		},
		true,
		{
			{"a1", false},
			{"a2", false},
			{"o1", true},
			{},
		}
		
	},
	{
		{
			.Name = "all false, output active low",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", true}
		},
		true,
		{
			{"a1", false},
			{"a2", false},
			{"o1", false},
			{},
		}
	},
	{
		{
			.Name = "and true and input of or",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = true,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		},
		true,
		{
			{"a1", true},
			{"a2", true},
			{"o1", false},
			{},
		}
	},
	{
		{
			.Name = "and input of or, or true",
			.AndSignalInputs = {{"a1", false, 0}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = true,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		},
		true,
		{
			{"a1", false},
			{"a2", false},
			{"o1", true},
			{},
		}
	},
	{}
};

TEST(Logic, LUT) {
  boost::asio::io_context io;
  struct testcase *tc;
  struct Config cfg;
  SignalProvider sp(cfg);

  for (int i = 0; testcases[i].cfg.Name != ""; i++) {
    tc = &testcases[i];

    Logic *l = new Logic(io, sp, &tc->cfg);
    for (int states = 0; tc->inputStates[states].name != ""; states ++) {
      Signal *s = sp.Find(tc->inputStates[states].name);
      EXPECT_NE(s, nullptr);
      std::cout << "set input " << tc->inputStates[states].name << " to " << tc->inputStates[states].value << std::endl;
      s->SetLevel(tc->inputStates[states].value);
    }
    l->Update();
    Signal *out = sp.Find(tc->cfg.Out.SignalName);
    EXPECT_NE(out, nullptr);
    EXPECT_EQ(out->GetLevel(), tc->expectedResult);
    delete l;
  }
}

TEST(Logic, TestInputStable) {
  boost::asio::io_context io;
  struct Config cfg;

  cfg.Logic.push_back((struct ConfigLogic) {
			.Name = "all false",
			.AndSignalInputs = {{"a1", false, 1000}, {"a2", false, 0}},
			.OrSignalInputs = {{"o1", false, 0},},
			.AndThenOr = false,
			.InvertFirstGate = false,
			.DelayOutputUsec = 0,
			.Out = {"out", false}
		});
  SignalProvider sp(cfg);
  StateMachine sm(cfg, sp);

  Logic *l = new Logic(io, sp, &cfg.Logic[0]);
  Signal *a1 = sp.Find("a1");
  EXPECT_NE(a1, nullptr);
  Signal *a2 = sp.Find("a2");
  EXPECT_NE(a2, nullptr);
  a2->SetLevel(true);
  Signal *o1 = sp.Find("o1");
  EXPECT_NE(o1, nullptr);
  o1->SetLevel(true);
  Signal *out = sp.Find("out");
  EXPECT_NE(out, nullptr);

  boost::chrono::steady_clock::time_point start = boost::chrono::steady_clock::now();

  for (int i = 0; i < 1000; i++) {
    boost::chrono::nanoseconds ns;

    a1->SetLevel(a1->GetLevel() ^ 1);
    while (ns.count() < 1000000) {
      ns = boost::chrono::steady_clock::now() - start;
    }
    start = boost::chrono::steady_clock::now();

    sm.EvaluateState();
    EXPECT_EQ(out->GetLevel(), false);
  }
  a1->SetLevel(true);
  usleep(2000);
  sm.EvaluateState();
  EXPECT_EQ(out->GetLevel(), true);
  a1->SetLevel(false);
  usleep(2000);
  sm.EvaluateState();
  EXPECT_EQ(out->GetLevel(), false);

  delete l;
}