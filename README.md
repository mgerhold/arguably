![Unit Tests](https://github.com/mgerhold/arguably/actions/workflows/cmake.yml/badge.svg)
[![codecov](https://codecov.io/gh/mgerhold/arguably/branch/main/graph/badge.svg?token=2ARN577L42)](https://codecov.io/gh/mgerhold/arguably)
![Licence: MIT](https://img.shields.io/github/license/mgerhold/arguably)
![Discord Channel](https://img.shields.io/discord/834834066008309800?style=social)
![Twitch live status](https://img.shields.io/twitch/status/coder2k?style=social)

# Arguably
A simple-to-use command line argument parser written in modern C++.

## Usage Example
```cpp
auto parser = Arguably::create_parser()
                    .optionally_named<'f', "first", "", std::string>("-")
                    .optionally_named<'s', "second", "", std::string>("-")
                    .flag<'o', "option", "">()
                    .optionally_named<'t', "third", "", std::string>("-")
                    .create();

parser.parse(argv);

std::cout << parser.get<'f'>() << "\n"; // prints the argument passed to 'f'
```
