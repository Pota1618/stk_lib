struct SplayNode {
	SplayNode* left, * right, * parent;
	int size, value, minimum;

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
		this->minimum = this->value;
		if (this->left) {
			this->size += this->left->size;
			this->minimum = min(this->minimum, this->left->minimum);
		}
		if (this->right) {
			this->size += this->right->size;
			this->minimum = min(this->minimum, this->right->minimum);
		}
	}
};

SplayNode* get(int idx, SplayNode* root) {
	SplayNode* now = root;
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

using SN = SplayNode;

SN* merge(SN* lroot, SN* rroot) {
	if (lroot == nullptr) return rroot;
	if (rroot == nullptr) return lroot;
	lroot = get(lroot->size - 1, lroot);
	lroot->right = rroot;
	rroot->parent = lroot;
	lroot->update();
	return lroot;
}
pair<SN*, SN*> split(int left_cnt, SN* root) {
	if (left_cnt == 0) return make_pair(nullptr, root);
	if (left_cnt >= root->size) return make_pair(root, nullptr);
	root = get(left_cnt, root);
	SN* lroot, * rroot;
	lroot = root->left;
	rroot = root;
	rroot->left = nullptr;
	lroot->parent = nullptr;
	rroot->update();
	return make_pair(lroot, rroot);
}
SN* insert(int idx, SN* node, SN* root) {
	auto [lroot, rroot] = split(idx, root);
	return merge(merge(lroot, node), rroot);
}
pair<SN*, SN*> erase(int idx, SN* root) {
	root = get(idx, root);
	SN* lroot = root->left;
	SN* rroot = root->right;
	if (lroot) lroot->parent = nullptr;
	if (rroot) rroot->parent = nullptr;
	root->left = nullptr;
	root->right = nullptr;
	root->update();
	return make_pair(merge(lroot, rroot), root);
}

SN* shift(int l, int r, SN* root) {
	SN* node = nullptr;
	tie(root, node) = erase(r, root);
	return insert(l, node, root);
}
pair<SN*, int> rmq(int l, int r, SN* root) {
	SN* lroot, * mroot, * rroot;
	auto tmp = split(r + 1, root);
	rroot = tmp.second;
	tie(lroot, mroot) = split(l, tmp.first);
	int ans = mroot->minimum;
	return make_pair(merge(merge(lroot, mroot), rroot), ans);
}

int AOJ_ITP2_1_A_Vector() {
	vector<SplayNode> node(200200);
	int q, ord, x;
	cin >> q;
	int vecsize = 0;

	for (int i = 0; i < q; ++i) {
		node[i].parent = &node[i + 1];
		node[i + 1].left = &node[i];
		node[i + 1].update();
	}
	SplayNode* root = &node[q];

	for (int i = 0; i < q; ++i) {
		cin >> ord;
		if (ord == 0) {
			cin >> x;
			root = get(vecsize++, root);
			root->value = x;
		}
		else if (ord == 1) {
			cin >> x;
			root = get(x, root);
			cout << root->value << endl;
		}
		else {
			vecsize--;
		}
	}
	return 0;
}
int AOJ_Volume15_1508_RMQ() {
	int n, q; cin >> n >> q;
	int vecsize = 0;

	vector<SplayNode> node(n);

	for (int i = 0; i < n - 1; ++i) {
		node[i].parent = &node[i + 1];
		node[i + 1].left = &node[i];
		node[i + 1].update();
	}

	for (int i = 0; i < n; ++i) {
		cin >> node[i].value;
		node[i].update();
	}

	SplayNode* root = &node[n - 1];

	for (int i = 0; i < q; ++i) {
		int ord; cin >> ord;
		if (ord == 0) {
			int l, r; cin >> l >> r;
			root = shift(l, r, root);
		}
		if (ord == 1) {
			int l, r; cin >> l >> r;
			auto tmp = rmq(l, r, root);
			root = tmp.first;
			cout << tmp.second << endl;
		}
		if (ord == 2) {
			int pos, val; cin >> pos >> val;
			root = get(pos, root);
			root->value = val;
			root->update();
		}
	}
	return 0;
}
