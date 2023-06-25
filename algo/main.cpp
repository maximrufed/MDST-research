#include <iostream>
#include <vector>
#include <algorithm>
#include <optional>
#include <queue>

template <typename T>
using vector = std::vector<T>;

struct edge {
  int from;
  int to;
  int* in_span;
};

using Graph = vector<vector<int>>;

struct MDST {
  int vertex_count;
  vector<vector<edge>> graph;
  vector<vector<edge*>> span;  // spanning tree

  vector<int> used;
  vector<int> depth;

  // prepare
  vector<edge*> pred;  // edge up to parent of vertex i in spanning tree
  vector<int> up;  // dsu of vertexes, shows upper vertex
  vector<edge*> down;  // last edge down via merge in dfs - чтобы знать, какое удалять
  vector<edge*> conn_edge;  // edge that is deleted ...
  vector<int> good;
  vector<int> tin;
  vector<int> tout;

  int step_count;
  int max_degree;

  template<typename T, typename V>
  void erase(T& container, const V& value) {
    container.erase(std::remove(container.begin(), container.end(), value), container.end());
  }


  // void add_edge(int from, int to) {
  //   span[from].push_back(to);
  //   span[to].push_back(from);
  // }

  // void delete_edge(Graph& graph, int from, int to) {
  //   erase(graph[from], to);
  //   erase(graph[to], from);
  // }

  void dfs_find_spanning_tree(int vertex, int p) {
    // std::cout << "in " << vertex << " " << p << std::endl;
    used[vertex] = 1;
    for (auto& e : graph[vertex]) {
      if (!used[e.to]) {
        span[vertex].push_back(&e);
        *(e.in_span) = 1;
        dfs_find_spanning_tree(e.to, vertex);
      } else if (e.to == p) {
        span[vertex].push_back(&e);
      }
    }
  }

  void dfs_pred(int v, int p, int& timer, int d) {
    // std::cout << "in " << v << " " << p << std::endl;
    tin[v] = timer++;
    depth[v] = d;
    for (auto e : span[v]) {
      if (e->to == p) {
        pred[v] = e;
        continue;
      }
      dfs_pred(e->to, v, timer, d + 1);
    }
    tout[v] = timer++;
  }

  int get(int v) {
    if (up[v] == v) return v;
    return up[v] = get(up[v]);
  }

  void merge(int a, int b) {
    a = get(a);
    b = get(b);
    if (a == b) return;
    if (depth[a] > depth[b]) std::swap(a, b);
    up[b] = a;
  }

  bool is_pred(int a, int b) {
    // std::cout << "is pred " << a << " " << b << " = " << ((tin[a] <= tin[b]) & (tout[a] >= tout[b])) << std::endl;
    return (tin[a] <= tin[b]) && (tout[a] >= tout[b]);
  }

  void mark_good(int v) {
    good[v] = 1;
    for (auto e : span[v]) {
      if (e != pred[v]) {
        if (good[e->to]) {
          merge(v, e->to);
        }
      }
    }
  }

  int upsolve(int v, int o, edge* pre_edge) {
    v = get(v);
    // std::cout << "V = " << v << " " << o << std::endl;
    while (!is_pred(v, o)) {
      // std::cout << "in while" << std::endl;
      edge* e = pred[v];
      down[e->to] = e;
      conn_edge[e->to] = pre_edge;
      mark_good(e->to);

      if (span[e->to].size() == max_degree) {
        return e->to;
      }
      v = get(e->to);
    }
    return -1;
  }

  void change(int v) {
    if (span[v].size() < max_degree - 1) return;
    *(down[v]->in_span) = 0;
    *(conn_edge[v]->in_span) = 1;
    change(conn_edge[v]->from);
    change(conn_edge[v]->to);
  }

