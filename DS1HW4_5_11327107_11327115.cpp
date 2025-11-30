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
    std::cout << "(" << i + 1 << ") \t"
              << arr[i].OID << "\t"
              << arr[i].arrival << "\t"
              << arr[i].duration << "\t"
              << arr[i].timeout << std::endl;
  }
}

void SetSortFile(Order *arr, int quantity, std::string &sortfile) {
  std::ofstream fout(sortfile);                              // 建立 sortedXXX.txt 輸出檔

  fout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl; // 寫入標題

  for ( int i = 0; i < quantity; i++ ) {                    // 依序寫入每筆排序後資料
    fout << arr[i].OID << "\t"
         << arr[i].arrival << "\t"
         << arr[i].duration << "\t"
         << arr[i].timeout << std::endl;
  }

  fout.close();                                              // 關閉檔案
}

void ShellSort(Order *arr, int quantity) {
  for ( int gap = quantity / 2; gap > 0; gap /= 2 ) {        // 逐步縮小 gap（Shell Sort 標準流程）
    for ( int i = gap; i < quantity; i++ ) {
      Order temp = arr[i];                                   // 欲插入的元素
      int j = i;

      while ( j >= gap &&                                    // 插入式比較（非交換法）
             ( (arr[j - gap].arrival > temp.arrival) ||      // 先比 arrival
               (arr[j - gap].arrival == temp.arrival && 
                arr[j - gap].OID > temp.OID) ) ) {           // arrival 相同再比 OID
        arr[j] = arr[j - gap];                               // 移動元素騰出插入位置
        j = j - gap;
      }

      arr[j] = temp;                                         // 插入正確位置
    }
  }
}

void SaveAndShort(const std::string com) {
  Order *arr = nullptr;
  int quantity = 0;
  std::string filename = "input" + com + ".txt"; 
  std::string sortfile = "sorted" + com + ".txt";

  auto start = high_resolution_clock::now();                 
  if (!Loadfile(filename, arr, quantity)) {                  // 讀失敗直接返回
    return;
  }
  auto end = high_resolution_clock::now();                   
  long long reading = duration_cast<microseconds>(end - start).count();

  Print(arr, quantity);                                      // 列印原始資料

  auto start1 = high_resolution_clock::now();                
  ShellSort(arr, quantity);                       
  auto end1 = high_resolution_clock::now();                 
  long long sorting = duration_cast<microseconds>(end1 - start1).count();

  auto start2 = high_resolution_clock::now();             
  SetSortFile(arr, quantity, sortfile);                      // 寫入 sortedXXX.txt
  auto end2 = high_resolution_clock::now();                 
  long long writing = duration_cast<microseconds>(end2 - start2).count();

  std::cout << "\nReading data: " << reading << " us.\n";   
  std::cout << "\nSorting data: " << sorting << " us.\n";   
  std::cout << "\nWriting data: " << writing << " us.";                  
}

class Queue {
 private:
  Order* data;           // 動態配置的陣列，用來存放訂單
  int capacity;          // 佇列最大容量（預設 3）
  int front;             // 佇列頭索引（取出位置）
  int back;              // 佇列尾索引（放入位置）
  int count;             // 目前佇列內的資料筆數

 public:
  Queue(int cap = 3) : capacity(cap), front(0), back(0), count(0) {
    data = new Order[capacity];   // 配置固定容量的循環佇列空間
  }

  ~Queue() {
    delete[] data;                // 釋放動態陣列
  }

  bool empty() { 
    return count == 0;            // 判斷佇列是否為空
  }

  bool full() { 
    return count == capacity;     // 判斷佇列是否已滿
  }

  bool push(const Order &order) {
    if (full()) {                 // 空間不足 → push 失敗
      return false;
    }

    data[back] = order;           // 將訂單放入尾端
    back = (back + 1) % capacity; // 循環佇列尾指標往後移
    count++;                      // 資料數量 +1
    return true;
  }

  bool pop(Order &order) {
    if (empty()) {                // 空佇列 → pop 失敗
      return false;
    }

    order = data[front];          // 取出頭端資料
    front = (front + 1) % capacity; // 循環佇列頭指標往後移
    count--;                      // 資料數量 -1
    return true;
  }

