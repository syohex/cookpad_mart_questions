#include <cassert>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <functional>
#include <numeric>
#include <sstream>

namespace {

int truck_number = 3;

struct Product {
    explicit Product(int id, int weight) : id(id), weight(weight) {
    }

    bool operator==(const Product &other) const noexcept {
        return id == other.id && weight == other.weight;
    }

    int id;
    int weight;
};

/*!
@brief コマンドライン引数と渡されたプロダクト情報をパースし, プロダクト情報を返す

--truck= オプションが渡されたとき, トラック数の変更を行う

@param[in] argc 引数の数. プログラム名を含む
@param[in] argv コマンドライン引数群

@return プロダクト情報
*/
template <typename T>
std::vector<Product> ParseArguments(int argc, T argv[]) {
    std::vector<Product> ret;
    if (argc < 2) {
        return ret;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--truck=") == 0) {
            sscanf(argv[i], "--truck=%d", &truck_number);
            continue;
        }

        int id, weight;
        sscanf(argv[i], "%d:%d", &id, &weight);
        ret.emplace_back(id, weight);
    }

    return ret;
}

/*!
@brief elementsから countを選ぶ際のすべての組み合わせを返す

elements={1, 2, 3}, count=2 のとき {{1, 2}, {1, 3}, {2, 3}} の 3つの組み合わせを返す

@param[in] elements 選択対象となる要素群
@param[in] count 要素群から何個選ぶか

@return 全組み合わせ
*/
std::vector<std::vector<int>> Combinations(const std::vector<int> &elements, int count) {
    std::vector<std::vector<int>> ret;
    std::function<void(size_t size, int n, size_t index, const std::vector<int> &choices)> f;
    f = [&f, &ret, &elements](size_t size, int n, size_t index, const std::vector<int> &choices) {
        if (n == static_cast<int>(choices.size())) {
            ret.push_back(choices);
            return;
        }

        if (index >= size) {
            return;
        }

        auto v = choices;
        v.push_back(elements[index]);
        f(size, n, index + 1, v);
        f(size, n, index + 1, choices);
    };

    f(elements.size(), count, 0, std::vector<int>{});
    return ret;
}

/*!
@brief ベクタの差分を返す

@param[in] elements 全体の集合
@param[in] subset elements の部分集合

@return elementsの内 subsetに含まれない要素群
*/
std::vector<int> VectorDiff(const std::vector<int> &elements, const std::vector<int> &subset) {
    std::vector<int> ret;
    for (const auto index : elements) {
        if (std::count(subset.begin(), subset.end(), index) == 0) {
            ret.push_back(index);
        }
    }

    return ret;
}

/*!
@brief 0..total の数列を partitions分割するときのすべての候補を返す

例: total=3, partitions=2の場合, 以下の 3の組み合わせを返す

- [[0], [1, 2]]
- [[1], [0, 2]]
- [[2], [0, 1]]

候補には対称性を持つものは含まれない([[0], [1, 2]] に対する [[1, 2], [0]])

@param[in] total 数列の最大値+1
@param[in] partitions 分割数

@return 全組み合わせ
*/
std::vector<std::vector<std::vector<int>>> PartitionCombinations(size_t total, int partitions) {
    std::vector<std::vector<std::vector<int>>> ret;
    std::function<void(const std::vector<int> &elements, size_t min_size, int parts, const std::vector<std::vector<int>> &acc)> f;
    f = [&f, &ret, &total](const std::vector<int> &v, size_t min_size, int parts, const std::vector<std::vector<int>> &acc) {
        size_t sum = 0;
        for (const auto &v : acc) {
            sum += v.size();
        }
        if (parts == 0) {
            if (total == sum) {
                ret.push_back(acc);
            }
            return;
        }

        if (sum + min_size > total) {
            return;
        }

        size_t limit = v.size() - parts + 1;
        for (size_t i = min_size; i <= limit; ++i) {
            const auto candidates = Combinations(v, i);
            for (const auto &candidate : candidates) {
                const auto remainings = VectorDiff(v, candidate);
                auto new_acc = acc;
                new_acc.push_back(candidate);
                f(remainings, i, parts - 1, new_acc);
            }
        }
    };

    std::vector<int> base(total);
    std::iota(base.begin(), base.end(), 0);

    f(base, 1, partitions, std::vector<std::vector<int>>{});
    return ret;
}

