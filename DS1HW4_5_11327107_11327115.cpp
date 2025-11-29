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
  while (!q.empty() && idleTime <= limitTime) {                // 只有在廚師可工作且佇列不空時處理
    Order cur;
    q.pop(cur);                                                // 取出一筆舊訂單（FIFO）

    int startTime = std::max(idleTime, cur.arrival);           // 真正開始時間：不能早於 arrival

    if (cur.timeout < startTime) {                             // 取出時即逾時 → 取消
      int delay = startTime - cur.arrival;
      abortList.push_back({cur.OID, chefId + 1, delay, startTime});
      idleTime = startTime;                                     // 廚師時間前進到取消時刻
      continue;
    }

    int finishTime = startTime + cur.duration;                 // 完成時間
    idleTime = finishTime;                                      // 更新廚師閒置時刻

    if (cur.timeout < finishTime) {                            // 完成後才發現逾時
      int delay = startTime - cur.arrival;
      timeoutList.push_back({cur.OID, chefId + 1, delay, finishTime});
    }
  }
}


// 函數：命令 3/4 處理函數：多佇列 (N 廚師) 模擬 (Shortest Queue First, SQF)
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

  // 計算有效訂單總數
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

    // 檢查所有佇列是否為空
    bool allEmpty = true;
    for (int i = 0; i < N; i++) {
      if (!qs[i].empty()) {
        allEmpty = false;
        break;
      }
    }

    // 步驟 2: 若沒有剩餘有效訂單且所有佇列皆空 -> 模擬結束
    if (idx >= n && allEmpty) {
      break;
    }

    // 步驟 3: 找出下一個抵達的訂單時間 (nextArrival)
    int nextArrival;
    if (idx < n) {
      nextArrival = arr[idx].arrival;
    }
    else {
      // 若沒有新訂單，則設為最大值，讓廚師把隊列中剩下的訂單做完
      nextArrival = std::numeric_limits<int>::max();
    }  

    // Step A: 舊訂單先處理到 nextArrival 之前 (處理佇列內較早可執行的舊訂單)
    for (int i = 0; i < N; i++) {
      if (!qs[i].empty()) {
        // 只要這位廚師在 nextArrival 之前有空，就讓他從 queue 取訂單來處理
        if (idleTime[i] <= nextArrival) { 
          processChefUntil(i, nextArrival, qs[i], idleTime[i], abortList, timeoutList);
        }
      }
    }

    // 若已沒有新訂單，則跳到下一輪 while 檢查是否結束
    if (idx >= n) {
      continue; 
    }

    // Step B: 在 nextArrival 時刻處理新的訂單（可能有多筆 arrival 相同）
    while (idx < n && arr[idx].arrival == nextArrival) {
      Order &cur = arr[idx];

      // 再次確認是否有效
      if (cur.duration <= 0 || cur.arrival + cur.duration > cur.timeout) {
        idx++;
        continue;
      }
      
      // === 根據四種狀況選擇廚師 ===

      int chosen = -1; // 最終選擇的廚師 (0-based index)
      
      // 1. 找出所有符合「閒置且佇列空的」廚師
      std::vector<int> idleAndEmptyChefs;
      for (int i = 0; i < N; i++) {
          // 閒置定義：「閒置時刻」<=新訂單的「下單時刻」
          if (idleTime[i] <= nextArrival && qs[i].empty()) {
              idleAndEmptyChefs.push_back(i);
          }
      }

      if (idleAndEmptyChefs.size() > 0) {
          // Case 1 & 2: 存在閒置且佇列空的廚師
          
          // Case 2/1 規則：選這些閒置廚師中「廚師編號」最小者。
          // 因為 idleAndEmptyChefs 是 i 從 0 到 N-1 順序加入的，
          // 所以第一個元素 (index 0) 就是編號最小者。
          chosen = idleAndEmptyChefs[0];
      } 
      
      else {
          // 沒有廚師是「閒置且佇列是空的」，進入 Case 3 或 Case 4
          
          int bestLen = std::numeric_limits<int>::max(); // 最短佇列長度
          int shortestQueueChef = -1;

          // 2. 找出最短佇列 (用於 Case 3 的基礎)
          for (int i = 0; i < N; i++) {
              // 只考慮非滿的佇列
              if (!qs[i].full()) {
                  int len = qs[i].size();
                  
                  // Case 3 規則：選最短的；長度相同時，選編號最小者。
                  // 由於 i 是從小到大，使用 '<' 比較能保證當 len == bestLen 時，chosen 保持最小的 i (CID)。
                  if (len < bestLen) { 
                      bestLen = len;
                      shortestQueueChef = i;
                  }
              }
          }
          
          if (shortestQueueChef != -1) {
              // Case 3: 每位廚師都並非閒置 (已由外層 else 確定) 且至少一個佇列並非全滿
              chosen = shortestQueueChef;
          }
          else {
              // Case 4: 每位廚師都並非閒置且佇列全滿
              // chosen 保持 -1，將在下方邏輯中被取消
          }
      }

      // 3. 執行選擇或取消
      if (chosen != -1) {
          // Case 1, 2, 3: 找到廚師，將訂單丟入
          qs[chosen].push(cur);
      } 
      
      else {
          // Case 4: 沒有廚師可選 (所有佇列都滿)
          // 立即取消，CID 記成 0 號
          abortList.push_back({cur.OID, 0, 0, cur.arrival}); 
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

  // === 寫檔 (省略，保持原樣) ===
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
      // 結束前釋放動態陣列
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