  int size() const { 
    return count;                 // 回傳佇列內目前的訂單數
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

void processChefUntil(int chefId, int limitTime,
                      Queue &q, int &idleTime,
                      std::vector<Abortlist> &abortList,
                      std::vector<Timeout> &timeoutList) {
  
  while (!q.empty() && idleTime <= limitTime) {
    Order cur;
    q.pop(cur); 

    int startTime = std::max(idleTime, cur.arrival);

    if (cur.timeout < startTime) {
      int delay = startTime - cur.arrival;
      abortList.push_back({cur.OID, chefId + 1, delay, startTime});
      idleTime = startTime;
      continue;
    }

    int finishTime = startTime + cur.duration;
    idleTime = finishTime;

    if (cur.timeout < finishTime) {
      int delay = startTime - cur.arrival;
      timeoutList.push_back({cur.OID, chefId + 1, delay, finishTime});
    }
  }
}

void SimulateMultiQueues(Order* arr, int n, int N,
                         const std::string& prefix,
                         const std::string& com) {
  std::string outFile = prefix + com + ".txt";
  std::ofstream fout(outFile);

  int* idleTime = new int[N];
  Queue* qs     = new Queue[N];
  for (int i = 0; i < N; ++i) idleTime[i] = 0;

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  int validnum = 0;
  for (int i = 0; i < n; ++i) {
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) ++validnum;
  }

  int idx = 0;
  std::vector<int> idleChefs;

  // 分配
  while (idx < n) {
    while (idx < n && (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) idx++;
    if (idx >= n) break;

    int nextArrival = arr[idx].arrival;

    // 先處理所有佇列直到 nextArrival
    for (int i = 0; i < N; ++i) {
      if (!qs[i].empty() && idleTime[i] <= nextArrival) {
        processChefUntil(i, nextArrival, qs[i], idleTime[i], abortList, timeoutList);
      }
    }

    // 分配新訂單
    while (idx < n && arr[idx].arrival == nextArrival) {
      Order &cur = arr[idx];
      if (cur.duration <= 0 || cur.arrival + cur.duration > cur.timeout) { idx++; continue; }

      idleChefs.clear();
      for (int i = 0; i < N; ++i) {
        if (idleTime[i] <= nextArrival && qs[i].empty()) idleChefs.push_back(i);
      }

      int chosen = -1;
      if (idleChefs.size() >= 1) {
        chosen = idleChefs[0];
      } else {
        bool allFull = true;
        for (int i = 0; i < N; ++i) if (!qs[i].full()) { allFull = false; break; }
        if (allFull) {
          abortList.push_back({cur.OID, 0, 0, cur.arrival});
        } else {
          int bestLen = std::numeric_limits<int>::max();
          for (int i = 0; i < N; ++i) {
            if (!qs[i].full()) {
              int len = qs[i].size();
              if (len < bestLen) { bestLen = len; chosen = i; }
            }
          }
        }
      }

      if (chosen != -1) {
        qs[chosen].push(cur);
        if (idleTime[chosen] <= cur.arrival) {
             processChefUntil(chosen, cur.arrival, qs[chosen], idleTime[chosen], abortList, timeoutList);
        }
      }
      idx++;
    }
  }

  // 清空
  // 將訂單 pop 出來暫存在 curOrder 中
  std::vector<Order> curOrder(N);
  std::vector<bool> hasOrder(N, false);

  while (true) {
      bool anyBusy = false;
      for (int i = 0; i < N; ++i) {
          if (!hasOrder[i] && !qs[i].empty()) {
              qs[i].pop(curOrder[i]); 
              hasOrder[i] = true;
          }
          if (hasOrder[i]) anyBusy = true;
      }

      if (!anyBusy) break;

      // 找出所有「手上拿著訂單」的廚師中，完成時間最早的
      int minEventTime = std::numeric_limits<int>::max();
      int targetChef = -1;

      for (int i = 0; i < N; ++i) {
          if (hasOrder[i]) {
              Order &order = curOrder[i];
              int start = std::max(idleTime[i], order.arrival);
              int eventTime;
              
              if (order.timeout < start) {
                  eventTime = start; // Abort Time
              } else {
                  eventTime = start + order.duration; // Departure Time
              }

              if (eventTime < minEventTime) {
                  minEventTime = eventTime;
                  targetChef = i;
              }
          }
      }

      // 處理選中的那位廚師的訂單
      if (targetChef != -1) {
          Order &cur = curOrder[targetChef];
          int start = std::max(idleTime[targetChef], cur.arrival);
          
          if (cur.timeout < start) {
              int delay = start - cur.arrival;
              abortList.push_back({cur.OID, targetChef + 1, delay, start});
              idleTime[targetChef] = start;
          } else {
              int finishTime = start + cur.duration;
              idleTime[targetChef] = finishTime;
              if (cur.timeout < finishTime) {
                  int delay = start - cur.arrival;
                  timeoutList.push_back({cur.OID, targetChef + 1, delay, finishTime});
              }
          }
          // 標記該廚師手上的訂單已處理完畢，下次迴圈可以再從 queue 拿新的
          hasOrder[targetChef] = false;
      }
  }


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
    // 只要能讀出一個整數就當作合法指令，像 3.14 會被當成 3
    if (!(ss >> command)) {
      // 連一個整數都讀不到，直接結束程式
      if (arr != nullptr) {
        delete[] arr;
      }
      return 0;
    }

    if (command < 0 || command > 4) {
      // 是整數，但不在 [0, 4] 之內 → 印提示
      std::cout << std::endl << "Command does not exist!" << std::endl;
      Start();
      continue;
    }

    if (command == 0) {
      // 結束前釋放動態陣列
      if (arr != nullptr) {
        delete[] arr;
      }
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
        std::cout << "\n### Execute command 2 first! ###\n";
        Start();
        continue;
      }

      // N 固定為 2，prefix 為 "two"（產生 twoXXX.txt）
      SimulateMultiQueues(arr, quantity, 2, "two", loaded_file_number);
      Start();
      continue;
    }

    else if (command == 4) {
      // 任務四：同樣要先跑過 2
      if (!file_loaded) {
        std::cout << "\n### Execute command 2 first! ###\n";
        Start();
        continue;
      }

      int N = 0;
      std::cout << "\nInput the number of queues: ";
      while(1) {
        std::string tmp;
        std::getline(std::cin, tmp);

        if (tmp.empty()) {
          continue;
        }

        std::stringstream ssN(tmp);
        if (!(ssN >> N) || N < 0) {
          std::cout << "\nInput the number of queues: ";
          continue;
        }

        if (N == 0 || N > 19) {
          std::cout << "\n### It is NOT in [1,19] ###\n";
          std::cout << "\nInput the number of queues: ";
          continue;
        }

        if (N > 0 && N <= 19) {
          break;
        }
      }

      std::string prefix;
      if (N == 1) {
        prefix = "one";
      } else if (N == 2) {
        prefix = "two";
      } else {
        prefix = "any";
      }

      SimulateMultiQueues(arr, quantity, N, prefix, loaded_file_number);
      Start();
      continue;
    }
  }
}
