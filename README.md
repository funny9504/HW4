// 11327107 謝芳倪 11327115 郭琮禮

#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <fstream>  // 檔案讀取
#include <sstream>
#include <chrono> // 計時 微秒
#include <iomanip>


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
  int abort;
  int delay;
};

struct Timeout {
  int OID;
  int CID;
  int departure;
  int delay;
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

void SetSortFile(Order *arr, int quantity, std::string &newfile) {
  std::ofstream fout(newfile);
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

void SaveAndShort(const std::string &com) {
  Order *arr = nullptr;
  int quantity = 0;
  std::string filename = "input" + com + ".txt";
  std::string newfile = "sorted" + com + ".txt";

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
  SetSortFile(arr, quantity, newfile);
  auto end2 = high_resolution_clock::now();
  long long writing = duration_cast<microseconds>(end2 - start2).count();

  std::cout << "Reading data: " << reading << " us.\n" << std::endl;
  std::cout << "Sorting data: " << sorting << " us.\n" << std::endl;
  std::cout << "Writing data: " << writing << " us.";
  
  
}

class Queue {
 private:
  Order data[3];
  static const int capacity = 3; // 最多三筆
  int front;
  int back;
  int count;

 public:
  Queue() {
    front = 0;
    back = 0;
    count = 0;
  }

  bool empty() {
    return count == 0;
  }

  bool full() {
    return count == capacity;
  }

  bool push(Order &order) {
    if ( full() ) {
      return false;
    }

    data[back] = order;
    back = ( back + 1 ) % 3;
    count++;
    return true;
  }

  bool pop(Order &order) {
    if ( empty() ) {
      return false;
    }

    order = data[front];
    front = ( front + 1 ) % 3;
    count--;
    return true;
  }
};

void SetOneFile(Order *arr, int n) {
    Queue q;
    int current = 0; // 廚師閒置時刻
    int idx = 0; 

    std::vector<Abortlist> abortList;
    std::vector<Timeout> timeoutList;

    while (idx < n || !q.empty()) {
        
        // --- 情況 A: 佇列全空，且後面還有訂單沒來 ---
        if (q.empty() && idx < n && current < arr[idx].arrival) {
            current = arr[idx].arrival;
        }

        // --- 情況 B: 處理「現在」以前到達的新訂單 ---
        while (idx < n && arr[idx].arrival <= current) {
            if (q.full()) {
                Abortlist a;
                a.OID = arr[idx].OID;
                a.abort = arr[idx].arrival;
                a.delay = 0; // 佇列滿拒單，Delay 為 0
                abortList.push_back(a);
            } else {
                q.push(arr[idx]);
            }
            idx++;
        }

        // 若佇列還是空的
        if (q.empty()) {
            continue; 
        }

        // 開始做菜
        Order cur;
        q.pop(cur); // 同時取得並釋放空間

        int start = current;

        if (cur.timeout < start) {
            Abortlist a;
            a.OID = cur.OID;
            a.abort = start;
            a.delay = start - cur.arrival; // 取出時已逾時，Delay > 0
            abortList.push_back(a);
          
            continue; 
        }

        // 3. 正常製作
        int finish_time = start + cur.duration;

        while (idx < n && arr[idx].arrival < finish_time) {
            if (q.full()) {
                Abortlist a;
                a.OID = arr[idx].OID;
                a.abort = arr[idx].arrival;
                a.delay = 0;
                abortList.push_back(a);
            } else {
                q.push(arr[idx]);
            }
            idx++;
        }

        // 4. 更新時間與結算
        current = finish_time;

        if (cur.timeout < finish_time) {
            Timeout t;
            t.OID = cur.OID;
            t.departure = finish_time;
            t.delay = start - cur.arrival; 
            timeoutList.push_back(t);
        }
    }


    int totalDelay = 0;
    for (auto &a : abortList) totalDelay += a.delay;
    for (auto &t : timeoutList) totalDelay += t.delay;

    double failurePercent = 0.0;
    if (n > 0) {
        failurePercent = (double)(abortList.size() + timeoutList.size()) / n * 100.0;
    }
    // 四捨五入到小數點後兩位
    failurePercent = int(failurePercent * 100 + 0.5) / 100.0;

    // Abort List
    std::cout << "\t[Abort List]\n";
    std::cout << "\tOID\tCID\tDelay\tAbort\n";
    for (size_t i = 0; i < abortList.size(); ++i) {
        int cid = (abortList[i].delay == 0) ? 0 : 1;
        std::cout << "[" << (i + 1) << "]\t" 
                  << abortList[i].OID << "\t" 
                  << cid << "\t" 
                  << abortList[i].delay << "\t" 
                  << abortList[i].abort << "\n";
    }

    // Timeout List
    std::cout << "\t[Timeout List]\n";
    std::cout << "\tOID\tCID\tDelay\tDeparture\n";
    for (size_t i = 0; i < timeoutList.size(); ++i) {
        std::cout << "[" << (i + 1) << "]\t" 
                  << timeoutList[i].OID << "\t" 
                  << 1 << "\t" 
                  << timeoutList[i].delay << "\t" 
                  << timeoutList[i].departure << "\n";
    }

    std::cout << "[Total Delay]\n";
    std::cout << totalDelay << " min.\n";
    
    std::cout << "[Failure Percentage]\n";
    std::cout << std::fixed << std::setprecision(2) << failurePercent << " %\n";
} 


int main() {
  Start();                
  std::string com;
  int command;
  Order *arr = nullptr;
  int quantity = 0;
  bool file_loaded = false;

  while (1) {
    std::getline(std::cin, com);
    if (com.empty()) {
      continue;
    }

    std::stringstream ss(com);  // 把字串 com 放進 stringstream 裡
    // 無法成功讀取整數, 有多餘的東西沒用完
    if (!(ss >> command) || !(ss.eof()) || command < 0 || command > 4) {
      std::cout << std::endl << "Command does not exist!" << std::endl;
      Start();
      continue;
    }

    if (command == 0) {
      return 0;
    } 
    
    else if (command == 1) {
      std::cout << std::endl;
      std::cout << "Input a file number (e.g., 401, 402, 403, ...): ";
      std::getline(std::cin, com);
      while (com.empty()) {
        std::getline(std::cin, com);
      }
      SaveAndShort(com);
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
      if (!Loadfile(sortfile, arr, quantity)) {
        std::cout << std::endl;
        Start();
        continue;
      }
      
      Print(arr, quantity);
      SetOneFile(arr, quantity);

      Start();
      continue;
    }

    
      

  }
}
