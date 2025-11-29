// 11327107 謝芳倪 11327115 郭琮禮

#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <fstream>  // 檔案讀取
#include <sstream>
#include <chrono> // 計時 微秒
#include <iomanip>
#include <algorithm> // for std::max


using namespace std::chrono;

void Start() {
  std::cout << std::endl;
  std::cout << "*** (^_^) Data Structure (^o^) ***" << std::endl;
  std::cout << "** Simulate FIFO Queues by SQF ***" << std::endl;
  std::cout << "* 0. Quit                        *" << std::endl;
  std::cout << "* 1. Sort a file                 *" << std::endl;
  std::cout << "* 2. Simulate one FIFO queue     *" << std::endl;
  std::cout << "* 3. Simulate two queues by SQF  *" << std::endl;
  std::cout << "* 4. Simulate some queues by SQF *" << std::endl;
  std::cout << "**********************************" << std::endl;
  std::cout << "Input a command(0, 1, 2, 3, 4): ";
}

struct Order {
  int OID;
  int arrival;
  int duration;
  int timeout;
};

struct Abortlist {
  int OID;
  int CID;
  int delay;
  int abort;
};

struct Timeout {
  int OID;
  int CID;
  int delay;
  int departure;
};

bool Loadfile(std::string &file, Order* &arr, int &quantity) {
  std::ifstream fin(file);
  if (!fin) { 
    std::cout << std::endl;
    std::cout << "### " << file << " does not exist! ###";
    return false;
  }
  
  std::string line;
  getline(fin, line); // 跳過欄位名稱

  quantity = 0;
  while ( getline(fin, line) ) { // 計算資料有幾筆
    if ( !line.empty() ) {
      quantity++;
    }
  } 

  arr = new Order[quantity];

  // 重置檔案指標
  fin.clear();
  fin.seekg(0, std::ios::beg);

  getline(fin, line); // 跳過欄位名稱

  int num = 0;
  while ( getline(fin, line) ) {
    std::stringstream ss(line);
    ss >> arr[num].OID >> arr[num].arrival >> arr[num].duration >> arr[num].timeout;
    num++;
  }

  fin.close();
  return true;
}

void Print(Order *arr, int quantity) {
  std::cout << std::endl;
  std::cout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl;
  for ( int i = 0; i < quantity; i++ ) {
    std::cout << "(" << i + 1 << ")\t"
              << arr[i].OID << "\t"
              << arr[i].arrival << "\t"
              << arr[i].duration << "\t"
              << arr[i].timeout << std::endl;
  }
  std::cout << std::endl;
}

void SetSortFile(Order *arr, int quantity, std::string &sortfile) {
  std::ofstream fout(sortfile);
  fout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl;
  for ( int i = 0; i < quantity; i++ ) { 
    fout  << arr[i].OID << "\t"
          << arr[i].arrival << "\t"
          << arr[i].duration << "\t"
          << arr[i].timeout << std::endl;
  }
  fout.close();
}

void ShellSort(Order *arr, int quantity) {
  for ( int gap = quantity / 2; gap > 0; gap /= 2 ) {
    for ( int i = gap; i < quantity; i++ ) {
      Order temp = arr[i];
      int j = i;

      while ( j >= gap && ( (arr[j - gap].arrival > temp.arrival) || 
                            (arr[j - gap].arrival == temp.arrival && arr[j - gap].OID > temp.OID) ) ) {
                              arr[j] = arr[j - gap];
                              j = j - gap;
                            }
      arr[j] = temp;
    }
  }
}

void SaveAndShort(const std::string com) {
  Order *arr = nullptr;
  int quantity = 0;
  std::string filename = "input" + com + ".txt";
  std::string sortfile = "sorted" + com + ".txt";

  // 計讀檔時間
  auto start = high_resolution_clock::now();
  if (!Loadfile(filename, arr, quantity)) {
    return;
  }
  auto end = high_resolution_clock::now();
  long long reading = duration_cast<microseconds>(end - start).count();

  Print(arr, quantity);

  auto start1 = high_resolution_clock::now();
  ShellSort(arr, quantity);
  auto end1 = high_resolution_clock::now();
  long long sorting = duration_cast<microseconds>(end1 - start1).count();

  auto start2 = high_resolution_clock::now();
  SetSortFile(arr, quantity, sortfile);
  auto end2 = high_resolution_clock::now();
  long long writing = duration_cast<microseconds>(end2 - start2).count();

  std::cout << "Reading data: " << reading << " us.\n" << std::endl;
  std::cout << "Sorting data: " << sorting << " us.\n" << std::endl;
  std::cout << "Writing data: " << writing << " us.";
  
  
}

