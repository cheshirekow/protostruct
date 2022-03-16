// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <vector>

#include <gtest/gtest.h>

#include "tangent/tjson/ostream.h"
#include "tangent/tjson/tjson.h"

TEST(OStream, SimpleObjectFormatsAsExpected) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::OBJECT};
      ostrm << "fieldA" << 123;
      ostrm << "fieldB" << 12.5;
      ostrm << "fieldC" << true;
      ostrm << "fieldD"
            << "hello world!";
      ostrm << "fieldE" << std::string{"hello world!"};
      ostrm << "fieldF" << nullptr;
      ostrm << "fieldG" << 0;
    }
    return strm.str();
  }();

  EXPECT_EQ(result, R"({
    "fieldA": 123,
    "fieldB": 12.5,
    "fieldC": true,
    "fieldD": "hello world!",
    "fieldE": "hello world!",
    "fieldF": null,
    "fieldG": 0
}
)");
}

TEST(OStream, SimpleListFormatsAsExpected) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::LIST};
      ostrm << 123;
      ostrm << 12.5;
      ostrm << true;
    }
    return strm.str();
  }();

  EXPECT_EQ(result, R"([
    123,
    12.5,
    true
]
)");
}

TEST(OStream, EmptyObjectIsCompact) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::OBJECT};
    }
    return strm.str();
  }();

  EXPECT_EQ(result, "{}\n");
}

TEST(OStream, EmptyListIsCompact) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::LIST};
    }
    return strm.str();
  }();

  EXPECT_EQ(result, "[]\n");
}

TEST(OStream, NestedObjectIncreasesIndent) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::OBJECT};
      ostrm << "fieldA";
      {
        tjson::Guard guard{&ostrm, tjson::OBJECT};
        ostrm << "fieldB" << 1;
        ostrm << "fieldC" << 2;
        ostrm << "fieldD" << 3;
      }
      ostrm << "fieldE"
            << "hello!";
    }
    return strm.str();
  }();

  EXPECT_EQ(result, R"({
    "fieldA": {
        "fieldB": 1,
        "fieldC": 2,
        "fieldD": 3
    },
    "fieldE": "hello!"
}
)");
}

TEST(OStream, StringFieldsIncrementItemCount) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::OBJECT};
      ostrm << "fieldA"
            << "value A";
      ostrm << "fieldB"
            << "value B";
      ostrm << "fieldC"
            << "value C";
    }
    return strm.str();
  }();

  EXPECT_EQ(result, R"({
    "fieldA": "value A",
    "fieldB": "value B",
    "fieldC": "value C"
}
)");
}

TEST(OStream, ObjectsAndListsIncreaseItemCount) {
  const std::string result = []() {
    std::stringstream strm{};

    /* object scope */ {
      tjson::OStream ostrm{&strm, tjson_DefaultOpts};
      tjson::Guard guard{&ostrm, tjson::LIST};
      { tjson::Guard guard{&ostrm, tjson::OBJECT}; }
      { tjson::Guard guard{&ostrm, tjson::LIST}; }
      { tjson::Guard guard{&ostrm, tjson::OBJECT}; }
      { tjson::Guard guard{&ostrm, tjson::OBJECT}; }
    }
    return strm.str();
  }();

  EXPECT_EQ(result, R"([
    {},
    [],
    {},
    {}
]
)");
}
