//
// Created by coder2k on 25.07.2022.
//

#include "arguably.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(ParserBuilder, CreateParserBuilder) {
    /*auto parser = Arguably::create_parser()
                          .flag<'S', "bssembler", "only output the generated bssembler-code">()
                          .flag<'P', "test", "this is just a test">()
                          .parameter<'o', "output", "set the output filename", std::string>("-")
                          .argument<'i', "input", "set the input filename", std::string>("-")
                          .parameter<'n', "count", "set the count", int>(42)
                          .help<"this is the help text">()
                          .create();
    parser.print_help();
    //fmt::print("{}\n", parser.get_name<'S'>());
    fmt::print("{}\n", decltype(parser)::num_flags);
    fmt::print("{}\n", decltype(parser)::num_parameters);
    fmt::print("{}\n", decltype(parser)::num_arguments);

    fmt::print("indices:\n");
    fmt::print("{}\n", parser.index_of<'S'>());
    fmt::print("{}\n", parser.index_of<'P'>());
    fmt::print("{}\n", parser.index_of<'o'>());
    fmt::print("{}\n", parser.index_of<'i'>());

    fmt::print("\n");
    parser.print_default_values();

    fmt::print("value of 'S': {}\n", parser.get<'S'>());*/
}

TEST(Parser, FlagsTests) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "t", "">()
                          .flag<'a', "a", "">()
                          .flag<'x', "x", "">()
                          .flag<'z', "z", "">()
                          .flag<'d', "d", "">()
                          .flag<'b', "b", "">()
                          .flag<'c', "c", "">()
                          .create();

    const char* argv[] = { "a.out", "-iatb", "-z", "-x", nullptr };
    parser.parse(argv);

    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_TRUE(parser.get<'a'>());
    EXPECT_TRUE(parser.get<'b'>());
    EXPECT_FALSE(parser.get<'c'>());
    EXPECT_FALSE(parser.get<'d'>());
    EXPECT_TRUE(parser.get<'z'>());
    EXPECT_TRUE(parser.get<'x'>());
}

TEST(Parser, Parameter) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "test", "">()
                          .parameter<'o', "output", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "-tooutput_filename", nullptr };
    parser.parse(argv);

    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}

TEST(Parser, ParameterWithSpace) {
    auto parser = Arguably::create_parser()
                          .flag<'t', "test", "">()
                          .parameter<'o', "output", "", std::string>("-")
                          .create();
    const char* argv[] = { "a.out", "-to", "output_filename", nullptr };
    parser.parse(argv);

    EXPECT_TRUE(parser.get<'t'>());
    EXPECT_EQ(parser.get<'o'>(), "output_filename");
}
