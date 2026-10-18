#include <iostream>
#include <exception>
#include <vector>
#include <queue>
#include <cmath>
#include <random>
#include <string>
#include <algorithm>
#include <map>

enum Status { UNKNOWN, OK, BROKEN };

struct Edge {
    int64_t u;
    int64_t v;
    double p;
    Status status = UNKNOWN;
    
    int64_t get_other(int64_t node) const {
        return (node == u) ? v : u;
    }
};

struct Room {
    double profit;
    bool visited = false;
    bool looked = false;
};

class MineExplorer {
private:
    int64_t n;
    int64_t m;
    double clear_cost;
    int64_t start_room;
    int64_t current_room;
    
    std::vector<Room> rooms;
    std::vector<Edge> edges;
    std::vector<std::vector<int64_t>> adj;

    std::mt19937 rng;

    int64_t findEdgeIdx(int64_t u, int64_t v) {
        for (auto e_idx : adj[u]) {
            if (edges[e_idx].get_other(u) == v) {
                return e_idx;
            }
        }
        return -1;
    }

public:
    MineExplorer(int64_t n, int64_t m, double c, int64_t s) 
        : n(n), m(m), clear_cost(c), start_room(s), current_room(s) {
        rooms.resize(n);
        adj.resize(n);
        rng.seed(42);
    }

    void setProfit(int64_t u, double profit) {
        rooms[u].profit = profit;
    }

    void addEdge(int64_t u, int64_t v, double p) {
        Edge e;
        e.u = u;
        e.v = v;
        e.p = p;
        e.status = UNKNOWN;
        
        int64_t idx = edges.size();
        edges.push_back(e);
        adj[u].push_back(idx);
        adj[v].push_back(idx);
    }

    void sendLook() {
        if (rooms[current_room].looked) {
            return;
        }
        
        std::cout << "LOOK\n";
        std::cout.flush();

        for (int64_t i = 0; i < adj[current_room].size(); ++i) {
            int64_t v;
            std::string state;
            std::cin >> v >> state;

            edges[findEdgeIdx(current_room, v)].status = (state == "OK" ? OK : BROKEN);
        }
        rooms[current_room].looked = true;
    }

    void sendMove(int64_t v) {
        std::cout << "MOVE " << v << '\n';
        std::cout.flush();

        std::string res;
        std::cin >> res;
        
        if (res != "OK") {
            throw std::logic_error("Move error");
        }

        current_room = v;
        rooms[v].visited = true;
    }

    void sendClear(int64_t u, int64_t v) {
        std::cout << "CLEAR " << u << " " << v << '\n';
        std::cout.flush();

        std::string res;
        std::cin >> res;
        
        edges[findEdgeIdx(u, v)].status = OK;
        current_room = start_room;
    }

    void sendEnd() {
        std::cout << "END\n";
        std::cout.flush();

        std::string res;
        while(std::cin >> res) {}
        exit(0);
    }

    void physicallyMoveTo(int64_t target) {
        if (current_room == target) {
            return;
        }

        std::vector<int64_t> parent(n, -1);
        std::vector<bool> vis(n, false);
        std::queue<int64_t> q;
        
        q.push(current_room);
        vis[current_room] = true;

        while (!q.empty()) {
            int64_t u = q.front();
            q.pop();

            if (u == target) {
                break;
            }

            for (auto e_idx : adj[u]) {
                if (edges[e_idx].status == OK) {
                    int64_t nxt = edges[e_idx].get_other(u);
                    if (!vis[nxt]) {
                        vis[nxt] = true;
                        parent[nxt] = u;
                        q.push(nxt);
                    }
                }
            }
        }

        if (!vis[target]) {
            throw std::logic_error("Move error (BFS)");
        }

        std::vector<int64_t> path;
        int64_t curr = target;
        while (curr != current_room) {
            path.push_back(curr);
            curr = parent[curr];
        }
        std::reverse(path.begin(), path.end());

        for (auto next_room : path) {
            sendMove(next_room);
        }
    }

    void exploreKnownZone() {
        rooms[current_room].visited = true;
        
        bool changed = true;
        while (changed) {
            changed = false;
            sendLook();

            int64_t best_target = -1;
            
            std::queue<int64_t> q;
            std::vector<bool> vis(n, false);
            q.push(current_room);
            vis[current_room] = true;

            while (!q.empty()) {
                int64_t u = q.front();
                q.pop();

                if (!rooms[u].looked) {
                    best_target = u;
                    break;
                }

                for (auto e_idx : adj[u]) {
                    if (edges[e_idx].status == OK) {
                        int64_t nxt = edges[e_idx].get_other(u);
                        if (!vis[nxt]) {
                            vis[nxt] = true;
                            q.push(nxt);
                        }
                    }
                }
            }

            if (best_target != -1) {
                physicallyMoveTo(best_target);
                changed = true;
            }
        }
    }