class Queue {
private:
    Order* data;
    int capacity;
    int front;
    int back;
    int count;
public:
    Queue(int cap = 3) : capacity(cap), front(0), back(0), count(0) {
        data = new Order[capacity];
    }
    ~Queue() {
        delete[] data;
    }

    bool empty() const { 
      return count == 0;
    }
    bool full()  const { 
      return count == capacity; 
    }

    bool push(const Order &order) {
        if (full()) {
          return false;
        }
        data[back] = order;
        back = (back + 1) % capacity;
        ++count;
        return true;
    }

    bool pop(Order &order) {
        if (empty()) {
          return false;
        }
        order = data[front];
        front = (front + 1) % capacity;
        --count;
        return true;
    }

    int size() const { 
      return count; 
    }
};


void SetOneFile(Order *arr, int n, std::string com) {
  std::string onefile = "one" + com + ".txt";
  std::ofstream fout(onefile);
  Queue q;
  int idletime = 0; // 閒置時刻
  int idx = 0; 

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  // 計算有效訂單總數
  int validnum = 0;
  for(int i = 0; i < n; i++) {
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) {
      validnum++;
    }
  }

  while (idx < n || !q.empty()) {  
        
    // 確保arr[idx]是有效的
    while (idx < n && (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      idx++;
    }
        
    // 空佇列
    if (q.empty()) {
      // 閒置是下單時刻
      if (idx < n && idletime < arr[idx].arrival) {
        idletime = arr[idx].arrival;
      }
            
      // 補訂單，因為佇列剛空，補進第一筆訂單就跳出
      while (idx < n && arr[idx].arrival <= idletime) {
        // 過濾無效單
        if (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout) {
          idx++; 
          continue; 
        }

        q.push(arr[idx]);
        idx++;
                
        break;
      }

      // 如果補完還是空的，代表後面沒單了
      if (q.empty()) {
        if (idx >= n) {
          break;
        }

        else {
          continue;
        }
      }
    }

    // 佇列有訂單，優先處理
    Order cur;
    q.pop(cur);

    // 取出時就逾時
    if (cur.timeout < idletime) {
      int delay = idletime - cur.arrival;
      abortList.push_back({cur.OID, 1, delay, idletime});
      continue;
    }

    // 餐點完成時間
    int finishtime = idletime + cur.duration;

    // 補進 < finishtime 的單
    while (idx < n && arr[idx].arrival < finishtime) {
      // 過濾無效單
      if (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout) {
        idx++; 
        continue;
      }

      if (q.full()) { // 佇列已滿，取消訂單
        abortList.push_back({arr[idx].OID, 0, 0, arr[idx].arrival});
      } 
      
      else {
        q.push(arr[idx]);
      }
      idx++;
    }

    idletime = finishtime;
    if (cur.timeout < finishtime) { // 餐點逾時
      int delay = (idletime - cur.duration) - cur.arrival;
      timeoutList.push_back({cur.OID, 1, delay, idletime});
    }
  }

  int totaldelay = 0;
  for (int i = 0; i < abortList.size(); i++) {
    totaldelay += abortList[i].delay;
  }

  for (int i = 0; i < timeoutList.size(); i++) {
    totaldelay += timeoutList[i].delay;
  } 

  double failurePercent = 0.0;
  if (validnum > 0) {
    double temp = (double)(abortList.size() + timeoutList.size()) / validnum * 100.0;
    failurePercent = int(temp * 100 + 0.5) / 100.0;
  }


    // Abort List
  fout << "\t[Abort List]\n";
  fout << "\tOID\tCID\tDelay\tAbort\n";
  for (int i = 0; i < abortList.size(); i++) {
    fout << "[" << (i + 1) << "]\t" 
         << abortList[i].OID << "\t" 
         << abortList[i].CID << "\t" 
         << abortList[i].delay << "\t" 
         << abortList[i].abort << "\n";
  }

    // Timeout List
    fout << "\t[Timeout List]\n";
    fout << "\tOID\tCID\tDelay\tDeparture\n";
    for (int i = 0; i < timeoutList.size(); i++) {
        fout << "[" << (i + 1) << "]\t" 
             << timeoutList[i].OID << "\t" 
             << timeoutList[i].CID << "\t" 
             << timeoutList[i].delay << "\t" 
             << timeoutList[i].departure << "\n";
    }

    fout << "[Total Delay]" << std::endl;
    fout << totaldelay << " min." << std::endl;;
    
    fout << "[Failure Percentage]" << std::endl;
    fout << std::fixed << std::setprecision(2) << failurePercent << " %\n"; // 小數點後兩位
    fout.close();
}

