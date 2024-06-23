template<typename S>
class SplayArray {
private:
	struct SplayNode {
		SplayNode* left, * right, * parent;
		int size, rev;
		S value, prod;

		SplayNode() : left(nullptr), right(nullptr), parent(nullptr), size(1), rev(0) {}

		void rotate() {
			SplayNode* pp, * p, * c;
			p = this->parent;
			pp = p->parent;

			if (p->left == this) {
				c = this->right;
				this->right = p;
				p->left = c;
			}
			else {
				c = this->left;
				this->left = p;
				p->right = c;
			}
			if (pp && pp->left == p) pp->left = this;
			if (pp && pp->right == p) pp->right = this;
			this->parent = pp;
			p->parent = this;
			if (c) c->parent = p;

			p->update();
			this->update();
		}

		int state() {
			if (!this->parent) return 0;
			if (this->parent->left == this) return 1;
			if (this->parent->right == this) return -1;
			return 0;
		}

		void splay() {
			while (this->state() != 0) {
				if (this->parent->state() == 0) {
					this->rotate();
				}
				else if (this->state() == this->parent->state()) {
					this->parent->rotate();
					this->rotate();
				}
				else {
					this->rotate();
					this->rotate();
				}
			}
		}

		void update() {
			this->size = 1;
			this->prod = this->value;
			if (this->left) {
				this->size += this->left->size;
				this->prod = op(this->prod, this->left->prod);
			}
			if (this->right) {
				this->size += this->right->size;
				this->prod = op(this->right->prod, this->prod);
			}
		}
	};

	SplayNode* root;
	vector<SplayNode> A;
	const inline static function<S(S, S)> op = [](int a, int b) { return min(a, b); };
	const inline static S e = 0;

public:
	SplayArray(int N) : A(N) {
		for (int i = 0; i < N - 1; ++i) {
			A[i].parent = &A[i + 1];
			A[i + 1].left = &A[i];
			A[i + 1].update();
		}
		root = &A[N - 1];
	}

	S get(int idx) {
		root = get(idx, root);
		return root->value;
	}

	SplayNode* erase(int idx) {
		root = get(idx, root);
		SplayNode* lroot = root->left;
		SplayNode* rroot = root->right;
		if (lroot) lroot->parent = nullptr;
		if (rroot) rroot->parent = nullptr;
		root->left = nullptr;
		root->right = nullptr;
		root->update();
		SplayNode* res = root;
		root = merge(lroot, rroot);
		return res;
	}


	// pop r, insert l
	void shift(int l, int r) {
		if (l >= r) return;
		SplayNode* node = erase(r);
		insert(l, node);
	}
	// pop r, insert l, k times
	// if k < 0, pop l, insert r, k times
	void shift(int l, int r, int k) {
		if (l >= r) return;
		k %= r - l;
		if (k == 0) return;
		if (k < 0) k = (r - l) + k;
		SplayNode* left, * m1, * m2, * right;
		tie(m2, right) = split(r, root);
		tie(m1, m2) = split(l + k, m2);
		tie(left, m1) = split(l, m1);
		root = merge(merge(merge(left, m2), m1), right);
	}

	S prod(int l, int r) {
		SplayNode* lroot, * mroot, * rroot;
		auto tmp = split(r + 1, root);
		rroot = tmp.second;
		tie(lroot, mroot) = split(l, tmp.first);
		int res = mroot->prod;
		root = merge(merge(lroot, mroot), rroot);
		return res;
	}

	void set(int i, S val) {
		root = get(i, root);
		root->value = val;
		root->update();
	}

private:
	SplayNode* get(int idx, SplayNode* node) {
		SplayNode* now = node;
		while (true) {
			int lsize = now->left ? now->left->size : 0;
			if (idx < lsize) {
				now = now->left;
			}
			if (idx == lsize) {
				now->splay();
				return now;
			}
			if (idx > lsize) {
				now = now->right;
				idx = idx - lsize - 1;
			}
		}
	}

	SplayNode* insert(int idx, SplayNode* node) {
		auto [lroot, rroot] = split(idx, root);
		return root = merge(merge(lroot, node), rroot);
	}

	SplayNode* merge(SplayNode* lroot, SplayNode* rroot) {
		if (lroot == nullptr) return rroot;
		if (rroot == nullptr) return lroot;
		lroot = get(lroot->size - 1, lroot);
		lroot->right = rroot;
		rroot->parent = lroot;
		lroot->update();
		return lroot;
	}

	pair<SplayNode*, SplayNode*> split(int left_cnt, SplayNode* root) {
		if (left_cnt == 0) return make_pair(nullptr, root);
		if (left_cnt >= root->size) return make_pair(root, nullptr);
		root = get(left_cnt, root);
		SplayNode* lroot, * rroot;
		lroot = root->left;
		rroot = root;
		rroot->left = nullptr;
		lroot->parent = nullptr;
		rroot->update();
		return make_pair(lroot, rroot);
	}
};

