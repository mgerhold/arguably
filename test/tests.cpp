//
// Created by coder2k on 25.07.2022.
//

#include <arguably.hpp>
#include <gtest/gtest.h>
#include <string>

TEST(Parser, FlagsOnly) {
    auto parser = arguably::create_parser()
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
    auto parser = arguably::create_parser()
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
    auto parser = arguably::create_parser()
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
    EXPECT_TRUE(parser.result_is<arguably::result::UnknownOption>());
}

TEST(Parser, NamedWithoutSpace) {
    auto parser = arguably::create_parser().flag<'t', "test", "">().named<'o', "output", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-tooutput_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, NamedWithSpace) {
    auto parser = arguably::create_parser().flag<'t', "test", "">().named<'o', "output", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-to", "output_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, OptionallyNamed_UsingName_WithSpace) {
    auto parser = arguably::create_parser().optionally_named<'i', "input", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-i", "input_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "input_filename");
}

TEST(Parser, OptionallyNamed_UsingName_WithoutSpace) {
    auto parser = arguably::create_parser().optionally_named<'i', "input", "", std::string>("-").create();
    const char* argv[] = { "a.out", "-iinput_filename", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "input_filename");
}

TEST(Parser, OptionallyNamed_NotUsingName) {
    auto parser = arguably::create_parser()
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
    auto parser = arguably::create_parser()
                          .optionally_named<'f', "first", "", std::string>("-")
                          .optionally_named<'s', "second", "", std::string>("-")
                          .flag<'o', "option", "">()
                          .optionally_named<'t', "third", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "first_arg", "-o", "second_arg", "third_arg", "fourth_arg", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::ExcessUnnamedArguments>());
}

