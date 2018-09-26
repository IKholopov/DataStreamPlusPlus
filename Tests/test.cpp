#include <Recurrent.h>
#include <iostream>
#include <gtest/gtest.h>

TEST( SelectTest, ApplySelectInt ) {
    Container<int> t({1,2,3});
    DataStream other = &t;
    auto x2 = t.Select<int>([](int x) { return 2*x; });
    auto x2v = x2.ToVector();
    {
        std::vector<int> expected({2,4,6});
        auto it = expected.begin();
        for(auto val: x2v) {
            ASSERT_EQ(*it++, val);
        }
        ASSERT_TRUE(it == expected.end());
    }
    auto x3v = x2;
    DataStream x4(std::move(x2));
    {
        std::vector<int> expected({6,12,18});
        auto it = expected.begin();
        for(auto val: x4.Select<int>([](int x) { return 3*x; } )) {
            ASSERT_EQ(*it++, val);
        }
        ASSERT_TRUE(it == expected.end());
    }
}

TEST( FlattenTest, FlattenInts ) {
    Container<int> t1({1,2,3});
    auto t2 = t1.Select<int>([](int x) { return 2*x; });
    auto t3 = t1.Select<int>([](int x) { return 3*x; });
    Container< DataStream<int> > lists({ t1, t2, t3 });
    auto vs = lists.Flatten<int, int>([](int t) { return 2*t; });

    {
        std::vector<int> expected({2,4,6,4,8,12,6,12,18});
        auto it = expected.begin();
        for(auto val: vs) {
            ASSERT_EQ(*it++, val);
        }
        ASSERT_TRUE(it == expected.end());
    }
}

TEST( WhereTest, Mod2 ) {
    Container<int> t({1,2,3, 4, 5, 6, 7});

    {
        std::vector<int> expected({1,3,5,7});
        auto it = expected.begin();
        for(auto val: t.Where([](int x) { return x % 2 == 1; } )) {
            ASSERT_EQ(*it++, val);
        }
        ASSERT_TRUE(it == expected.end());
    }
}

TEST( TakeTest, Take4 ) {
    Container<int> t({1,2,3, 4, 5, 6, 7});

    {
        std::vector<int> expected({1,2,3,4});
        auto it = expected.begin();
        for(auto val: t.Take(4)) {
            ASSERT_EQ(*it++, val);
        }
        ASSERT_TRUE(it == expected.end());
    }
}

TEST ( GroupByTest, GroupMod2 ) {
    Container<int> t({1, 2, 3, 4, 5, 6, 7});

    auto vs = t.GroupBy<int>([](int x) { return x % 2; });
    {
        std::vector<int> expectedKeys({1,0});
        auto itKeys = expectedKeys.begin();
        std::vector<int> expectedVals({1,3,5,7,
                                       2, 4, 6});
        auto it = expectedVals.begin();
        for(auto val: vs) {
            ASSERT_EQ(*itKeys++, val.first);
            for(auto v: val.second) {
                ASSERT_EQ(*it++, v);
            }
        }
        ASSERT_TRUE(it == expectedVals.end());
        ASSERT_TRUE(itKeys == expectedKeys.end());
    }
}

TEST ( OrderByTest, OrderByMod3) {
    Container<int> t({1, 2, 3, 4, 5, 6, 7});

    auto vs = t.GroupBy<int>([](int x) { return x % 3; })
            .OrderBy<int>([](auto x) { return -x.first; });
    {
        std::vector<int> expectedKeys({2,1,0});
        auto itKeys = expectedKeys.begin();
        std::vector<int> expectedVals({2,5,
                                       1,4,7,
                                       3,6});
        auto it = expectedVals.begin();
        for(auto val: vs) {
            ASSERT_EQ(*itKeys++, val.first);
            for(auto v: val.second) {
                ASSERT_EQ(*it++, v);
            }
        }
        ASSERT_TRUE(it == expectedVals.end());
        ASSERT_TRUE(itKeys == expectedKeys.end());
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FibState {
  int current;
  int prev;
} starting {1, 0};

TEST ( IntegralTests,  Fibbonaci) {
    std::cout << "Test fibbonaci" << std::endl;
    auto  fibbonaciSource = RecurrentContainer< FibState >([](const FibState st){
                return FibState{st.current+st.prev, st.current};
            }, starting)
            .Select<int>([](const FibState st){ return st.current; });

    std::vector<int> expected({3,21,20736,987,6765});
    auto it = expected.begin();

    for(auto v: fibbonaciSource.Where([](const int x){ return x%3 == 0; })
        .Select<int>([](const int x) {
                        return x%2 == 0 ? x*x : x;
                    })
        .Take(5))
    {
        std::cout << v << std::endl;
        ASSERT_EQ(*it++, v);
    }
    ASSERT_TRUE(it == expected.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* const testString = "I think \"because it's a pain in the mikta to both specify properly and implement\" is the major reason we don't have this. \n"
                               "Sure, it would be easy to make   just this bit work, but then people would complain about how using struct template parameters does not work\n"
                               "in all the same situations the other template parameters do (consider partial specializations, or what to do with operator==).";

TEST ( IntegralTests,  WordCount) {
    std::cout << "Test word count" << std::endl;

    Container<char, std::string> str(testString);
    auto tokens = str.GroupBy<int>([](const char c) { return 0; })
            .Select<DataStream<std::string>>([](auto x){
                std::string token;
                std::vector<std::string> strings;
                for(auto c: x.second) {
                    if(c == ' ' || c == '\n') {
                        if(token.size() > 0) {
                            strings.push_back(token);
                            token = "";
                        }
                    } else {
                        token += c;
                    }
                }
                if(token.size() > 0) {
                    strings.push_back(token);
                }
                const Container<std::string> container(strings);
                return DataStream<std::string>(&container);
            })
           .Flatten<std::string, std::string>([](const std::string s) { return s; });
    auto frequences = tokens.GroupBy<std::string>([](const std::string str){
                    return str;
            })
            .Select< std::pair<std::string, int> >([](auto token){
                return std::make_pair( token.first, token.second.ToVector().size() );
            })
            .OrderBy<int>( [](auto token) {
                return -token.second;
            });
    std::vector<int> expectedFrequencies({4,3,2});
    auto itFreq = expectedFrequencies.begin();
    std::vector<std::string> expectedWords({"the","to","parameters"});
    auto itWords = expectedWords.begin();
    for(auto t: frequences) {
        std::cout << t.first << ":" << t.second << std::endl;
        if(itFreq != expectedFrequencies.end()) {
            ASSERT_EQ(*itWords++, t.first);
            ASSERT_EQ(*itFreq++, t.second);
        }
    }
}

int main(int argc, char** argv) {

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