int AOJ_Volume15_1508_RMQ() {
	int n, q; cin >> n >> q;
	SplayArray<int> sp(n);

	for (int i = 0; i < n; ++i) {
		int a; cin >> a;
		sp.set(i, a);
	}

	for (int i = 0; i < q; ++i) {
		int ord; cin >> ord;
		if (ord == 0) {
			int l, r; cin >> l >> r;
			sp.shift(l, r);
		}
		if (ord == 1) {
			int l, r; cin >> l >> r;
			cout << sp.prod(l, r) << endl;
		}
		if (ord == 2) {
			int pos, val; cin >> pos >> val;
			sp.set(pos, val);
		}
	}
	return 0;
}

void RangeShiftTest() {
	int N = 10;
	SplayArray<int> sp(N);
	rep(i, N) sp.set(i, i);
	rep(j, N)
		cout << sp.get(j) << " "; cout << endl;
	int W = 5;
	for (int i = 0; i < N - W + 1; ++i) {
		sp.shift(i, i + W, -3);
		rep(j, N) cout << sp.get(j) << " "; cout << endl;
	}
	for (int i = N - W; i >= 0; --i) {
		sp.shift(i, i + W, 3);
		rep(j, N) cout << sp.get(j) << " "; cout << endl;
	}
}





/*
* なんかバグっていて range reverse range sum が通らない。
* https://judge.yosupo.jp/problem/range_reverse_range_sum
// https://www.mathenachia.blog/splay-tree-impl/
constexpr ll MOD = 998244353;
class LazySplayTree {
private:
	struct S { long long x, z; }; // (ans, 区間幅)
	static S op(S l, S r) { return { (l.x + r.x) % MOD, l.z + r.z }; }
	static S e() { return { 0,0 }; }
	static void reverse_prod(S& x) {  } // 反転時に prod を更新する
	struct F { long long a, b; }; // 一次関数
	static S mapping(F f, S x) { return { (x.x * f.a + x.z * f.b) % MOD, x.z }; }
	static F composition(F f, F x) { return { f.a * x.a % MOD, (f.a * x.b + f.b) % MOD }; }
	static F id() { return { 1,0 }; }

	struct Node {
		Node* l = 0, * r = 0, * p = 0;
		S val = e(); // 頂点が持つ値
		S prod = e(); // 集約（ rev==1 のときでも reverse_prod 作用済み）
		F f = id(); // 遅延（a,prod には作用済み）
		int i = -1; // 配列の index
		int z = 0; // 頂点の重み ( NIL なら 0 、普通の頂点は 1 ）
		int sumz = 0; // 部分木の重み
		int rev = 0; // 反転の遅延
	};

	using NodePtr = unique_ptr<Node>;
	NodePtr pNIL;
	Node* NIL = nullptr;
	vector<NodePtr> A;
	Node* root;

public:
	LazySplayTree() {
		if (!pNIL) {
			pNIL = make_unique<Node>();
			NIL = pNIL.get();
			NIL->l = NIL->r = NIL->p = NIL;
		}
		root = NIL;
	}

	S at(int k) {
		return kth_element(k)->val;
	}

	void insert(int k, S x) {
		NodePtr pnx = make_unique<Node>(*NIL);
		Node* nx = pnx.get();
		nx->z = nx->sumz = 1;
		nx->i = A.size();
		nx->val = nx->prod = x;
		A.emplace_back(move(pnx));
		if (k == 0) {  // 左端
			nx->r = root;
			if (root != NIL) root->p = nx; // 元々 0 頂点かもしれない
			root = nx;
			update(nx); // 挿入したら集約
			return;
		}
		if (k == root->sumz) { // 右端（左端と同様）
			nx->l = root;
			if (root != NIL) root->p = nx;
			root = nx;
			update(nx);
			return;
		}
		auto p = kth_element(k);
		nx->l = p->l;
		nx->r = p;
		root = nx;
		p->l->p = nx;
		p->p = nx;
		p->l = NIL;
		update(p);  // split/merge の影響
		update(nx); //
	}

	void erase(int k) {
		auto p = kth_element(k);
		if (k == 0) { // 左端
			root = p->r;
			if (root != NIL) root->p = NIL; // 0 頂点になるかもしれない
		}
		else if (k == root->sumz - 1) { // 右端
			root = p->l;
			if (root != NIL) root->p = NIL;
		}
		else {
			auto l = p->l;
			auto r = p->r;
			r->p = NIL;   // split
			root = r;     //
			kth_element(0);
			r = root;     // merge
			r->l = l;     //
			l->p = r;     //
			update(r);    // split/merge の影響
		}
		swap(p->i, A.back()->i); // index が更新されるよ
		swap(A[p->i], A[A.back()->i]); // 後ろに移動して
		A.pop_back(); // 削除
	}

	void reverse(int l, int r) {
		auto c = range(l, r);
		c->rev ^= 1;
		reverse_prod(c->prod);
		splay(c);
	}

	void apply(int l, int r, F f) {
		auto c = range(l, r);
		c->val = mapping(f, c->val);
		c->prod = mapping(f, c->prod);
		c->f = composition(f, c->f);
		splay(c);
	}

	S prod(int l, int r) {
		return range(l, r)->prod;
	}

	int size() { return (int)A.size(); }

private:
	// 伝播
	void propagate(Node* node) {
		if (node->l != NIL) {
			// a,prod への作用を忘れずに
			node->l->val = mapping(node->f, node->l->val);
			node->l->prod = mapping(node->f, node->l->prod);
			node->l->f = composition(node->f, node->l->f);
		}
		if (node->r != NIL) {
			// a,prod への作用を忘れずに
			node->r->val = mapping(node->f, node->r->val);
			node->r->prod = mapping(node->f, node->r->prod);
			node->r->f = composition(node->f, node->r->f);
		}
		if (node->rev) {
			swap(node->l, node->r);
			if (node->l != NIL) {
				node->l->rev ^= 1; // 伝播
				reverse_prod(node->l->prod);
			}
			if (node->r != NIL) {
				node->r->rev ^= 1; // 伝播
				reverse_prod(node->r->prod);
			}
			node->rev = 0;
		}
		node->f = id(); // 伝播済み
	}

	// 集約
	void update(Node* c) {
		c->sumz = c->l->sumz + c->r->sumz + 1; // 部分木の重み
		c->prod = op(op(c->l->prod, c->val), c->r->prod); // 集約　c は伝播済み
	}

	// （便利）
	// p の親が、子として p を参照するので、それを書き換えられるようにする
	// 根の場合に拡張して R の参照を返す。
	Node*& parentchild(Node* p) {
		if (p->p == NIL) return root;
		if (p->p->l == p) return p->p->l;
		else return p->p->r;
	}

	// 左回転
	void rotL(Node* c) {
		Node* p = c->p;
		parentchild(p) = c;
		c->p = p->p;
		p->p = c;
		if (c->l != NIL) c->l->p = p; // 子が NIL かもしれない
		p->r = c->l;
		c->l = p;
	}

	// 右回転
	void rotR(Node* c) {
		Node* p = c->p;
		parentchild(p) = c;
		c->p = p->p;
		p->p = c;
		if (c->r != NIL) c->r->p = p; // 子が NIL かもしれない
		p->l = c->r;
		c->r = p;
	}

	// splay 後、 c は伝播済み
	void splay(Node* c) {
		propagate(c); // ループが回らない時のために
		while (c->p != NIL) {
			Node* p = c->p;
			Node* pp = p->p;
			// 伝播は親から
			if (pp != NIL) propagate(pp);
			if (p != NIL) propagate(p);
			propagate(c);
			if (p->l == c) {
				if (pp == NIL) { rotR(c); }
				else if (pp->l == p) { rotR(p); rotR(c); }
				else if (pp->r == p) { rotR(c); rotL(c); }
			}
			else {
				if (pp == NIL) { rotL(c); }
				else if (pp->r == p) { rotL(p); rotL(c); }
				else if (pp->l == p) { rotL(c); rotR(c); }
			}
			// 集約は子から
			if (pp != NIL) update(pp);
			if (p != NIL) update(p);
			update(c);
		}
		update(c); // ループが回らない時のために
	}

	Node* kth_element(int idx) {
		Node* now = root;
		while (true) {
			propagate(now);
			int lsize = now->l != NIL ? now->l->sumz : 0;
			if (idx < lsize) {
				now = now->l;
			}
			else if (idx == lsize) {
				break;
			}
			else if (idx > lsize) {
				now = now->r;
				idx = idx - lsize - 1;
			}
		}
		propagate(now);
		splay(now);
		return now;
	}

	Node* range(int l, int r) {
		if (l == 0 && r == root->sumz) return root; // 全域
		if (l == 0) return kth_element(r)->l; // 左端
		if (r == root->sumz) return kth_element(l - 1)->r; // 右端
		auto rp = kth_element(r);
		auto lp = rp->l;
		root = lp;        // split
		lp->p = NIL;   //
		lp = kth_element(l - 1);
		root = rp;        // merge
		rp->l = lp;    //
		lp->p = rp;    //
		propagate(rp); // split/merge の影響
		return lp->r;
	}
};
*/
