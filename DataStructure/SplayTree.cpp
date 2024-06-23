template<typename S>
class SplayArray {
private:
	struct SplayNode {
		SplayNode* left, * right, * parent;
		int size;
		S value, prod;

		SplayNode() {
			left = nullptr;
			right = nullptr;
			parent = nullptr;
			size = 1;
		}

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