    double evaluateDijkstra(int64_t start_node) {
        std::vector<double> dist(n, 1e9);
        std::priority_queue<std::pair<double, int64_t>, std::vector<std::pair<double, int64_t>>, std::greater<std::pair<double, int>>> pq;
        
        dist[start_node] = 0.0;
        pq.push({0, start_node});
        double expected_profit = 0;

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d > dist[u]) {
                continue;
            }

            if (!rooms[u].visited) {
                expected_profit += rooms[u].profit * exp(-d);
            }

            for (int64_t e_idx : adj[u]) {
                Edge e = edges[e_idx];
                if (e.status == BROKEN) {
                    continue;
                }
                
                int64_t v = e.get_other(u);
                double weight = 0;
                
                if (e.status == UNKNOWN) {
                    if (e.p <= 1e-7) {
                        continue;
                    }
                    weight = -log(e.p);
                }

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.push({dist[v], v});
                }
            }
        }
        return expected_profit;
    }

    double evaluateMonteCarlo(int64_t start_node, int64_t sim_count = 500) {
        double total_profit = 0;
        std::vector<int> visited_sim(n, -1);
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        for (int64_t sim = 0; sim < sim_count; ++sim) {
            double current_sim_profit = 0;
            std::queue<int64_t> q;
            q.push(start_node);
            visited_sim[start_node] = sim;

            while (!q.empty()) {
                int64_t u = q.front();
                q.pop();
                
                if (!rooms[u].visited) {
                    current_sim_profit += rooms[u].profit;
                }

                for (auto e_idx : adj[u]) {
                    Edge e = edges[e_idx];
                    
                    if (e.status == BROKEN || e.status == UNKNOWN && dist(rng) > e.p) {
                        continue;
                    }

                    int64_t v = e.get_other(u);
                    if (visited_sim[v] != sim) {
                        visited_sim[v] = sim;
                        q.push(v);
                    }
                }
            }
            total_profit += current_sim_profit;
        }
        return total_profit / sim_count;
    }

    int64_t countCyclesInSubgraph(int64_t start_node) {
        std::queue<int64_t> q;
        std::vector<bool> local_vis(n, false);
        
        q.push(start_node);
        local_vis[start_node] = true;

        int64_t v_count = 0;
        int64_t e_count = 0;

        while (!q.empty()) {
            int64_t u = q.front();
            q.pop();
            ++v_count;

            for (auto e_idx : adj[u]) {
                Edge e = edges[e_idx];
                
                if (e.status == BROKEN) {
                    continue;
                }
                
                int64_t v = e.get_other(u);
                
                if (rooms[v].visited) {
                    continue;
                }

                ++e_count;

                if (!local_vis[v]) {
                    local_vis[v] = true;
                    q.push(v);
                }
            }
        }
        
        e_count /= 2;
        return e_count - v_count + 1;
    }

    double evaluateClearEV(int64_t e_idx) {
        Edge e = edges[e_idx];
        int64_t target_node = rooms[e.u].visited ? e.v : e.u;

        int64_t cycles = countCyclesInSubgraph(target_node);
        double expected_profit = 0;
        
        if (cycles <= 2) { 
            expected_profit = evaluateDijkstra(target_node);
        } else {
            expected_profit = evaluateMonteCarlo(target_node, 1000);
        }

        return expected_profit - clear_cost;
    }

    void run() {
        while (true) {
            exploreKnownZone();

            int64_t best_edge = -1;
            double max_ev = 0.0;

            for (int64_t i = 0; i < m; ++i) {
                if (edges[i].status == BROKEN && rooms[edges[i].u].visited ^ rooms[edges[i].v].visited) {
                    double ev = evaluateClearEV(i);
                    if (ev > max_ev) {
                        max_ev = ev;
                        best_edge = i;
                    }
                }
            }

            if (best_edge != -1 && max_ev > 0) {
                sendClear(edges[best_edge].u, edges[best_edge].v);
            } else {
                sendEnd();
            }
        }
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    int64_t n;
    int64_t m;
    int64_t s;
    double c;
    std::cin >> n >> m >> c >> s;

    MineExplorer explorer(n, m, c, s);

    for (int64_t i = 0; i < n; ++i) {
        double profit;
        std::cin >> profit;
        explorer.setProfit(i, profit);
    }

    for (int64_t i = 0; i < m; ++i) {
        int64_t u;
        int64_t v;
        double p;
        std::cin >> u >> v >> p;
        explorer.addEdge(u, v, p);
    }

    explorer.run();
}