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
      SetOneFile(arr, quantity, com);

      Start();
      continue;
    }

    
      

  }
}
