// 11327107 謝芳倪 11327115 郭琮禮

#include <iostream>
#include <string>
#include <limits>       // for std::numeric_limits
#include <vector>
#include <fstream>      // 檔案讀取/寫入
#include <sstream>
#include <chrono>       // 計時 微秒
#include <iomanip>
#include <algorithm>    // for std::max

using namespace std::chrono;

// 顯示主選單
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

// 訂單結構
struct Order {
  int OID;        // 訂單編號 (Order ID)
  int arrival;    // 到達時刻
  int duration;   // 製作所需時間
  int timeout;    // 逾時時刻
};

// 取消/放棄清單結構 (Abort List)
struct Abortlist {
  int OID;        // 訂單編號
  int CID;        // 處理的廚師編號 (Chef ID)，0 表示在排隊前被取消
  int delay;      // 延遲時間 (AbortTime - ArrivalTime)
  int abort;      // 取消時刻
};

// 逾時清單結構 (Timeout List)
struct Timeout {
  int OID;        // 訂單編號
  int CID;        // 處理的廚師編號 (Chef ID)
  int delay;      // 延遲時間 (StartTime - ArrivalTime)
  int departure;  // 完成/出餐時刻 (FinishTime)
};

// 讀取檔案內容到 Order 結構陣列
bool Loadfile(std::string &file, Order* &arr, int &quantity) {
  std::ifstream fin(file);
  if (!fin) { 
    std::cout << std::endl;
    std::cout << "### " << file << " does not exist! ###";
    return false; // 檔案不存在
  }
  
  std::string line;
  getline(fin, line); // 跳過欄位名稱

  quantity = 0;
  while ( getline(fin, line) ) { // 計算資料有幾筆
    if ( !line.empty() ) {
      quantity++;
    }
  } 

  arr = new Order[quantity]; // 根據數量分配記憶體

  // 重置檔案指標，準備重新讀取資料
  fin.clear();
  fin.seekg(0, std::ios::beg);

  getline(fin, line); // 再次跳過欄位名稱

  int num = 0;
  while ( getline(fin, line) ) {
    std::stringstream ss(line);
    // 從每一行讀取 OID, arrival, duration, timeout
    ss >> arr[num].OID >> arr[num].arrival >> arr[num].duration >> arr[num].timeout;
    num++;
  }

  fin.close();
  return true;
}

// 輸出 Order 結構陣列內容
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

// 將 Order 結構陣列內容寫入檔案
void SetSortFile(Order *arr, int quantity, std::string &sortfile) {
  std::ofstream fout(sortfile);
  fout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl;
  for ( int i = 0; i < quantity; i++ ) { 
    fout << arr[i].OID << "\t"
         << arr[i].arrival << "\t"
         << arr[i].duration << "\t"
         << arr[i].timeout << std::endl;
  }
  fout.close();
}

// 使用 Shell Sort 排序 Order 陣列
// 排序規則：主要依據 arrival time 升序，次要依據 OID 升序
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

// 命令 1 處理函數：讀取、排序並寫入檔案，並計時
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

  Print(arr, quantity); // 輸出原始資料

  // 計排序時間
  auto start1 = high_resolution_clock::now();
  ShellSort(arr, quantity);
  auto end1 = high_resolution_clock::now();
  long long sorting = duration_cast<microseconds>(end1 - start1).count();

  // 計寫檔時間
  auto start2 = high_resolution_clock::now();
  SetSortFile(arr, quantity, sortfile);
  auto end2 = high_resolution_clock::now();
  long long writing = duration_cast<microseconds>(end2 - start2).count();

  std::cout << "Reading data: " << reading << " us.\n" << std::endl;
  std::cout << "Sorting data: " << sorting << " us.\n" << std::endl;
  std::cout << "Writing data: " << writing << " us.";
  
  delete[] arr; // 釋放動態記憶體
}

// 實作環狀佇列 (Circular Queue)
class Queue {
 private:
  Order* data;
  int capacity; // 佇列最大容量
  int front;    // 隊頭索引
  int back;     // 隊尾索引
  int count;    // 當前元素數量

 public:
  Queue(int cap = 3) : capacity(cap), front(0), back(0), count(0) {
    data = new Order[capacity]; // 初始化動態陣列
  }

  ~Queue() {
    delete[] data; // 釋放記憶體
  }

  bool empty() { 
    return count == 0;
  }

  bool full() { 
    return count == capacity; 
  }

  // 入隊
  bool push(const Order &order) {
    if (full()) {
      return false;
    }

    data[back] = order;
    back = (back + 1) % capacity; // 環狀增加
    count++;
    return true;
  }

  // 出隊
  bool pop(Order &order) {
    if (empty()) {
      return false;
    }

    order = data[front];
    front = (front + 1) % capacity; // 環狀增加
    count--;
    return true;
  }