TEST(Parser, OptionallyNamed_MixedUsingNameAndNotUsingName) {
    auto parser = arguably::create_parser()
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

TEST(Parser, SingleNamedParameter_UsingSpace) {
    auto parser = arguably::create_parser().named<'i', "input", "specify input file", std::string>("-").create();

    const char* argv[] = { "backseat.exe", "--input", "my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

TEST(Parser, OnlyUnnamedParametersAfterFreeStandingDoubleDash) {
    auto parser = arguably::create_parser()
                          .flag<'i', "some_flag", "">()
                          .optionally_named<'a', "arg", "", std::string>("-")
                          .create();
    const char* argv[] = { "rm", "--", "-irgendeinidiothateinendateinamenmiteinembindestrichangelegt", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_FALSE(parser.get<'i'>());
    EXPECT_EQ(parser.get<'a'>(), "-irgendeinidiothateinendateinamenmiteinembindestrichangelegt");
}

TEST(Parser, SingleDashAsUnnamedArgument) {
    auto parser = arguably::create_parser()
                          .flag<'i', "some_flag", "">()
                          .optionally_named<'a', "arg", "", std::string>("some_default")
                          .create();

    const char* argv[] = { "rm", "-", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_FALSE(parser.get<'i'>());
    EXPECT_EQ(parser.get<'a'>(), "-");
}

TEST(Parser, NamedArgument_NoSpace) {
    auto parser = arguably::create_parser().named<'i', "input", "", std::string>("-").create();

    const char* argv[] = { "backseat.exe", "-imy_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

TEST(Parser, NamedArgument_WithSpace) {
    auto parser = arguably::create_parser().named<'i', "input", "", std::string>("-").create();

    const char* argv[] = { "backseat.exe", "-i", "my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

TEST(Parser, NamedArgument_WithName_WithEquals) {
    auto parser = arguably::create_parser().named<'i', "input", "", std::string>("-").create();

    const char* argv[] = { "backseat.exe", "--input=my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

TEST(Parser, OptionallyNamedArgumentWithFollowingFlag) {
    auto parser = arguably::create_parser()
                          .flag<'S', "HugeS", "">()
                          .optionally_named<'i', "input", "", std::string>("-")
                          .create();

    const char* argv[] = { "backseat.exe", "my_code.bs", "-S", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'S'>());
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

// backseat.exe -- my_code.bs
TEST(Parser, OptionallyNamedArgumentAfterFreestandingDoubleDash) {
    auto parser = arguably::create_parser()
                          .flag<'S', "HugeS", "">()
                          .flag<'m', "mothership", "">()
                          .optionally_named<'i', "input", "", std::string>("-")
                          .create();

    const char* argv[] = { "backseat.exe", "--", "my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_FALSE(parser.get<'S'>());
    EXPECT_FALSE(parser.get<'m'>());
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

// backseat.exe -Si my_code.bs
TEST(Parser, FlagWithFollowingOptionallyNamedParameter) {
    auto parser = arguably::create_parser()
                          .flag<'S', "HugeS", "">()
                          .flag<'m', "mothership", "">()
                          .optionally_named<'i', "input", "", std::string>("-")
                          .create();

    const char* argv[] = { "backseat.exe", "-Si", "my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'S'>());
    EXPECT_FALSE(parser.get<'m'>());
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

// backseat.exe -Simy_code.bs
TEST(Parser, FlagWithFollowingOptionallyNamedParameter_NoSpace) {
    auto parser = arguably::create_parser()
                          .flag<'S', "HugeS", "">()
                          .flag<'m', "mothership", "">()
                          .optionally_named<'i', "input", "", std::string>("-")
                          .create();

    const char* argv[] = { "backseat.exe", "-Simy_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_TRUE(parser.get<'S'>());
    EXPECT_FALSE(parser.get<'m'>());
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

// backseat.exe -akjfi filename.bs
TEST(Parser, MultipleFlagsWithFollowingOptionallyNamedParameter_WithSpace) {
    auto parser = arguably::create_parser()
                          .flag<'S', "HugeS", "">()
                          .flag<'m', "mothership", "">()
                          .optionally_named<'i', "input", "", std::string>("-")
                          .flag<'a', "a_arg", "">()
                          .flag<'k', "k_arg", "">()
                          .flag<'j', "j_arg", "">()
                          .flag<'f', "f_arg", "">()
                          .create();

    const char* argv[] = { "backseat.exe", "-akjfi", "my_code.bs", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    ASSERT_TRUE(parser);
    EXPECT_FALSE(parser.get<'S'>());
    EXPECT_FALSE(parser.get<'m'>());
    EXPECT_TRUE(parser.get<'a'>());
    EXPECT_TRUE(parser.get<'k'>());
    EXPECT_TRUE(parser.get<'j'>());
    EXPECT_TRUE(parser.get<'f'>());
    EXPECT_EQ(parser.get<'i'>(), "my_code.bs");
}

TEST(Parser, PrintHelp) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_arg", "This is the description of a">()
                          .help<"This is the best program ever!">()
                          .create();
    std::stringstream stream;
    parser.print_help(stream);
    EXPECT_EQ(
            stream.str(),
            "This is the best program ever!\n"
            "-h, --help   show help\n"
            "-a, --a_arg  This is the description of a\n"
    );
}

TEST(Parser, UseOfUnknownDoubleDashArgumentWithEquals) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_arg", "This is the description of a">()
                          .help<"This is the best program ever!">()
                          .create();

    const char* argv[] = { "backseat.exe", "--Pedder__=something", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::UnknownOption>());
}

TEST(Parser, DoubleDashArgumentWithEqualsIsFlag) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .help<"This is the best program ever!">()
                          .create();

    const char* argv[] = { "backseat.exe", "--a_flag=bla", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::CannotSetValueOfFlag>());
}

TEST(Parser, DoubleDashArgumentWithEmptyValue) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();

    const char* argv[] = { "backseat.exe", "--name=", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::MissingArgument>());
}

TEST(Parser, UseOfUnknownDoubleDashArgumentWithoutEquals) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_arg", "This is the description of a">()
                          .help<"This is the best program ever!">()
                          .create();

    const char* argv[] = { "backseat.exe", "--Pedder__", "something", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::UnknownOption>());
}

TEST(Parser, UseOfDoubleDashArgumentWithoutEqualsWithMissingValue) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();

    const char* argv[] = { "backseat.exe", "--name", nullptr };

    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::MissingArgument>());
}

TEST(Parser, OnlySingleDashAsArgument) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .optionally_named<'o', "optionally", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "-", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'o'>(), "-");
}

TEST(Parser, OnlySingleDashAsArgument_ParserHasNoOptionallyNamedArguments) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "-", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::ExcessUnnamedArguments>());
}

TEST(Parser, OnlyDoubleDashAsArgument) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "--", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
}

TEST(Parser, TwoDoubleDashesAsArguments_ParserHasNoOptionallyNamedArguments) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "--", "--", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::ExcessUnnamedArguments>());
}

TEST(Parser, TwoDoubleDashesAsArguments_ParserHasOneOptionallyNamedArgument) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .optionally_named<'o', "optionally", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "--", "--", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'o'>(), "--");
}

TEST(Parser, TryingToParseTwice) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .optionally_named<'o', "optionally", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "--", "--", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'o'>(), "--");

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::CannotParseAgain>());
}