// 幫某一位廚師處理佇列中的訂單，直到他的 idleTime 超過 limitTime 為止
void processChefUntil(int chefId, int limitTime,
                      Queue &q, int &idleTime,
                      std::vector<Abortlist> &abortList,
                      std::vector<Timeout> &timeoutList) {
  // chefId: 0-based，實際 CID = chefId + 1
  while (!q.empty() && idleTime <= limitTime) {
    // 先取出佇列最前面的訂單
    Order cur;
    q.pop(cur);

    // 真正開始處理這筆訂單的時間：
    // 不能早於廚師目前的閒置時刻，也不能早於訂單的到達時刻
    int startTime = std::max(idleTime, cur.arrival);

    // 取出時就發現已經逾時：Timeout < 取出時刻(startTime)
    if (cur.timeout < startTime) {
      int delay = startTime - cur.arrival;  // Abort - Arrival
      abortList.push_back({cur.OID, chefId + 1, delay, startTime});
      idleTime = startTime;                 // 廚師時間跳到這個取消時刻
      continue;
    }

    // 可以開始做，計算完成時間
    int finishTime = startTime + cur.duration;
    idleTime = finishTime;   // 廚師完成這道菜的時刻

    // 做完才發現逾時：Timeout < 完成時刻
    if (cur.timeout < finishTime) {
      int delay = startTime - cur.arrival;  // 取出時刻 - Arrival
      timeoutList.push_back({cur.OID, chefId + 1, delay, finishTime});
    }
    // 沒逾時：只更新 idleTime，不需記錄
  }
}