/*!
@brief produces[indexes...]の総和を返す

@param[in] products プロダクト情報
@param[in] indexes 選択対象のインデックス群

@return 総重量
*/
int TotalWeights(const std::vector<Product> &products, const std::vector<int> &indexes) {
    int sum = 0;
    for (const auto index : indexes) {
        sum += products[index].weight;
    }

    return sum;
}

/*!
@brief 各トラックに積まれるプロダクト重量の差分の総和を返す

@param[in] products プロダクト情報
@param[in] indexes 各トラックに積むプロダクト indexの集合

@return 各トラックに積まれるプロダクト重量の差分の総和
*/
int DiffSum(const std::vector<Product> &products, const std::vector<std::vector<int>> &candidates) {
    std::vector<int> sums;
    for (const auto &candidate : candidates) {
        sums.push_back(TotalWeights(products, candidate));
    }

    int diff_sum = 0;
    for (size_t i = 0; i < sums.size() - 1; ++i) {
        for (size_t j = i + 1; j < sums.size(); ++j) {
            diff_sum += std::abs(sums[i] - sums[j]);
        }
    }

    return diff_sum;
}

/*!
@brief 結果表示用

@param[in] products プロダクト情報
@param[in] indexes 各トラックに積むプロダクト indexの集合

@return 各トラックに積まれるプロダクト IDをカンマで結合した文字列
*/
std::string ProductIDString(const std::vector<Product> &products, const std::vector<int> &indexes) {
    std::stringstream ss;
    for (size_t i = 0; i < indexes.size(); ++i) {
        if (i != 0) {
            ss << ",";
        }

        ss << products[indexes[i]].id;
    }

    return ss.str();
}

#ifdef UNIT_TEST
[[noreturn]] void UnitTest() {
    const char *argv[] = {"dummy", "1:50", "2:30", "3:40", "4:10"};
    {
        auto ret = ParseArguments(5, argv);
        assert(ret.size() == 4);
        assert((ret[0] == Product{1, 50}));
        assert((ret[1] == Product{2, 30}));
        assert((ret[2] == Product{3, 40}));
        assert((ret[3] == Product{4, 10}));
    }

    {
        auto ret = Combinations(std::vector<int>{1, 2, 3}, 2);
        assert(ret.size() == 3); // (1, 2), (1, 3), (2, 3)
        assert((std::count(ret.begin(), ret.end(), std::vector<int>{1, 2}) == 1));
        assert((std::count(ret.begin(), ret.end(), std::vector<int>{1, 3}) == 1));
        assert((std::count(ret.begin(), ret.end(), std::vector<int>{2, 3}) == 1));
    }

    {
        auto rest = VectorDiff(std::vector<int>{0, 1, 2, 3, 4}, std::vector<int>{2, 3});
        assert(rest.size() == 3);
        assert(rest[0] == 0);
        assert(rest[1] == 1);
        assert(rest[2] == 4);

        rest = VectorDiff(std::vector<int>{0, 1, 2, 3}, std::vector<int>{0, 1, 2});
        assert(rest.size() == 1);
        assert(rest[0] == 3);
    }

    {
        const auto candidates = PartitionCombinations(3, 2);
        assert(candidates.size() == 3);
        assert(std::all_of(candidates.begin(), candidates.end(),
                           [](const std::vector<std::vector<int>> &c) { return c.size() == 2; }));

        decltype(candidates) expecteds{
            {
                {0},
                {1, 2},
            },
            {
                {1},
                {0, 2},
            },
            {
                {2},
                {0, 1},
            },
        };

        for (const auto &expected : expecteds) {
            assert(std::count(candidates.begin(), candidates.end(), expected) == 1);
        }
    }

    printf("Test OK!!\n");
    std::exit(0);
}
#endif /* UNIT_TEST */

} // namespace

int main(int argc, char *argv[]) {
#if UNIT_TEST
    UnitTest();
#endif

    const std::vector<Product> products = ParseArguments(argc, argv);
    if (products.empty()) {
        printf("Usage: %s product_id:weight ...\n", argv[0]);
        return 1;
    }
    auto conbinations = PartitionCombinations(products.size(), truck_number);

    std::vector<std::vector<int>> ret;
    int min_val = INT_MAX;
    for (const auto &conbination : conbinations) {
        int diff_sum = DiffSum(products, conbination);
        if (diff_sum < min_val) {
            ret = conbination;
            min_val = diff_sum;
        }
    }

    for (int i = 1; i <= truck_number; ++i) {
        const auto &indexes = ret[i - 1];
        printf("truck_%d:%s\n", i, ProductIDString(products, indexes).c_str());
    }

    return 0;
}