TEST(Parser, MissingValueAfterSingleDashNamedArgumentAbbreviation) {
    auto parser = arguably::create_parser()
                          .flag<'a', "a_flag", "This is the description of a">()
                          .named<'n', "name", "", std::string>("-")
                          .optionally_named<'o', "optionally", "", std::string>("-")
                          .help<"This is the best program ever!">()
                          .create();
    const char* argv[] = { "backseat.exe", "-an", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);

    EXPECT_TRUE(parser.result_is<arguably::result::MissingArgument>());
}

TEST(Parser, IntegerArgument) {
    auto parser = arguably::create_parser().named<'i', "integer", "this is the description", int>(42).create();
    const char* argv[] = { "backseat.exe", "-i43", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), 43);
}

TEST(Parser, IntegerArgument_InvalidArgument) {
    auto parser = arguably::create_parser().named<'i', "integer", "this is the description", int>(42).create();
    const char* argv[] = { "backseat.exe", "-iabc", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_FALSE(parser);
    EXPECT_TRUE(parser.result_is<arguably::result::ArgumentTypeMismatch>());
}

TEST(Parser, QueryIfParameterWasProvided) {
    auto parser = arguably::create_parser()
                          .named<'i', "integer", "this is the description", int>(42)
                          .named<'n', "number", "this is the description", int>(42)
                          .create();
    const char* argv[] = { "backseat.exe", "-i42", nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), 42);
    EXPECT_EQ(parser.get<'n'>(), 42);
    EXPECT_TRUE(parser.was_provided<'i'>());
    EXPECT_FALSE(parser.was_provided<'n'>());
}

TEST(Parser, UseNonConstArgumentVector) {
    auto parser = arguably::create_parser().named<'i', "integer", "this is the description", int>(42).create();
    char first[] = "backseat.exe";
    char second[] = "-i43";
    char* argv[] = { first, second, nullptr };
    EXPECT_FALSE(parser);

    parser.parse(argv);

    EXPECT_TRUE(parser);
    EXPECT_EQ(parser.get<'i'>(), 43);
}

TEST(Parser, InfoText) {
    auto parser = arguably::create_parser().info<"info">().help<"help">().create();
    std::stringstream stream;
    parser.print_help(stream);
    EXPECT_EQ(
            stream.str(),
            "help\n"
            "-h, --help  show help\n"
    );

    stream = std::stringstream {};
    parser.print_info(stream);
    EXPECT_EQ(
            stream.str(),
            "info\n"
    );
}

TEST(Parser, ParserHasHelpFlag_IsTrueIfSet) {
    auto parser = arguably::create_parser().create();
    const char* argv[] = { "backseat.exe", "-h", nullptr };

    EXPECT_FALSE(parser);
    parser.parse(argv);
    EXPECT_TRUE(parser);
    EXPECT_TRUE(parser.get<'h'>());
}

TEST(Parser, ParserHasHelpFlag_IsFalseIfNotSet) {
    auto parser = arguably::create_parser().create();
    const char* argv[] = { "backseat.exe", nullptr };

    EXPECT_FALSE(parser);
    parser.parse(argv);
    EXPECT_TRUE(parser);
    EXPECT_FALSE(parser.get<'h'>());
}