void SimulateMultiQueues(Order* arr, int n, int N,
                         const std::string& prefix,
                         const std::string& com) {
  // 輸出檔名：prefix + 檔案編號，例如 "two401.txt"、"any402.txt"
  std::string outFile = prefix + com + ".txt";
  std::ofstream fout(outFile);

  // N 位廚師，各自有 idleTime 與 Queue
  int* idleTime = new int[N];
  Queue* qs     = new Queue[N];

  for (int i = 0; i < N; ++i) {
    idleTime[i] = 0;
  }

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  // 計算有效訂單總數（和 SetOneFile 一致）
  int validnum = 0;
  for (int i = 0; i < n; ++i) {
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) {
      ++validnum;
    }
  }

  int idx = 0; // 指向下一筆要處理的訂單 index

  while (true) {
    // 跳過無效訂單
    while (idx < n &&
          (arr[idx].duration <= 0 ||
           (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      ++idx;
    }

    // 檢查所有佇列是否為空
    bool allEmpty = true;
    for (int i = 0; i < N; ++i) {
      if (!qs[i].empty()) {
        allEmpty = false;
        break;
      }
    }

    // 若沒有剩餘有效訂單且所有佇列皆空 -> 模擬結束
    if (idx >= n && allEmpty) {
      break;
    }

    // 下一個抵達的訂單時間
    int nextArrival;
    if (idx < n) nextArrival = arr[idx].arrival;
    else         nextArrival = std::numeric_limits<int>::max();

    // Step A: 舊訂單先處理到 nextArrival 之前
    for (int i = 0; i < N; ++i) {
      if (!qs[i].empty()) {
        // 只要這位廚師在 nextArrival 之前有空，就讓他從 queue 取訂單來處理
        if (idleTime[i] <= nextArrival) {
          processChefUntil(i, nextArrival, qs[i], idleTime[i], abortList, timeoutList);
        }
      }
    }

    // 若已沒有新訂單（idx >= n），這一輪 Step A 已經把剩餘 queue 中的訂單處理到 limitTime，
    // 再檢查一次是否全空，如果不是，下一輪 limitTime 會是 INT_MAX，會把全部做完。
    if (idx >= n) {
      // 直接進入下一輪 while，等待 allEmpty == true 觸發 break
      continue;
    }

    // Step B: 在 nextArrival 時刻處理新的訂單（可能有多筆 arrival 相同）
    while (idx < n && arr[idx].arrival == nextArrival) {
      Order &cur = arr[idx];

      // 再次確認是否有效（理論上前面已經過濾）
      if (cur.duration <= 0 || cur.arrival + cur.duration > cur.timeout) {
        ++idx;
        continue;
      }

      // 找出閒置且 queue 為空的廚師（Case1 / Case2 使用）
      std::vector<int> idleChefs;
      for (int i = 0; i < N; ++i) {
        if (idleTime[i] <= nextArrival && qs[i].empty()) {
          idleChefs.push_back(i);
        }
      }

      int chosen = -1; // -1 表示還沒選出廚師

      if (idleChefs.size() == 1) {
        // Case 1: 只有一位廚師閒置且佇列空
        chosen = idleChefs[0];
      } else if (idleChefs.size() > 1) {
        // Case 2: 不只一位廚師閒置，選編號最小
        chosen = idleChefs[0]; // idleChefs 按 i 遞增加入
      } else {
        // 沒有閒置廚師，進入 Case 3 / Case 4 判斷

        // 先檢查是否所有佇列都滿
        bool allFull = true;
        for (int i = 0; i < N; ++i) {
          if (!qs[i].full()) {
            allFull = false;
            break;
          }
        }

        if (allFull) {
          // Case 4: 每位廚師都不閒置且所有佇列皆滿 -> 立刻取消，CID = 0
          abortList.push_back({cur.OID, 0, 0, cur.arrival});
        } else {
          // Case 3: 至少一個佇列未滿 -> 選佇列長度最短的，若有多個取編號最小
          int bestLen = std::numeric_limits<int>::max();
          for (int i = 0; i < N; ++i) {
            if (!qs[i].full()) {
              int len = qs[i].size();
              if (len < bestLen) {
                bestLen = len;
                chosen = i;
              }
            }
          }
        }
      }

      if (chosen != -1) {
        // 有選到廚師 -> 丟進他的 queue
        qs[chosen].push(cur);
      }

      ++idx; // 處理下一筆 arrival == nextArrival 的訂單
    }

    // 回到 while 開頭，繼續下一輪（下一個 arrival 或處理剩餘 queue）
  }

  // 所有訂單模擬完成後：計算 total delay 與 failure percentage
  int totaldelay = 0;
  for (int i = 0; i < (int)abortList.size(); ++i) {
    totaldelay += abortList[i].delay;
  }
  for (int i = 0; i < (int)timeoutList.size(); ++i) {
    totaldelay += timeoutList[i].delay;
  }

  double failurePercent = 0.0;
  if (validnum > 0) {
    double temp = (double)(abortList.size() + timeoutList.size()) / validnum * 100.0;
    failurePercent = int(temp * 100 + 0.5) / 100.0;
  }

  // === 寫檔 ===
  fout << "\t[Abort List]\n";
  fout << "\tOID\tCID\tDelay\tAbort\n";
  for (int i = 0; i < (int)abortList.size(); ++i) {
    fout << "[" << (i + 1) << "]\t"
         << abortList[i].OID << "\t"
         << abortList[i].CID << "\t"
         << abortList[i].delay << "\t"
         << abortList[i].abort << "\n";
  }

  fout << "\t[Timeout List]\n";
  fout << "\tOID\tCID\tDelay\tDeparture\n";
  for (int i = 0; i < (int)timeoutList.size(); ++i) {
    fout << "[" << (i + 1) << "]\t"
         << timeoutList[i].OID << "\t"
         << timeoutList[i].CID << "\t"
         << timeoutList[i].delay << "\t"
         << timeoutList[i].departure << "\n";
  }

  fout << "[Total Delay]\n";
  fout << totaldelay << " min.\n";
  fout << "[Failure Percentage]\n";
  fout << std::fixed << std::setprecision(2) << failurePercent << " %\n";

  fout.close();
  delete[] idleTime;
  delete[] qs;
}

int main() {
  Start();                
  std::string com;
  int command;
  Order *arr = nullptr;
  int quantity = 0;
  bool file_loaded = false;          // 有沒有成功讀過 sorted 檔
  std::string loaded_file_number;    // 最近一次 command 2 讀的是哪個檔號 (例如 "401")

  while (1) {
    std::getline(std::cin, com);
    if (com.empty()) {
      continue;
    }

    std::stringstream ss(com);
    if (!(ss >> command) || !(ss.eof()) || command < 0 || command > 4) {
      std::cout << std::endl << "Command does not exist!" << std::endl;
      Start();
      continue;
    }

    if (command == 0) {
      // 結束前記得釋放動態陣列
      delete[] arr;
      return 0;
    } 
    
    else if (command == 1) {
      std::cout << std::endl;
      std::cout << "Input a file number (e.g., 401, 402, 403, ...): ";
      std::getline(std::cin, com);
      while (com.empty()) {
        std::getline(std::cin, com);
      }
      SaveAndShort(com);   // 這裡會產生 sortedXXX.txt
      std::cout << std::endl;
      Start();
      continue;
    }

    else if (command == 2) {
      std::cout << std::endl;
      std::cout << "Input a file number (e.g., 401, 402, 403, ...): ";
      std::getline(std::cin, com);
      while (com.empty()) {
        std::getline(std::cin, com);
      }

      std::string sortfile = "sorted" + com + ".txt";

      // 若之前已經有讀過 arr，要先釋放
      if (arr != nullptr) {
        delete[] arr;
        arr = nullptr;
        quantity = 0;
      }

      if (!Loadfile(sortfile, arr, quantity)) {
        std::cout << std::endl;
        Start();
        continue;
      }

      // 記錄這次成功載入的檔案編號
      loaded_file_number = com;
      file_loaded = true;

      Print(arr, quantity);
      SetOneFile(arr, quantity, com);   // 任務二模擬

      Start();
      continue;
    }

    else if (command == 3) {
      // 任務三：一定要先有任務二載入過資料
      if (!file_loaded) {
        std::cout << "\n### sorted" 
              << (loaded_file_number.empty() ? "???" : loaded_file_number) 
              << ".txt does not exist! ###\n";
        Start();
        continue;
      }

      // 這裡直接用 arr, quantity, loaded_file_number
      // N 固定為 2，prefix 為 "two"（產生 twoXXX.txt）
      SimulateMultiQueues(arr, quantity, 2, "two", loaded_file_number);

      Start();
      continue;
    }

    else if (command == 4) {
      // 任務四：同樣要先跑過 2
      if (!file_loaded) {
        std::cout << "\n### sorted" 
              << (loaded_file_number.empty() ? "???" : loaded_file_number) 
              << ".txt does not exist! ###\n";
        Start();
        continue;
      }

      int N = 0;
      std::string tmp;
      std::cout << "\nInput the number of queues: ";
      std::getline(std::cin, tmp);
      while (tmp.empty()) std::getline(std::cin, tmp);

      std::stringstream ssN(tmp);
      if (!(ssN >> N) || N <= 0) {
        std::cout << "Invalid N!\n";
        Start();
        continue;
      }

      std::string prefix;
      if (N == 1) prefix = "one";
      else if (N == 2) prefix = "two";
      else prefix = "any";

      SimulateMultiQueues(arr, quantity, N, prefix, loaded_file_number);

      Start();
      continue;
    }
  }
}
