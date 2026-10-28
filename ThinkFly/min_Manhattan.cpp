#include <iostream>
#include <vector>
#include <queue>
#include <string>
using namespace std;

// 无人机从 S 到 T 的最短步数（BFS，因有障碍物，不能简单用曼哈顿距离）
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<string> g(n);
    int sx = -1, sy = -1, tx = -1, ty = -1;

    for (int i = 0; i < n; ++i) {
        cin >> g[i];
        for (int j = 0; j < m; ++j) {
            if (g[i][j] == 'S') { sx = i; sy = j; }
            else if (g[i][j] == 'T') { tx = i; ty = j; }
        }
    }

    // dist 初始化为 -1，表示未访问
    vector<vector<int>> dist(n, vector<int>(m, -1));

    queue<pair<int, int>> q;
    q.push({sx, sy});
    dist[sx][sy] = 0;

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        pair<int, int> cur = q.front();
        q.pop();
        int x = cur.first, y = cur.second;

        if (x == tx && y == ty) break; // 已到达目标，可提前结束

        for (int k = 0; k < 4; ++k) {
            int nx = x + dx[k];
            int ny = y + dy[k];
            if (nx < 0 || nx >= n || ny < 0 || ny >= m) continue;
            if (g[nx][ny] == '#') continue;
            if (dist[nx][ny] != -1) continue;

            dist[nx][ny] = dist[x][y] + 1;
            q.push({nx, ny});
        }
    }

    cout << dist[tx][ty] << '\n';
    return 0;
}
