#include <iostream>
#include <bitset>
#include <vector>
#include <cassert>
#include <numeric>

using namespace std;
using ll = long long;

// https://judge.yosupo.jp/submission/238886
// https://atcoder.jp/contests/jag2013summer-day4/submissions/42876075

template<int M>
class BitMatrix {
private:
    int h, w;
    vector<bitset<M>> v;

public:
    BitMatrix(int n, int m) : h(n), w(m), v(n) {}
    BitMatrix(int n = 0) : h(n), w(n), v(n) {
        for (int i = 0; i < n; ++i) v[i][i] = 1;
    }

    int height() const { return h; }
    int width() const { return w; }

    BitMatrix(const BitMatrix& other) = default;
    BitMatrix& operator=(const BitMatrix& other) = default;

    bool operator==(const BitMatrix& other) const { return h == other.h && w == other.w && v == other.v; }
    bool operator!=(const BitMatrix& other) const { return !(*this == other); }

    inline const bitset<M>& operator[](int i) const { return v[i]; }
    inline bitset<M>& operator[](int i) { return v[i]; }

    void push_back(const bitset<M>& row) { v.push_back(row); ++h; }
    void emplace_back(bitset<M>&& row) { v.emplace_back(row); ++h; }

    BitMatrix operator*(const BitMatrix& B) const {
        assert(w == B.h);
        BitMatrix Bt(B.w, B.h);
        for (int i = 0; i < B.h; ++i) for (int j = 0; j < B.w; ++j) Bt[j][i] = B[i][j];
        BitMatrix res(h, B.w);
        for (int i = 0; i < h; ++i) for (int j = 0; j < B.w; ++j)
            res[i][j] = (v[i] & Bt[j]).count() & 1;

        return res;
    }
    BitMatrix& operator*=(const BitMatrix& B) { *this = *this * B; return *this; }

    BitMatrix pow(unsigned long long d) {
        BitMatrix res(h), pow2 = *this;
        while (d > 0) {
            if (d & 1) res *= pow2;
            pow2 *= pow2;
            d >>= 1;
        }
        return res;
    }
};

//【逆行列】O(n^3 / 64)
/*
* n 次正方行列 A の逆行列を返す（存在しなければ空）
*/
template<int M>
BitMatrix<M> Inverse(const BitMatrix<M>& A) {
    int n = A.height();

    // 右側に単位行列を連結した拡大行列を構築
    BitMatrix<2 * M> v(n, 2 * n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
        v[i][j] = A[i][j];
    }
    for (int i = 0; i < n; ++i)
        v[i][n + i] = 1;

    int m = 2 * n;

    // 現在見ている場所を (i, j) とする
    int i = 0, j = 0;

    // 拡大行列に対して行基本変形を行い、
    // 左側を単位行列にできれば逆行列が右側に現れる
    while (i < n && j < m) {
        int i2 = i;
        while (i2 < n && !v[i2][j]) ++i2;

        // すべて 0 の列があったらもうダメ。0x0 行列を返す
        if (i2 == n) return BitMatrix<M>();

        if (i != i2) swap(v[i], v[i2]);

        // 掃き出し
        for (int i2 = 0; i2 < n; ++i2) if (v[i2][j] && i2 != i)
            v[i2] ^= v[i];

        ++i; ++j;
    }

    // 右側が逆行列なのでコピー
    BitMatrix<M> invA(n, n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
        invA[i][j] = v[i][n + j];

    return invA;
}

//【行簡約階段形】O(n m min(n, m) / 64)
/*
* 行基本変形で m×n 行列 A を行簡約階段形に変形し，rank A を返す（ピボットの上下は全て 0）
* また変形後の第 i 行が元の第何行だったかを is[i] に，ピボットが第何列にあるかを js[i] に格納する．
*/
template<int M>
int ReducedRowEchelonFrom(BitMatrix<M>& A, vector<int>* is = nullptr, vector<int>* js = nullptr) {
    int n = A.height(), m = A.width();
    int i = 0, j = 0;

    if (is) {
        is->resize(n);
        iota(is->begin(), is->end(), 0);
    }

    while (i < n && j < m) {
        int i2 = i;
        while (i2 < n && !A[i2][j]) ++i2;
        if (i2 == n) { ++j; continue; }

        if (i != i2) {
            swap(A[i], A[i2]);
            if (is) swap((*is)[i], (*is)[i2]);
        }

        if (js) js->emplace_back(j);

        for (int i2 = 0; i2 < n; ++i2) if (A[i2][j] && i2 != i)
            A[i2] ^= A[i];

        ++i; ++j;
    }

    if (is) is->resize(i);

    return i;
}

// https://judge.yosupo.jp/problem/system_of_linear_equations_mod_2
template<int M>
int LinearEquations(const BitMatrix<M>& A, const vector<bool> b, bitset<M>* x0 = nullptr, vector<bitset<M>>* sol_basis = nullptr) {
    int n = A.height(), m = A.width();

    // v := 拡大係数行列 (A | b)
    vector<bitset<M + 1>> v(n);
    for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) v[i][j] = A[i][j];
    for (int i = 0; i < n; ++i) v[i][m] = b[i];

    // pivots[i] := 第 i 行目のピボットが何列目にあるか
    // あとで解を復元するときに使う
    vector<int> pivots;

    // i, j := 現在の注目位置
    int i = 0, j = 0;

    // <= m なのに注意
    while (i < n && j <= m) {
        // 今見ている列から下の 1 を探す
        int i2 = i;
        while (i2 < n && !v[i2][j]) ++i2;

        // みつからなかったら次の列へ
        if (i2 == n) {
            ++j;
            continue;
        }
        
        // 見つかったら注目している行とスワップ
        if (i != i2) swap(v[i], v[i2]);

        // j 列目にピボットがあったことを記録する
        pivots.emplace_back(j);

        // 掃き出し
        for (int i2 = 0; i2 < n; ++i2) if (v[i2][j] && i2 != i)
            v[i2] ^= v[i];

        // 注目位置を右下へ
        ++i; ++j;
    }

    // 最後に見つかったピボットが m 列目
    // つまり (0 0 ... 0 | 1) のような行が存在したということなのでエラー
    if (!pivots.empty() && pivots.back() == m) return -1;


    // Ax = b の特殊解の構成（任意定数はすべて 0 にする）
    if (x0 != nullptr) {
        x0->reset();
        int rank = pivots.size();
        // x_{pivopts[i]} を変更しようと思ったら i 行目を変えるしかないので
        for (int i = 0; i < rank; ++i) (*x0)[pivots[i]] = v[i][m];

        // 同次形 Ax = b の一般解 {x} の基底の構成（任意定数を 1-hot にする）
        if (sol_basis != nullptr) {
            sol_basis->clear();

            int i = 0;
            for (int j = 0; j < m; ++j) {
                if (i < rank && j == pivots[i]) {
                    ++i;
                    continue;
                }

                bitset<M> x;
                x[j] = 1;
                for (int i2 = 0; i2 < i; ++i2)
                    x[pivots[i2]] = v[i2][j];
                sol_basis->emplace_back(move(x));
            }
        }
    }

    return pivots.size(); // rank を返す
}

