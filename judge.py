import subprocess
import sys

def run_test(executable, test_file):
    with open(test_file, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]

    n, m, c, s = map(float, lines[0].split())
    n, m, s = int(n), int(m), int(s)
    
    profits = []
    for i in range(1, n + 1):
        profits.append(float(lines[i]))
        
    edges = []
    true_states = {}
    for i in range(n + 1, n + 1 + m):
        parts = lines[i].split()
        u, v = int(parts[0]), int(parts[1])
        p = float(parts[2])
        is_ok = int(parts[3]) == 1
        edges.append((u, v, p))
        true_states[(u, v)] = is_ok
        true_states[(v, u)] = is_ok

    proc = subprocess.Popen([executable], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

    proc.stdin.write(f"{n} {m} {c} {s}\n")
    for p in profits:
        proc.stdin.write(f"{p}\n")
    for u, v, p in edges:
        proc.stdin.write(f"{u} {v} {p}\n")
    proc.stdin.flush()

    total_profit = 0.0
    days_spent = 0
    current_room = s
    visited_rooms = {s}
    total_profit = float(profits[s])
    days_spent = 0

    print(f"--- Запуск теста {test_file} ---")

    try:
        while True:
            cmd_line = proc.stdout.readline().strip()
            if not cmd_line:
                break
            
            parts = cmd_line.split()
            cmd = parts[0]

            if cmd == "LOOK":
                adj = []
                for u, v, _ in edges:
                    if u == current_room: adj.append(v)
                    elif v == current_room: adj.append(u)
                
                for v in adj:
                    state = "OK" if true_states[(current_room, v)] else "BROKEN"
                    proc.stdin.write(f"{v} {state}\n")
                proc.stdin.flush()

            elif cmd == "MOVE":
                v = int(parts[1])
                if (current_room, v) in true_states and true_states[(current_room, v)]:
                    current_room = v
                    if v not in visited_rooms:
                        visited_rooms.add(v)
                        total_profit += profits[v]
                    proc.stdin.write("OK\n")
                else:
                    proc.stdin.write("FAIL\n")
                proc.stdin.flush()

            elif cmd == "CLEAR":
                u, v = int(parts[1]), int(parts[2])
                days_spent += 1
                true_states[(u, v)] = True
                true_states[(v, u)] = True
                current_room = s
                proc.stdin.write("OK\n")
                proc.stdin.flush()

            elif cmd == "END":
                print("Экспедиция завершена.")
                break
            else:
                print(f"Неизвестная команда: {cmd_line}")
                break

    except Exception as e:
        print(f"Ошибка взаимодействия: {e}")

    proc.terminate()
    
    final_score = total_profit - (days_spent * c)
    print(f"Комнат посещено: {len(visited_rooms)}")
    print(f"Дней потрачено: {days_spent}")
    print(f"Собрано золота: {total_profit}")
    print(f"ИТОГОВАЯ ПРИБЫЛЬ: {final_score}\n")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Использование: python3 judge.py ./solution test.txt")
    else:
        run_test(sys.argv[1], sys.argv[2])