  // 回傳佇列長度
  int size() const { 
    return count; 
  }
};

// 命令 2 處理函數：單一 FIFO 佇列模擬
void SetOneFile(Order *arr, int n, std::string com) {
  std::string onefile = "one" + com + ".txt";
  std::ofstream fout(onefile);
  Queue q; // 預設容量 3
  int idletime = 0; // 閒置時刻 (即下一筆訂單開始處理的時間)
  int idx = 0;      // 正在處理的 arr 陣列索引

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  // 計算有效訂單總數 (duration > 0 且 arrival + duration <= timeout)
  int validnum = 0;
  for(int i = 0; i < n; i++) {
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) {
      validnum++;
    }
  }

  while (idx < n || !q.empty()) {  
        
    // 步驟 1: 跳過無效訂單
    while (idx < n && (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      idx++;
    }
        
    // 步驟 2: 佇列為空 (處理廚師閒置與新訂單到達)
    if (q.empty()) {
      // 若有新訂單且廚師閒置時間早於訂單到達時間，則廚師閒置到訂單到達
      if (idx < n && idletime < arr[idx].arrival) {
        idletime = arr[idx].arrival;
      }
            
      // 補進在 idletime 或更早到達的第一筆有效訂單
      while (idx < n && arr[idx].arrival <= idletime) {
        // 過濾無效單 (冗餘檢查)
        if (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout) {
          idx++; 
          continue; 
        }

        q.push(arr[idx]);
        idx++;
                
        break; // 每次只補一筆就跳出，確保優先處理
      }

      // 如果補完還是空的，且沒有更多訂單，則結束模擬
      if (q.empty()) {
        if (idx >= n) {
          break;
        }
        else {
          continue; // 繼續下一輪 while，等待下一筆訂單
        }
      }
    }

    // 步驟 3: 佇列有訂單，取出並處理
    Order cur;
    q.pop(cur);

    // 步驟 3a: 取出時就逾時 (Abort by Timeout)
    // 逾時時刻 < 取出時刻(idletime)
    if (cur.timeout < idletime) {
      int delay = idletime - cur.arrival;
      // CID = 1 (只有一個廚師)
      abortList.push_back({cur.OID, 1, delay, idletime}); 
      continue; // 跳過做菜，處理下一單
    }

    // 步驟 3b: 餐點完成時間
    int finishtime = idletime + cur.duration;

    // 步驟 3c: 處理在做菜期間到達的新訂單 (Arrival < FinishTime)
    while (idx < n && arr[idx].arrival < finishtime) {
      // 過濾無效單 (冗餘檢查)
      if (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout) {
        idx++; 
        continue;
      }

      if (q.full()) { // 佇列已滿，取消訂單 (Abort by Full Queue)
        // CID = 0，Delay = 0，Abort = Arrival
        abortList.push_back({arr[idx].OID, 0, 0, arr[idx].arrival});
      } 
      
      else {
        q.push(arr[idx]); // 成功入隊
      }
      idx++;
    }

    // 步驟 3d: 更新廚師閒置時間
    idletime = finishtime;
    
    // 步驟 3e: 檢查做完時是否逾時 (Timeout by Completion)
    if (cur.timeout < finishtime) { 
      int delay = (idletime - cur.duration) - cur.arrival; // StartTime - Arrival
      // CID = 1 (只有一個廚師)
      timeoutList.push_back({cur.OID, 1, delay, idletime});
    }
  }

  // 計算總延遲時間
  int totaldelay = 0;
  for (size_t i = 0; i < abortList.size(); i++) {
    totaldelay += abortList[i].delay;
  }
  for (size_t i = 0; i < timeoutList.size(); i++) {
    totaldelay += timeoutList[i].delay;
  } 

  // 計算失敗百分比
  double failurePercent = 0.0;
  if (validnum > 0) {
    double temp = (double)(abortList.size() + timeoutList.size()) / validnum * 100.0;
    // 四捨五入到小數點後兩位
    failurePercent = int(temp * 100 + 0.5) / 100.0; 
  }


    // 寫入 Abort List
  fout << "\t[Abort List]\n";
  fout << "\tOID\tCID\tDelay\tAbort\n";
  for (size_t i = 0; i < abortList.size(); i++) {
    fout << "[" << (i + 1) << "]\t" 
         << abortList[i].OID << "\t" 
         << abortList[i].CID << "\t" 
         << abortList[i].delay << "\t" 
         << abortList[i].abort << "\n";
  }

    // 寫入 Timeout List
    fout << "\t[Timeout List]\n";
    fout << "\tOID\tCID\tDelay\tDeparture\n";
    for (size_t i = 0; i < timeoutList.size(); i++) {
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

// 輔助函數：讓指定的廚師處理佇列中的訂單，直到他的 idleTime 超過 limitTime 為止
void processChefUntil(int chefId, int limitTime,
                      Queue &q, int &idleTime,
                      std::vector<Abortlist> &abortList,
                      std::vector<Timeout> &timeoutList) {
  // chefId: 0-based，實際 CID = chefId + 1
  while (!q.empty() && idleTime <= limitTime) {
    // 1. 先取出佇列最前面的訂單 (FIFO)
    Order cur;
    q.pop(cur);

    // 2. 真正開始處理這筆訂單的時間：StartTime
    // 不能早於廚師目前的閒置時刻(idleTime)，也不能早於訂單的到達時刻(cur.arrival)
    int startTime = std::max(idleTime, cur.arrival);

    // 3. 取出時就發現已經逾時：Timeout < 取出時刻(startTime) -> Abort
    if (cur.timeout < startTime) {
      int delay = startTime - cur.arrival;  // Abort - Arrival
      abortList.push_back({cur.OID, chefId + 1, delay, startTime});
      idleTime = startTime;                 // 廚師時間跳到這個取消時刻
      continue;
    }

    // 4. 可以開始做，計算完成時間
    int finishTime = startTime + cur.duration;
    idleTime = finishTime;   // 廚師完成這道菜的時刻

    // 5. 做完才發現逾時：Timeout < 完成時刻 -> Timeout
    if (cur.timeout < finishTime) {
      int delay = startTime - cur.arrival;  // 取出時刻 - Arrival
      timeoutList.push_back({cur.OID, chefId + 1, delay, finishTime});
    }
    // 沒逾時：只更新 idleTime，不需記錄
  }
}

// 命令 3/4 處理函數：多佇列 (N 廚師) 模擬 (Shortest Queue First, SQF)
void SimulateMultiQueues(Order* arr, int n, int N,
                         const std::string& prefix,
                         const std::string& com) {
  // 輸出檔名：prefix + 檔案編號，例如 "two401.txt"、"any402.txt"
  std::string outFile = prefix + com + ".txt";
  std::ofstream fout(outFile);

  // N 位廚師，各自有 idleTime 與 Queue
  int* idleTime = new int[N];  // 記錄每位廚師下一筆訂單開始處理的時間
  Queue* qs = new Queue[N];    // N 個 FIFO 佇列 (每個廚師對應一個)

  for (int i = 0; i < N; i++) {
    idleTime[i] = 0;
  }

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  // 計算有效訂單總數（和 SetOneFile 一致）
  int validnum = 0;
  for (int i = 0; i < n; i++) {
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) {
      validnum++;
    }
  }

  int idx = 0; // 指向下一筆要處理的 arr 陣列索引

  while (true) {
    // 步驟 1: 跳過無效訂單
    while (idx < n &&
          (arr[idx].duration <= 0 ||
           (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      idx++;
    }

    // 步驟 2: 檢查所有佇列是否為空
    bool allEmpty = true;
    for (int i = 0; i < N; i++) {
      if (!qs[i].empty()) {
        allEmpty = false;
        break;
      }
    }

    // 步驟 3: 若沒有剩餘有效訂單且所有佇列皆空 -> 模擬結束
    if (idx >= n && allEmpty) {
      break;
    }

    // 步驟 4: 找出下一個抵達的訂單時間 (nextArrival)
    int nextArrival;
    if (idx < n) {
      nextArrival = arr[idx].arrival;
    }
    else {
      // 若沒有新訂單，則設為最大值，讓廚師把隊列中剩下的訂單做完
      nextArrival = std::numeric_limits<int>::max();
    }  

    // Step A: 舊訂單先處理到 nextArrival 之前 (或 limitTime 之前)
    for (int i = 0; i < N; i++) {
      if (!qs[i].empty()) {
        // 只要這位廚師在 nextArrival 之前有空，就讓他從 queue 取訂單來處理
        // limitTime = nextArrival (新訂單到達之前)
        if (idleTime[i] <= nextArrival) { 
          processChefUntil(i, nextArrival, qs[i], idleTime[i], abortList, timeoutList);
        }
      }
    }

    // 若已沒有新訂單，這一輪 Step A 已經把剩餘 queue 中的訂單處理到 limitTime，
    // 下一輪會因為 allEmpty == true 而 break
    if (idx >= n) {
      continue; 
    }

    // Step B: 在 nextArrival 時刻處理新的訂單（可能有多筆 arrival 相同）
    while (idx < n && arr[idx].arrival == nextArrival) {
      Order &cur = arr[idx];

      // 再次確認是否有效（理論上前面已經過濾）
      if (cur.duration <= 0 || cur.arrival + cur.duration > cur.timeout) {
        idx++;
        continue;
      }

      // 找出閒置且 queue 為空的廚師（Case1 / Case2 使用）
      std::vector<int> idleChefs;
      for (int i = 0; i < N; i++) {
        // 閒置定義：idleTime <= nextArrival 且 佇列為空
        if (idleTime[i] <= nextArrival && qs[i].empty()) { 
          idleChefs.push_back(i);
        }
      }

      int chosen = -1; // -1 表示還沒選出廚師

      if (idleChefs.size() == 1) {
        // Case 1: 只有一位廚師閒置且佇列空 -> 選他
        chosen = idleChefs[0];
      } 
      
      else if (idleChefs.size() > 1) {
        // Case 2: 不只一位廚師閒置 -> 選編號最小 (i 遞增加入)
        chosen = idleChefs[0]; 
      } 
      
      else {
        // 沒有閒置廚師，進入 Case 3 / Case 4 判斷

        // 先檢查是否所有佇列都滿
        bool allFull = true;
        for (int i = 0; i < N; i++) {
          if (!qs[i].full()) {
            allFull = false;
            break;
          }
        }

        if (allFull) {
          // Case 4: 每位廚師都不閒置且所有佇列皆滿 -> 立刻取消 (Abort by Full Queue)
          // CID = 0，Delay = 0，Abort = Arrival
          abortList.push_back({cur.OID, 0, 0, cur.arrival}); 
        } 
        
        else {
          // Case 3: 至少一個佇列未滿 -> 選佇列長度最短的 (Shortest Queue First, SQF)
          int bestLen = std::numeric_limits<int>::max();
          for (int i = 0; i < N; i++) {
            if (!qs[i].full()) {
              int len = qs[i].size();
              if (len < bestLen) {
                bestLen = len;
                chosen = i; // 更新選擇的廚師編號
              }
              // 如果 len == bestLen，維持選編號最小的 (i 最小)
            }
          }
        }
      }

      if (chosen != -1) {
        // 有選到廚師 -> 丟進他的 queue
        qs[chosen].push(cur);
      }

      idx++; // 處理下一筆 arrival == nextArrival 的訂單
    }

    // 回到 while 開頭，繼續下一輪
  }

  // 模擬完成後：計算 total delay 與 failure percentage
  int totaldelay = 0;
  for (size_t i = 0; i < abortList.size(); i++) {
    totaldelay += abortList[i].delay;
  }
  for (size_t i = 0; i < timeoutList.size(); i++) {
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
  for (size_t i = 0; i < abortList.size(); i++) {
    fout << "[" << (i + 1) << "]\t"
         << abortList[i].OID << "\t"
         << abortList[i].CID << "\t"
         << abortList[i].delay << "\t"
         << abortList[i].abort << "\n";
  }

  fout << "\t[Timeout List]\n";
  fout << "\tOID\tCID\tDelay\tDeparture\n";
  for (size_t i = 0; i < timeoutList.size(); i++) {
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
  delete[] qs; // 釋放動態記憶體
}

int main() {
  Start();                
  std::string com;
  int command;
  Order *arr = nullptr;
  int quantity = 0;
  bool file_loaded = false;          // 標記：有沒有成功讀過 sorted 檔
  std::string loaded_file_number;    // 記錄：最近一次 command 2 讀的是哪個檔號 (例如 "401")

  while (1) {
    std::getline(std::cin, com);
    if (com.empty()) {
      continue;
    }

    std::stringstream ss(com);
    // 檢查輸入是否為單一數字，且在 0-4 之間
    if (!(ss >> command) || !(ss.eof()) || command < 0 || command > 4) {
      std::cout << std::endl << "Command does not exist!" << std::endl;
      Start();
      continue;
    }

    if (command == 0) {
      // 結束前釋放動態陣列
      delete[] arr;
      return 0;
    } 
    
    else if (command == 1) {
      // 任務一：讀取原始檔案並排序
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
      // 任務二：讀取已排序檔案並進行單佇列模擬
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
      // 任務三：雙佇列 SQF 模擬 (N=2)
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
      // 任務四：多佇列 SQF 模擬 (N 可變)
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
        // 檢查輸入是否為數字
        if (!(ssN >> N) || N < 0) { 
          std::cout << "\nInput the number of queues: ";
          continue;
        }

        // 檢查 N 範圍 [1, 19]
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
      if (N == 1) prefix = "one"; // 實際上 N=1 時和 command 2 行為相同
      else if (N == 2) prefix = "two"; // 實際上 N=2 時和 command 3 行為相同
      else prefix = "any";

      SimulateMultiQueues(arr, quantity, N, prefix, loaded_file_number);
      Start();
      continue;
    }
  }
}