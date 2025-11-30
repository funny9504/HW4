// 11327107 謝芳倪 11327115 郭琮禮

#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <fstream>   // 檔案讀取
#include <sstream>
#include <chrono>    // 計時 微秒
#include <iomanip>   // for std::fixed, std::setprecision
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

// 待處理的訂單
struct Order {
  int OID;        // 訂單ID
  int arrival;    // 訂單到達時間
  int duration;   // 處理所需時間
  int timeout;    // 訂單逾時時間（超過此時間完成視為失敗）
};

// 被取消的訂單
struct Abortlist {
  int OID;        
  int CID;        // 廚師ID
  int delay;      // 取消時間點 - 接單時間點
  int abort;      // 取消時間點
};

// 完成後逾時的訂單
struct Timeout {
  int OID;        
  int CID;        
  int delay;      // 完成時間 - (接單時間 + 製作時間)
  int departure;  // 完成時間點
};

bool Loadfile(std::string &file, Order* &arr, int &quantity) {
  std::ifstream fin(file);
  if (!fin) { 
    std::cout << std::endl;
    std::cout << "### " << file << " does not exist! ###";
    return false; // 檔案不存在，讀取失敗
  }
  
  std::string line;
  getline(fin, line); // 跳過欄位名稱

  quantity = 0;
  while (getline(fin, line)) { // 計算資料有幾筆
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
  return true;  // 讀取成功
}

// 列印訂單資料
void Print(Order *arr, int quantity) {
  std::cout << std::endl;
  std::cout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl;
  for (int i = 0; i < quantity; i++) {
    std::cout << "(" << i + 1 << ") \t"
              << arr[i].OID << "\t"
              << arr[i].arrival << "\t"
              << arr[i].duration << "\t"
              << arr[i].timeout << std::endl;
  }
}

// 寫sorted檔
void SetSortFile(Order *arr, int quantity, std::string &sortfile) {
  std::ofstream fout(sortfile);                              // 建立 sortedXXX.txt 輸出檔

  fout << "\tOID\tArrival\tDuration\tTimeOut" << std::endl; // 寫入標題

  for (int i = 0; i < quantity; i++) {                    // 依序寫入每筆排序後資料
    fout << arr[i].OID << "\t"
         << arr[i].arrival << "\t"
         << arr[i].duration << "\t"
         << arr[i].timeout << std::endl;
  }

  fout.close();                                              // 關閉檔案
}

// Shell Sort 排序
void ShellSort(Order *arr, int quantity) {
  for (int gap = quantity / 2; gap > 0; gap /= 2) {        // 逐步縮小 gap（Shell Sort 標準流程）
    for (int i = gap; i < quantity; i++) {
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

// 協調 Command 1 的流程
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
  long long reading = duration_cast<microseconds>(end - start).count(); // 計算讀取時間

  Print(arr, quantity);                                      // 列印原始資料

  auto start1 = high_resolution_clock::now();                
  ShellSort(arr, quantity);                       
  auto end1 = high_resolution_clock::now();                 
  long long sorting = duration_cast<microseconds>(end1 - start1).count(); // 計算排序時間

  auto start2 = high_resolution_clock::now();             
  SetSortFile(arr, quantity, sortfile);                      // 寫入 sortedXXX.txt
  auto end2 = high_resolution_clock::now();                 
  long long writing = duration_cast<microseconds>(end2 - start2).count(); // 計算寫入時間

  std::cout << "\nReading data: " << reading << " us.\n";   
  std::cout << "\nSorting data: " << sorting << " us.\n";   
  std::cout << "\nWriting data: " << writing << " us.";                  
}

class Queue {
 private:
  Order* data;           // 動態配置的陣列，用來存放訂單
  int capacity;          // 佇列最大容量
  int front;             // 佇列頭索引（取出位置）
  int back;              // 佇列尾索引（放入位置）
  int count;             // 佇列內的資料筆數

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
    count++;                      
    return true;
  }

  // 取出資料
  bool pop(Order &order) {
    if (empty()) {                // 空佇列 → pop 失敗
      return false;
    }

    order = data[front];          // 取出頭端資料
    front = (front + 1) % capacity; // 循環佇列頭指標往後移
    count--;                      
    return true;
  }

  int size() const { 
    return count;                 // 回傳佇列內的訂單數
  }
};

// 模擬單一 FIFO 佇列
void SetOneFile(Order *arr, int n, std::string com) {
  std::string onefile = "one" + com + ".txt";
  std::ofstream fout(onefile);
  Queue q;
  int idletime = 0; // 閒置時刻
  int idx = 0;      // 處理輸入訂單陣列的索引

  std::vector<Abortlist> abortList;
  std::vector<Timeout> timeoutList;

  // 計算有效訂單總數
  int validnum = 0;
  for(int i = 0; i < n; i++) {
    // 檢查 duration > 0 且 訂單完成時間 (arrival + duration) <= 逾時時間
    if (arr[i].duration > 0 && (arr[i].arrival + arr[i].duration <= arr[i].timeout)) {
      validnum++;
    }
  }

  while (idx < n || !q.empty()) {  
        
    // 跳過無效訂單
    while (idx < n && (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      idx++;
    }
        
    // 空佇列
    if (q.empty()) {
      // 如果還有訂單，閒置時間至少是下一個訂單的到達時間
      if (idx < n && idletime < arr[idx].arrival) {
        idletime = arr[idx].arrival;
      }
            
      // 補訂單：將在 idletime 或之前到達的有效訂單補進佇列
      while (idx < n && arr[idx].arrival <= idletime) {
        // 過濾無效單
        if (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout) {
          idx++; 
          continue; 
        }

        q.push(arr[idx]);
        idx++;
                
        break;  // 單佇列模擬，每次空佇列只補進第一筆，因為 idleTime 會被更新
      }

      // 如果補完還是空的，且沒有更多輸入訂單，則結束模擬
      if (q.empty()) {
        if (idx >= n) {
          break;
        } else {
          continue; // 佇列仍空，但還有輸入訂單（等待下一筆訂單到達）
        }
      }
    }

    // 佇列有訂單，取出並處理
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
      } else {
        q.push(arr[idx]); // 入佇列
      }
      idx++;
    }

    idletime = finishtime; // 更新閒置時間為目前餐點完成時間
    if (cur.timeout < finishtime) { // 餐點逾時
      int delay = (idletime - cur.duration) - cur.arrival;
      timeoutList.push_back({cur.OID, 1, delay, idletime});
    }
  }

  // 計算總延遲
  int totaldelay = 0;
  for (int i = 0; i < abortList.size(); i++) {
    totaldelay += abortList[i].delay;
  }

  for (int i = 0; i < timeoutList.size(); i++) {
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
  for (int i = 0; i < abortList.size(); i++) {
    fout << "[" << (i + 1) << "]\t" 
         << abortList[i].OID << "\t" 
         << abortList[i].CID << "\t" 
         << abortList[i].delay << "\t" 
         << abortList[i].abort << "\n";
  }

    // 寫入 Timeout List
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

// 處理一個佇列直到某個時間點
void processChefUntil(int chefId, int limitTime,
                      Queue &q, int &idleTime,
                      std::vector<Abortlist> &abortList,
                      std::vector<Timeout> &timeoutList) {

  // 只要佇列不空，且處理完一個後閒置時間不超過限制時間
  while (!q.empty() && idleTime <= limitTime) {
    Order cur;
    q.pop(cur); // 取出要處理的訂單

    // 實際開始處理時間：max(廚師可開始時間, 訂單到達時間)
    int startTime = std::max(idleTime, cur.arrival);

    // 處理前逾時（開始處理時間 > 訂單逾時時間）
    if (cur.timeout < startTime) {
      int delay = startTime - cur.arrival;
      abortList.push_back({cur.OID, chefId + 1, delay, startTime});
      idleTime = startTime; // 閒置時間更新為取消時間點
      continue;
    }

    // 處理完成時間
    int finishTime = startTime + cur.duration;
    idleTime = finishTime;  // 閒置時間更新為完成時間

    // 處理後逾時（完成時間 > 訂單逾時時間）
    if (cur.timeout < finishTime) {
      int delay = startTime - cur.arrival;  // 延遲 = 等待時間
      timeoutList.push_back({cur.OID, chefId + 1, delay, finishTime});
    }
  }
}

// 模擬多個佇列的排程
void SimulateMultiQueues(Order* arr, int n, int N,
                         const std::string& prefix,
                         const std::string& com) {
  std::string outFile = prefix + com + ".txt";
  std::ofstream fout(outFile);

  int* idle = new int[N];   // 每個廚師的閒置時間
  Queue* qs = new Queue[N]; // N 個佇列
  for (int i = 0; i < N; i++) {
    idle[i] = 0;
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

  int idx = 0;
  std::vector<int> idleChefs; // 暫存閒置廚師的 ID

  // 輸入階段 (處理所有從檔案讀入的訂單)
  while (idx < n) {
    // 跳過無效訂單
    while (idx < n && (arr[idx].duration <= 0 || (arr[idx].arrival + arr[idx].duration) > arr[idx].timeout)) {
      idx++;
    }
    if (idx >= n) {
      break;  // 已無輸入訂單
    }

    int nextArrival = arr[idx].arrival; // 下一個訂單到達時間

    // Step A: 更新舊狀態
    for (int i = 0; i < N; i++) {
      if (!qs[i].empty()) {
        if (idle[i] <= nextArrival) {
          processChefUntil(i, nextArrival, qs[i], idle[i], abortList, timeoutList);
        }
      }
    }

    // Step B: 分配新訂單
    while (idx < n && arr[idx].arrival == nextArrival) {  // 處理所有同時到達的訂單
      Order &cur = arr[idx];
      // 再次過濾無效訂單
      if (cur.duration <= 0 || cur.arrival + cur.duration > cur.timeout) {
        idx++; continue;
      }

      idleChefs.clear();
      // 尋找空閒的廚師
      for (int i = 0; i < N; i++) {
        if (idle[i] <= nextArrival && qs[i].empty()) {
          idleChefs.push_back(i);
        }
      }

      int chosen = -1;
      if (idleChefs.size() >= 1) {
        // 優先選擇閒置廚師 (選 CID 小的)
        chosen = idleChefs[0];
      } else {
        // 沒有閒置廚師，檢查是否所有佇列都滿了
        bool allFull = true;
        for (int i = 0; i < N; i++) {
          if (!qs[i].full()) { 
            allFull = false; break; 
          }
        }

        if (allFull) {
          // 所有佇列都滿，取消訂單
          abortList.push_back({cur.OID, 0, 0, cur.arrival});
        } else {
          // 選擇最短的佇列(SQF)
          int bestLen = std::numeric_limits<int>::max();
          for (int i = 0; i < N; i++) {
            if (!qs[i].full()) {
              int len = qs[i].size();
              // 找最短佇列，若長度相同，選 CID 小的
              if (len < bestLen) { 
                bestLen = len; chosen = i; 
              }
            }
          }
        }
      }

      if (chosen != -1) {
        qs[chosen].push(cur);
        // 檢查廚師是否在訂單到達時是閒置的
        if (idle[chosen] <= cur.arrival) {
             processChefUntil(chosen, cur.arrival, qs[chosen], idle[chosen], abortList, timeoutList);
        }
      }
      idx++;
    }
  }

  // 清空階段 (處理佇列中剩餘的訂單)
  std::vector<Order> curOrder(N);       // 暫存每個佇列彈出的訂單
  std::vector<bool> hasOrder(N, false); // 標記每個廚師是否有彈出的訂單待處理

  while (true) {
      // 從非空佇列中彈出一個訂單到 curOrder
      bool anyBusy = false;
      for (int i = 0; i < N; i++) {
          if (!hasOrder[i] && !qs[i].empty()) {
              qs[i].pop(curOrder[i]); 
              hasOrder[i] = true;
          }

          // 只要有任何廚師有訂單待處理，就繼續迴圈
          if (hasOrder[i]) {
            anyBusy = true;
          }
      }

      // 所有佇列都空了，且所有彈出的訂單都處理完畢
      if (!anyBusy) {
        break;
      }

      // 比對：找出最早發生的事件
      int minEventTime = std::numeric_limits<int>::max();
      int targetChef = -1;

      for (int i = 0; i < N; i++) {
          if (hasOrder[i]) {
              Order &order = curOrder[i];
              // 實際開始處理時間：max(廚師可開始時間, 訂單到達時間)
              int start = std::max(idle[i], order.arrival);
              int eventTime;
              
              if (order.timeout < start) {
                  eventTime = start; 
              } else {
                  eventTime = start; 
              }

              // 找最小值 (若 start 時間相同，選 CID 小的)
              if (eventTime < minEventTime) {
                  minEventTime = eventTime;
                  targetChef = i;
              }
          }
      }

      // 執行：處理選中的訂單
      if (targetChef != -1) {
          Order &cur = curOrder[targetChef];
          int start = std::max(idle[targetChef], cur.arrival);
          
          if (cur.timeout < start) {
              int delay = start - cur.arrival;
              abortList.push_back({cur.OID, targetChef + 1, delay, start});
              idle[targetChef] = start; // 廚師在 Abort 時間點即可開始下一個任務
          } else {
              int finishTime = start + cur.duration;
              idle[targetChef] = finishTime;  // 廚師在 finishTime 點才可開始下一個任務
              if (cur.timeout < finishTime) {
                  int delay = start - cur.arrival;
                  timeoutList.push_back({cur.OID, targetChef + 1, delay, finishTime});
              }
          }
          hasOrder[targetChef] = false; // 該廚師完成一單，準備從佇列彈出下一單
      }
  }

  // ===== 模擬結束，統計 Total Delay 與 Failure Percentage =====
  int totaldelay = 0;
  for (int i = 0; i < (int)abortList.size(); i++) {
    totaldelay += abortList[i].delay;
  }
  for (int i = 0; i < (int)timeoutList.size(); i++) {
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
  for (int i = 0; i < (int)abortList.size(); i++) {
    fout << "[" << (i + 1) << "]\t"
         << abortList[i].OID << "\t"
         << abortList[i].CID << "\t"
         << abortList[i].delay << "\t"
         << abortList[i].abort << "\n";
  }

  fout << "\t[Timeout List]\n";
  fout << "\tOID\tCID\tDelay\tDeparture\n";
  for (int i = 0; i < (int)timeoutList.size(); i++) {
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
  delete[] idle;
  delete[] qs;
}

int main() {
  Start();                // 顯示主菜單
  std::string com;
  int command;
  Order *arr = nullptr;
  int quantity = 0;
  bool file_loaded = false;          // 有沒有成功讀過 sorted 檔
  std::string loaded_file_number;    // 最近一次 command 2 讀的檔號

  while (1) {
    std::getline(std::cin, com);
    if (com.empty()) {
      continue;
    }

    std::stringstream ss(com);
    // 嘗試將輸入轉換為整數指令
    if (!(ss >> command)) {
      // 讀不到整數或輸入無效，結束程式
      if (arr != nullptr) {
        delete[] arr;
      }
      return 0;
    }

    if (command < 0 || command > 4) {
      std::cout << std::endl << "Command does not exist!" << std::endl;
      Start();
      continue;
    }

    if (command == 0) {
      if (arr != nullptr) {
        delete[] arr;
      }
      return 0;

    } else if (command == 1) {
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

    } else if (command == 2) {
      std::cout << std::endl;
      std::cout << "Input a file number (e.g., 401, 402, 403, ...): ";
      std::getline(std::cin, com);
      while (com.empty()) {
        std::getline(std::cin, com);
      }
      std::string sortfile = "sorted" + com + ".txt";
      // 釋放上次載入的陣列
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
      SetOneFile(arr, quantity, com);
      Start();
      continue;

    } else if (command == 3) {
      // 任務三：一定要先有任務二
      if (!file_loaded) {
        std::cout << "\n### Execute command 2 first! ###\n";
        Start();
        continue;
      }

      // N 固定為 2，prefix 為 "two"（產生 twoXXX.txt）
      SimulateMultiQueues(arr, quantity, 2, "two", loaded_file_number);
      Start();
      continue;

    } else if (command == 4) {
      // 任務四：一定要先有任務二
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
