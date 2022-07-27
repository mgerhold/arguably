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

TEST(Parser, FlagsOnly_WithDoubleDashes) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "t_arg", "">()
                          .flag<'a', "a_arg", "">()
                          .flag<'x', "x_arg", "">()
                          .flag<'z', "z_arg", "">()
                          .flag<'d', "d_arg", "">()
                          .flag<'b', "b_arg", "">()
                          .flag<'c', "c_arg", "">()
                          .create();

    const char* argv[] = { "a.out", "--a_arg", "--t_arg", "--b_arg", "--z_arg", "--x_arg", nullptr };

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

TEST(Parser, NamedWithoutSpace) {
    auto parser = Arguably::create_parser().flag<'t', "test", "">().named<'o', "output", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-tooutput_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, NamedWithSpace) {
    auto parser = Arguably::create_parser().flag<'t', "test", "">().named<'o', "output", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-to", "output_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, OptionallyNamed_UsingName_WithSpace) {
    auto parser = Arguably::create_parser().optionally_named<'i', "input", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-i", "input_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "input_filename");
}

TEST(Parser, OptionallyNamed_UsingName_WithoutSpace) {
    auto parser = Arguably::create_parser().optionally_named<'i', "input", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-iinput_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "input_filename");
}

TEST(Parser, OptionallyNamed_NotUsingName) {
    auto parser = Arguably::create_parser()
                          .optionally_named<'f', "first", "", std::string>("-")
                          .optionally_named<'s', "second", "", std::string>("-")
                          .flag<'o', "option", "">()
                          .optionally_named<'t', "third", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "first_arg", "-o", "second_arg", "third_arg", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'f'>(), "first_arg");
    EXPECT_EQ(parser.get<'s'>(), "second_arg");
    EXPECT_EQ(parser.get<'t'>(), "third_arg");
    EXPECT_TRUE(parser.get<'o'>());
}

TEST(Parser, OptionallyNamed_NotUsingName_FailOnTooManyArguments) {
    auto parser = Arguably::create_parser()
                          .optionally_named<'f', "first", "", std::string>("-")
                          .optionally_named<'s', "second", "", std::string>("-")
                          .flag<'o', "option", "">()
                          .optionally_named<'t', "third", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "first_arg", "-o", "second_arg", "third_arg", "fourth_arg", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<Arguably::Result::ExcessUnnamedArguments>());
}

TEST(Parser, OptionallyNamed_MixedUsingNameAndNotUsingName) {
    auto parser = Arguably::create_parser()
                          .optionally_named<'f', "first", "", std::string>("-")
                          .optionally_named<'s', "second", "", std::string>("-")
                          .flag<'o', "option", "">()
                          .optionally_named<'t', "third", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "-f", "first_arg", "-o", "-s", "second_arg", "third_arg", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'f'>(), "first_arg");
    EXPECT_EQ(parser.get<'s'>(), "second_arg");
    EXPECT_EQ(parser.get<'t'>(), "third_arg");
    EXPECT_TRUE(parser.get<'o'>());
}
