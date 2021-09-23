#include <iostream>
#include "Logic.hpp"

#include <gtest/gtest.h>
 
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
			"all false",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			false,
			false,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
			"invert first gate",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			false,
			true,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
			"or input true",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			false,
			false,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
			"or input true, output active low",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			false,
			true,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
			"all false, output active low",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			false,
			false,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
		},
		true,
		{
			{"a1", true},
			{"a2", true},
			{"o1", true},
			{},
		}
	},
	{
		{
			"and true and input of or",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			true,
			false,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
			"and input of or, or is true",
			{{"a1", false, 0}, {"a2", false, 0}},
			{{"o1", false, 0},},
			true,
			false,
			0,
			{"out", false}
			// Out configures which Signal is driven by this logic block
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
  SignalProvider sp;
  boost::asio::io_context io;
  struct testcase *tc;

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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}