  std::optional<Graph> findMDST(Graph init_graph) {
    vertex_count = init_graph.size();

    int m = 0;
    for (int i = 0; i < vertex_count; ++i) {
      for (auto e : init_graph) {
        m++;
      }
    }
    m /= 2;

    vector<int> in_span_vector(m, 0);

    // graph = init_graph;
    graph.resize(vertex_count);
    m = 0;
    for (int i = 0; i < vertex_count; ++i) {
      for (auto to : init_graph[i]) {
        if (i >= to) continue;
        // std::cout << "edge " << i << " " << to << " " << &(in_span_vector[in_span_vector.size() - 1]) << std::endl;
        graph[i].push_back(edge{i, to, &(in_span_vector[m])});
        graph[to].push_back(edge{to, i, &(in_span_vector[m])});
        ++m;
      }
    }

    // find arbitrary spanning tree via dfs
    used.assign(vertex_count, 0);
    span.resize(vertex_count);

    dfs_find_spanning_tree(0, 0);

    // std::cout << "size = " << in_span_vector.size() << std::endl;
    // in_span_vector[1] = 1;
    // for (int i = 0; i < vertex_count; ++i) {
    //   std::cout << i << " : ";
    //   for (auto e : graph[i]) {
    //     std::cout << "{" << e.from << " " << e.to << " : " << *(e.in_span) << " } ";
    //   }
    //   std::cout << std::endl;
    // }

// {
//     vector<vector<int>> res;
//     res.resize(vertex_count);
//     for (int i = 0; i < vertex_count; ++i) {
//       for (auto e : graph[i]) {
//         if (*(e.in_span)) {
//           res[i].push_back(e.to);
//         }
//       }
//     }
// }

//     return res;

    // check if graph is connected
    if (std::find(used.begin(), used.end(), 0) != used.end()) {
      // graph is not connected
      return std::nullopt;
    }

    pred.resize(vertex_count);
    up.resize(vertex_count);
    down.resize(vertex_count);
    good.resize(vertex_count);
    tin.resize(vertex_count);
    tout.resize(vertex_count);
    conn_edge.resize(vertex_count);
    depth.resize(vertex_count);


    // run steps
    step_count = 0;
    while (true) {
      ++step_count;

      // std::cout << step_count << std::endl;

      // find max degree of vertexes in current spanning tree
      max_degree = 0;
      for (int i = 0; i < vertex_count; ++i) {
        max_degree = std::max(max_degree, static_cast<int>(span[i].size()));
      }

      // calc good
      // std::cout << "GOOD: ";
      for (int i = 0; i < vertex_count; ++i) {
        good[i] = (span[i].size() <= max_degree - 2);
        // std::cout << good[i] << " ";
      }

      // std::cout << std::endl;

      // dfs
      int timer = 0;
      dfs_pred(0, 0, timer, 0);

      // make queue of edges
      std::queue<edge*> edges;


      for (int i = 0; i < vertex_count; ++i) {
        if (!good[i]) continue;
        for (auto& e : graph[i]) {
          if (!good[e.to]) continue;
          if (e.to <= i) continue;
          if (*(e.in_span)) continue;
          edges.push(&e);
        }
      }


      // up init
      up[0] = 0;
      for (int i = 1; i < vertex_count; ++i) {
        up[i] = i;
        if (good[i] && good[pred[i]->to]) {
          up[i] = pred[i]->to;
        }
        // up[i] = (good[i] ? pred[i]->to : i);
      }

      int status = 0;
      while (!edges.empty()) {
        edge* cur_edge = edges.front();
        edges.pop();

        // std::cout << "edge " << cur_edge->from << " " << cur_edge->to << std::endl;

        int a = get(cur_edge->from);
        int b = get(cur_edge->to);

        if (a == b) continue;


        int v1 = upsolve(a, b, cur_edge);
        if (v1 != -1) {
          change(v1);
          status = 1;
          break;
        }

        int v2 = upsolve(b, a, cur_edge);
        if (v2 != -1) {
          change(v2);
          status = 1;
          break;
        }

      }

      // std::cout << "status = " << status << std::endl;
      if (status == 0) {
        // stop algo
        break;
      }

      // change span
      span.clear();
      span.resize(vertex_count);

      for (int i = 0; i < vertex_count; ++i) {
        for (auto& e : graph[i]) {
          if (*(e.in_span)) {
            span[i].push_back(&e);
          }
        }
      }

    }

    vector<vector<int>> res;
    res.resize(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      for (auto e : graph[i]) {
        if (*(e.in_span)) {
          res[i].push_back(e.to);
        }
      }
    }

    return res;
  }
};

int main() {

  int n, m;
  vector<vector<int>> g;

  std::cin >> n >> m;
  g.resize(n);
  for (int i = 0; i < m; ++i) {
    int a, b;
    std::cin >> a >> b;
    g[a].push_back(b);
    g[b].push_back(a);
  }

  MDST span;

  std::optional<Graph> tree = span.findMDST(g);

  if (!tree.has_value()) {
    std::cout << "NO VALUE" << std::endl;
  } else {
    vector<vector<int>> real_tree = tree.value();
    std::cout << "n = " << real_tree.size() << "\n";
    for (int i = 0; i < real_tree.size(); ++i) {
      std::cout << i << " : ";
      for (auto e : real_tree[i]) {
        std::cout << e << " ";
      }
      std::cout << "\n";
    }
  }

  return 0;
}
