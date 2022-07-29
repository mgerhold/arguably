![Unit Tests](https://github.com/mgerhold/arguably/actions/workflows/cmake.yml/badge.svg)
[![codecov](https://codecov.io/gh/mgerhold/arguably/branch/main/graph/badge.svg?token=2ARN577L42)](https://codecov.io/gh/mgerhold/arguably)
![Licence: MIT](https://img.shields.io/github/license/mgerhold/arguably)
![Discord Channel](https://img.shields.io/discord/834834066008309800?style=social)
![Twitch live status](https://img.shields.io/twitch/status/coder2k?style=social)

# Arguably
A simple-to-use command line argument parser written in modern C++.

## Usage
You can create a parser using the [*builder pattern*](https://en.wikipedia.org/wiki/Builder_pattern). There are three different kinds of arguments:
* flags (`true` or `false`),
* named parameters, and
* optionally named parameters.

Just concatenate the parameters you need while creating the parser and finish your call with `create()`. Please take a look at this example:
```cpp
auto parser = arguably::create_parser()
                    .optionally_named<'f', "first", "This is the first parameter", std::string>("-")
                    .named<'n', "named", "This is some named parameter", std::string>("-")
                    .optionally_named<'s', "second", "This is the second parameter", std::string>("-")
                    .flag<'o', "option", "This is some flag">()
                    .optionally_named<'t', "third", "This is the third parameter", std::string>("-")
                    .create();

parser.parse(argv);

if (parser) { // did the parsing succeed?
    std::cout << parser.get<'f'>() << "\n"; // prints the argument passed to 'f'
}
```
For flags, you just do:
```cpp
.flag<'f', "flag", "this is the description">()
```
For named parameters, you just do:
```cpp
.named<'n', "named", "this is the description", std::string>("default-value")
```
For optionally named parameters, you just do:
```cpp
.optionally_named<'n', "named", "this is the description", std::string>("default-value")
```

## Features

This library does compile-time-checks. The following code won't compile:
```cpp
auto parser = arguably::create_parser()
                    .flag<'a', "a_flag", "this is a description">()
                    .create();
parser.parse(argv);
std::cout << parser.get<'b'>() << "\n"; // <- won't compile, because 'b' is no valid command abbreviation
```
