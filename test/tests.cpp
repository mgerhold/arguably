//
// Created by coder2k on 25.07.2022.
//

#include "arguably.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(Parser, FlagsOnly) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "t", "">()
                          .flag<'a', "a", "">()
                          .flag<'x', "x", "">()
                          .flag<'z', "z", "">()
                          .flag<'d', "d", "">()
                          .flag<'b', "b", "">()
                          .flag<'c', "c", "">()
                          .create();

    const char* argv[] = { "a.out", "-atb", "-z", "-x", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_TRUE(parser.get<'a'>());
    EXPECT_TRUE(parser.get<'b'>());
    EXPECT_FALSE(parser.get<'c'>());
    EXPECT_FALSE(parser.get<'d'>());
    EXPECT_TRUE(parser.get<'z'>());
    EXPECT_TRUE(parser.get<'x'>());
}

TEST(Parser, UnknownOption) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "t", "">()
                          .flag<'a', "a", "">()
                          .flag<'x', "x", "">()
                          .flag<'z', "z", "">()
                          .flag<'d', "d", "">()
                          .flag<'b', "b", "">()
                          .flag<'c', "c", "">()
                          .create();

    const char* argv[] = { "a.out", "-atyb", "-z", "-x", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<Arguably::Result::UnknownOption>());
}

TEST(Parser, ParameterWithoutSpace) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "test", "">()
                          .parameter<'o', "output", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "-tooutput_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, ParameterWithSpace) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "test", "">()
                          .parameter<'o', "output", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "-to", "output_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}
