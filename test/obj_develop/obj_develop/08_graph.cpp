// Copyright 2016 Aether authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================


#include <iostream>
#include <vector>
#include <set>

struct O;
struct P {
  O* o_;
  P(O* o) : o_(o) {}
  ~P() { release(); }
  void release();
  P(const P& p);
};

struct O {
  int i_;
  int r_ = 1;
  std::vector<P> p_;
  O(int i, const std::vector<P>& p = std::vector<P>()) : i_(i), p_(p) {
  }
  ~O() {
    std::cout << "~ " << i_ << ": ";
    for (auto p : p_) std::cout << p.o_->i_ << ", ";
    std::cout << "\n";
  }
};

void P::release() {
  if (o_ && --o_->r_ == 0) {
    delete o_;
    o_ = nullptr;
  }
}

P::P(const P& p) : o_(p.o_) {
  o_->r_++;
}

void Subgraph(std::set<O*>& subgraph, O* o) {
  // Resolve cyclic references.
  if (subgraph.find(o) != subgraph.end()) {
    return;
  }
  subgraph.insert(o);
  for (auto r : o->p_) {
    Subgraph(subgraph, r.o_);
  }
}

std::set<O*> Subgraph(O* o) {
  std::set<O*> subgraph;
  Subgraph(subgraph, o);
  return subgraph;
}

std::set<O*> ToRelease(const P& root, const P& del) {
  auto list = Subgraph(root.o_);
  auto o = Subgraph(del.o_);
  for (auto it = o.begin(); it != o.end();) {
    if (list.find(*it) != list.end()) {
      it = o.erase(it);
    } else {
      ++it;
    }
  }
  return o;
}

void Graph() {
  P p1{new O(1)};
  P p2{new O(2, {p1})};
  p1.o_->p_.push_back(p2);
  P root{new O(666, {p1, p2})};
  P root2{new O(6666, {p1, p2})};
  p1.release();
  p2.release();
  std::set<O*> r = ToRelease(root, p2);
  root.release();
  
  std::cout << "Run graph\n";
}
